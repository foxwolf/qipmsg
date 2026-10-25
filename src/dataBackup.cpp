// This file is part of QIpMsg.
//
// QIpMsg is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// QIpMsg is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with QIpMsg.  If not, see <http://www.gnu.org/licenses/>.
//

#include "dataBackup.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>

namespace {

const qint64 kMaxMemberBytes = 256ll * 1024ll * 1024ll;
const qint64 kMaxArchiveBytes = 1024ll * 1024ll * 1024ll;


void appendU16(QByteArray *out, quint16 value)
{
    out->append(static_cast<char>(value & 0xff));
    out->append(static_cast<char>((value >> 8) & 0xff));
}

void appendU32(QByteArray *out, quint32 value)
{
    appendU16(out, static_cast<quint16>(value & 0xffff));
    appendU16(out, static_cast<quint16>((value >> 16) & 0xffff));
}

bool readU16(const QByteArray &bytes, int offset, quint16 *value)
{
    if (offset < 0 || offset + 2 > bytes.size()) {
        return false;
    }
    const unsigned char *p = reinterpret_cast<const unsigned char *>(
                bytes.constData() + offset);
    *value = static_cast<quint16>(p[0] | (p[1] << 8));
    return true;
}

bool readU32(const QByteArray &bytes, int offset, quint32 *value)
{
    quint16 lo = 0;
    quint16 hi = 0;
    if (!readU16(bytes, offset, &lo) || !readU16(bytes, offset + 2, &hi)) {
        return false;
    }
    *value = static_cast<quint32>(lo) | (static_cast<quint32>(hi) << 16);
    return true;
}

quint32 crc32Of(const QByteArray &data)
{
    static quint32 table[256];
    static bool ready = false;
    if (!ready) {
        for (quint32 i = 0; i < 256; ++i) {
            quint32 c = i;
            for (int k = 0; k < 8; ++k) {
                if (c & 1u) {
                    c = 0xedb88320u ^ (c >> 1);
                } else {
                    c = c >> 1;
                }
            }
            table[i] = c;
        }
        ready = true;
    }

    quint32 crc = 0xffffffffu;
    const unsigned char *p = reinterpret_cast<const unsigned char *>(data.constData());
    for (int i = 0; i < data.size(); ++i) {
        crc = table[(crc ^ p[i]) & 0xffu] ^ (crc >> 8);
    }
    return crc ^ 0xffffffffu;
}

bool isExactFile(const QString &rel)
{
    return rel == QLatin1String("qipmsg.ini")
            || rel == QLatin1String("peers.json")
            || rel == QLatin1String("recent.json")
            || rel == QLatin1String("resume.json")
            || rel == QLatin1String("group_policy.json")
            || rel == QLatin1String("stickers.json")
            || rel == QLatin1String("discussions.json")
            || rel == QLatin1String("offline_queue.json");
}

bool isAllowedTree(const QString &rel)
{
    return rel == QLatin1String("history")
            || rel.startsWith(QLatin1String("history/"))
            || rel == QLatin1String("stickers")
            || rel.startsWith(QLatin1String("stickers/"))
            || rel == QLatin1String("avatars")
            || rel.startsWith(QLatin1String("avatars/"));
}

bool isAllowedFile(const QString &rel)
{
    if (isExactFile(rel)) {
        return true;
    }
    if (rel.endsWith(QLatin1Char('/'))) {
        return false;
    }
    return isAllowedTree(rel);
}

bool safeRelative(const QString &rel)
{
    if (rel.isEmpty() || rel.startsWith(QLatin1Char('/'))
            || rel.startsWith(QLatin1Char('\\'))) {
        return false;
    }
    if (rel.size() >= 2 && rel.at(1) == QLatin1Char(':')) {
        return false;
    }
    const QStringList parts = rel.split(QLatin1Char('/'));
    for (int i = 0; i < parts.size(); ++i) {
        const QString &part = parts.at(i);
        if (part.isEmpty() || part == QLatin1String(".")
                || part == QLatin1String("..")) {
            return false;
        }
    }
    return isAllowedFile(rel);
}

QString cleanAbs(const QString &path)
{
    QString cleaned = QDir::cleanPath(QFileInfo(path).absoluteFilePath());
    cleaned.replace(QLatin1Char('\\'), QLatin1Char('/'));
#ifdef Q_OS_WIN
    cleaned = cleaned.toLower();
#endif
    return cleaned;
}

bool pathInsideRoot(const QString &rootAbs, const QString &destAbs)
{
    const QString root = cleanAbs(rootAbs);
    const QString dest = cleanAbs(destAbs);
    return dest == root || dest.startsWith(root + QLatin1Char('/'));
}

void collectFiles(const QString &root, const QString &rel, QStringList *out,
                  QString *error)
{
    const QString abs = rel.isEmpty() ? root : QDir(root).filePath(rel);
    QDir dir(abs);
    if (!dir.exists()) {
        return;
    }
    const QFileInfoList list = dir.entryInfoList(
                QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable,
                QDir::Name);
    for (int i = 0; i < list.size(); ++i) {
        const QFileInfo fi = list.at(i);
        if (fi.isSymLink()) {
            continue;
        }
        const QString child = rel.isEmpty()
                ? fi.fileName()
                : rel + QLatin1Char('/') + fi.fileName();
        if (fi.isDir()) {
            if (rel.isEmpty()) {
                if (child == QLatin1String("history")
                        || child == QLatin1String("stickers")
                        || child == QLatin1String("avatars")) {
                    collectFiles(root, child, out, error);
                }
            } else if (isAllowedTree(child)) {
                collectFiles(root, child, out, error);
            }
            if (error && !error->isEmpty()) {
                return;
            }
            continue;
        }
        if (!fi.isFile() || !isAllowedFile(child)) {
            continue;
        }
        if (fi.size() > kMaxMemberBytes) {
            if (error) {
                *error = DataBackup::tr("File is too large to back up: %1").arg(child);
            }
            return;
        }
        out->append(child);
    }
}

struct CentralEntry
{
    QByteArray name;
    quint32 crc;
    quint32 size;
    quint32 offset;
    quint16 method;
    quint16 flags;
};

QByteArray localHeader(const CentralEntry &entry)
{
    QByteArray header;
    appendU32(&header, 0x04034b50u);
    appendU16(&header, 20);
    appendU16(&header, entry.flags);
    appendU16(&header, entry.method);
    appendU16(&header, 0);
    appendU16(&header, 0);
    appendU32(&header, entry.crc);
    appendU32(&header, entry.size);
    appendU32(&header, entry.size);
    appendU16(&header, static_cast<quint16>(entry.name.size()));
    appendU16(&header, 0);
    header.append(entry.name);
    return header;
}

QByteArray centralHeader(const CentralEntry &entry)
{
    QByteArray header;
    appendU32(&header, 0x02014b50u);
    appendU16(&header, 20);
    appendU16(&header, 20);
    appendU16(&header, entry.flags);
    appendU16(&header, entry.method);
    appendU16(&header, 0);
    appendU16(&header, 0);
    appendU32(&header, entry.crc);
    appendU32(&header, entry.size);
    appendU32(&header, entry.size);
    appendU16(&header, static_cast<quint16>(entry.name.size()));
    appendU16(&header, 0);
    appendU16(&header, 0);
    appendU16(&header, 0);
    appendU16(&header, 0);
    appendU32(&header, 0);
    appendU32(&header, entry.offset);
    header.append(entry.name);
    return header;
}

bool readAt(QFile *file, qint64 offset, qint64 count, QByteArray *out, QString *error)
{
    if (count < 0 || offset < 0) {
        if (error) {
            *error = DataBackup::tr("The archive is truncated.");
        }
        return false;
    }
    if (!file->seek(offset)) {
        if (error) {
            *error = DataBackup::tr("The archive is truncated.");
        }
        return false;
    }
    *out = file->read(count);
    if (out->size() != count) {
        if (error) {
            *error = DataBackup::tr("The archive is truncated.");
        }
        return false;
    }
    return true;
}

} // namespace

bool DataBackup::exportTree(const QString &root, const QString &zipPath,
                            QString *error, int *fileCount)
{
    if (fileCount) {
        *fileCount = 0;
    }
    if (error) {
        error->clear();
    }
    const QString rootAbs = cleanAbs(root);
    if (rootAbs.isEmpty() || !QDir(rootAbs).exists()) {
        if (error) {
            *error = DataBackup::tr("The data folder does not exist.");
        }
        return false;
    }

    QStringList rels;
    QString walkError;
    collectFiles(rootAbs, QString(), &rels, &walkError);
    if (!walkError.isEmpty()) {
        if (error) {
            *error = walkError;
        }
        return false;
    }
    rels.sort();

    QSaveFile out(zipPath);
    if (!out.open(QIODevice::WriteOnly)) {
        if (error) {
            *error = DataBackup::tr("Could not write %1.").arg(zipPath);
        }
        return false;
    }

    QList<CentralEntry> entries;
    qint64 offset = 0;
    for (int i = 0; i < rels.size(); ++i) {
        const QString rel = rels.at(i);
        if (!safeRelative(rel)) {
            continue;
        }
        const QString abs = QDir(rootAbs).filePath(rel);
        if (!pathInsideRoot(rootAbs, abs)) {
            continue;
        }
        QFile in(abs);
        if (!in.open(QIODevice::ReadOnly)) {
            if (error) {
                *error = DataBackup::tr("Could not read %1.").arg(rel);
            }
            return false;
        }
        const QByteArray payload = in.readAll();
        if (payload.size() != in.size() && in.size() >= 0) {
            if (error) {
                *error = DataBackup::tr("Could not read %1.").arg(rel);
            }
            return false;
        }

        CentralEntry entry;
        entry.name = rel.toUtf8();
        entry.crc = crc32Of(payload);
        entry.size = static_cast<quint32>(payload.size());
        entry.offset = static_cast<quint32>(offset);
        entry.method = 0;
        entry.flags = 0x0800;
        const QByteArray header = localHeader(entry);
        if (out.write(header) != header.size() || out.write(payload) != payload.size()) {
            if (error) {
                *error = DataBackup::tr("Could not write %1.").arg(zipPath);
            }
            return false;
        }
        offset += header.size() + payload.size();
        if (offset > 0xffffffffll) {
            if (error) {
                *error = DataBackup::tr("The backup is larger than 4 GB.");
            }
            return false;
        }
        entries.append(entry);
    }

    const qint64 cdOffset = offset;
    QByteArray central;
    for (int i = 0; i < entries.size(); ++i) {
        central.append(centralHeader(entries.at(i)));
    }
    if (out.write(central) != central.size()) {
        if (error) {
            *error = DataBackup::tr("Could not write %1.").arg(zipPath);
        }
        return false;
    }

    QByteArray eocd;
    appendU32(&eocd, 0x06054b50u);
    appendU16(&eocd, 0);
    appendU16(&eocd, 0);
    appendU16(&eocd, static_cast<quint16>(entries.size()));
    appendU16(&eocd, static_cast<quint16>(entries.size()));
    appendU32(&eocd, static_cast<quint32>(central.size()));
    appendU32(&eocd, static_cast<quint32>(cdOffset));
    appendU16(&eocd, 0);
    if (out.write(eocd) != eocd.size() || !out.commit()) {
        if (error) {
            *error = DataBackup::tr("Could not write %1.").arg(zipPath);
        }
        return false;
    }
    if (fileCount) {
        *fileCount = entries.size();
    }
    return true;
}

bool DataBackup::importTree(const QString &root, const QString &zipPath,
                            QString *error, int *fileCount)
{
    if (fileCount) {
        *fileCount = 0;
    }
    if (error) {
        error->clear();
    }
    const QString rootAbs = cleanAbs(root);
    if (rootAbs.isEmpty()) {
        if (error) {
            *error = DataBackup::tr("The data folder does not exist.");
        }
        return false;
    }
    QDir().mkpath(rootAbs);

    QFile file(zipPath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) {
            *error = DataBackup::tr("Could not read %1.").arg(zipPath);
        }
        return false;
    }
    const qint64 archiveSize = file.size();
    if (archiveSize < 22) {
        if (error) {
            *error = DataBackup::tr("The archive is not a zip file.");
        }
        return false;
    }
    if (archiveSize > kMaxArchiveBytes) {
        if (error) {
            *error = DataBackup::tr("The archive is larger than 1 GB.");
        }
        return false;
    }

    const qint64 scan = qMin(archiveSize, qint64(65557));
    QByteArray tail;
    if (!readAt(&file, archiveSize - scan, scan, &tail, error)) {
        return false;
    }
    const QByteArray signature("PK\x05\x06", 4);
    const int eocdAt = tail.lastIndexOf(signature);
    if (eocdAt < 0 || eocdAt + 22 > tail.size()) {
        if (error) {
            *error = DataBackup::tr("The archive is not a zip file.");
        }
        return false;
    }
    quint16 entriesOnDisk = 0;
    quint32 cdSize = 0;
    quint32 cdOffset = 0;
    if (!readU16(tail, eocdAt + 8, &entriesOnDisk)
            || !readU32(tail, eocdAt + 12, &cdSize)
            || !readU32(tail, eocdAt + 16, &cdOffset)) {
        if (error) {
            *error = DataBackup::tr("The archive is not a zip file.");
        }
        return false;
    }

    QByteArray central;
    if (!readAt(&file, cdOffset, cdSize, &central, error)) {
        return false;
    }

    QList<CentralEntry> entries;
    int cursor = 0;
    while (cursor + 46 <= central.size()) {
        quint32 sig = 0;
        if (!readU32(central, cursor, &sig) || sig != 0x02014b50u) {
            if (error) {
                *error = DataBackup::tr("The archive is not a zip file.");
            }
            return false;
        }
        quint16 flags = 0;
        quint16 method = 0;
        quint32 crc = 0;
        quint32 size = 0;
        quint16 nameLen = 0;
        quint16 extraLen = 0;
        quint16 commentLen = 0;
        quint32 localOffset = 0;
        if (!readU16(central, cursor + 8, &flags)
                || !readU16(central, cursor + 10, &method)
                || !readU32(central, cursor + 16, &crc)
                || !readU32(central, cursor + 20, &size)
                || !readU16(central, cursor + 28, &nameLen)
                || !readU16(central, cursor + 30, &extraLen)
                || !readU16(central, cursor + 32, &commentLen)
                || !readU32(central, cursor + 42, &localOffset)) {
            if (error) {
                *error = DataBackup::tr("The archive is truncated.");
            }
            return false;
        }
        const int nameAt = cursor + 46;
        if (nameAt + nameLen > central.size()) {
            if (error) {
                *error = DataBackup::tr("The archive is truncated.");
            }
            return false;
        }
        CentralEntry entry;
        entry.name = central.mid(nameAt, nameLen);
        entry.crc = crc;
        entry.size = size;
        entry.offset = localOffset;
        entry.method = method;
        entry.flags = flags;
        entries.append(entry);
        cursor = nameAt + nameLen + extraLen + commentLen;
        if (entries.size() > 20000) {
            if (error) {
                *error = DataBackup::tr("The archive has too many files.");
            }
            return false;
        }
    }
    if (entries.size() != entriesOnDisk && entriesOnDisk != 0
            && entries.size() != int(entriesOnDisk)) {
        if (error) {
            *error = DataBackup::tr("The archive is truncated.");
        }
        return false;
    }

    struct Pending
    {
        QString rel;
        QByteArray data;
    };
    QList<Pending> pending;
    for (int i = 0; i < entries.size(); ++i) {
        const CentralEntry &entry = entries.at(i);
        if (entry.flags & 0x0008) {
            if (error) {
                *error = DataBackup::tr("This zip uses a data descriptor. Export the backup from QIpMsg.");
            }
            return false;
        }
        if (entry.method != 0) {
            if (error) {
                *error = DataBackup::tr("This zip is compressed. Export the backup from QIpMsg.");
            }
            return false;
        }
        if (static_cast<qint64>(entry.size) > kMaxMemberBytes) {
            if (error) {
                *error = DataBackup::tr("A file in the archive is too large.");
            }
            return false;
        }
        const QString rel = (entry.flags & 0x0800)
                ? QString::fromUtf8(entry.name)
                : QString::fromLatin1(entry.name);
        const QString normalized = QString(rel).replace(QLatin1Char('\\'), QLatin1Char('/'));
        if (normalized.endsWith(QLatin1Char('/')) || entry.size == 0 && normalized.endsWith(QLatin1Char('/'))) {
            continue;
        }
        if (normalized.endsWith(QLatin1Char('/'))) {
            continue;
        }
        if (!safeRelative(normalized)) {
            if (error) {
                *error = DataBackup::tr("The archive contains a path that is not private qipmsg data.");
            }
            return false;
        }

        QByteArray local;
        if (!readAt(&file, entry.offset, 30, &local, error)) {
            return false;
        }
        quint32 sig = 0;
        quint16 localName = 0;
        quint16 localExtra = 0;
        if (!readU32(local, 0, &sig) || sig != 0x04034b50u
                || !readU16(local, 26, &localName)
                || !readU16(local, 28, &localExtra)) {
            if (error) {
                *error = DataBackup::tr("The archive is truncated.");
            }
            return false;
        }
        const qint64 dataAt = static_cast<qint64>(entry.offset) + 30
                + localName + localExtra;
        QByteArray payload;
        if (!readAt(&file, dataAt, entry.size, &payload, error)) {
            return false;
        }
        if (crc32Of(payload) != entry.crc) {
            if (error) {
                *error = DataBackup::tr("The archive checksum does not match.");
            }
            return false;
        }
        Pending item;
        item.rel = normalized;
        item.data = payload;
        pending.append(item);
    }

    if (pending.isEmpty()) {
        if (error) {
            *error = DataBackup::tr("The archive has no private qipmsg files.");
        }
        return false;
    }

    for (int i = 0; i < pending.size(); ++i) {
        const Pending &item = pending.at(i);
        const QString dest = QDir(rootAbs).filePath(item.rel);
        if (!pathInsideRoot(rootAbs, dest)) {
            if (error) {
                *error = DataBackup::tr("The archive contains a path that is not private qipmsg data.");
            }
            return false;
        }
        QDir().mkpath(QFileInfo(dest).absolutePath());
        QSaveFile out(dest);
        if (!out.open(QIODevice::WriteOnly)) {
            if (error) {
                *error = DataBackup::tr("Could not write %1.").arg(item.rel);
            }
            return false;
        }
        if (out.write(item.data) != item.data.size() || !out.commit()) {
            if (error) {
                *error = DataBackup::tr("Could not write %1.").arg(item.rel);
            }
            return false;
        }
    }
    if (fileCount) {
        *fileCount = pending.size();
    }
    return true;
}
