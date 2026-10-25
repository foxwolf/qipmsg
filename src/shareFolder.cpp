// This file is part of QIpMsg.
//
// QIpMsg is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "shareFolder.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>

const int ShareFolder::maxListEntries = 200;

namespace {

QString cleanAbs(const QString &path)
{
    QString cleaned = QDir::cleanPath(QFileInfo(path).absoluteFilePath());
    cleaned.replace(QLatin1Char('\\'), QLatin1Char('/'));
#ifdef Q_OS_WIN
    cleaned = cleaned.toLower();
#endif
    return cleaned;
}

bool insideRoot(const QString &rootAbs, const QString &destAbs)
{
    const QString root = cleanAbs(rootAbs);
    const QString dest = cleanAbs(destAbs);
    if (root.isEmpty() || dest.isEmpty()) {
        return false;
    }
    return dest == root || dest.startsWith(root + QLatin1Char('/'));
}

QString rootCanonical(const QString &root, QString *error)
{
    if (root.trimmed().isEmpty()) {
        if (error) {
            *error = QStringLiteral("off");
        }
        return QString();
    }
    const QFileInfo info(root);
    if (!info.exists() || !info.isDir()) {
        if (error) {
            *error = QStringLiteral("off");
        }
        return QString();
    }
    const QString canon = info.canonicalFilePath();
    if (canon.isEmpty()) {
        if (error) {
            *error = QStringLiteral("off");
        }
        return QString();
    }
    return canon;
}

QByteArray hexToBytes(const QByteArray &hex)
{
    return QByteArray::fromHex(hex);
}

} // namespace

QString ShareFolder::localToken(bool enabled, const QString &path,
                                const QString &password)
{
    QString token = QStringLiteral("qipmsg");
    if (enabled && !path.trimmed().isEmpty()) {
        token += QLatin1String(",share");
        if (!password.isEmpty()) {
            token += QLatin1String(",lock");
        }
    }
    return token;
}

bool ShareFolder::tokenIsQipmsg(const QString &token)
{
    return token == QLatin1String("qipmsg")
            || token.startsWith(QLatin1String("qipmsg,"));
}

bool ShareFolder::tokenHasShare(const QString &token)
{
    if (!tokenIsQipmsg(token)) {
        return false;
    }
    return token.split(QLatin1Char(',')).contains(QLatin1String("share"));
}

bool ShareFolder::tokenHasLock(const QString &token)
{
    if (!tokenHasShare(token)) {
        return false;
    }
    return token.split(QLatin1Char(',')).contains(QLatin1String("lock"));
}

bool ShareFolder::passwordMatches(const QString &expected, const QString &given)
{
    if (expected.isEmpty()) {
        return true;
    }
    const QByteArray left = expected.toUtf8();
    const QByteArray right = given.toUtf8();
    int diff = left.size() ^ right.size();
    const int count = qMax(left.size(), right.size());
    for (int i = 0; i < count; ++i) {
        const unsigned char a = i < left.size()
                ? static_cast<unsigned char>(left.at(i)) : 0;
        const unsigned char b = i < right.size()
                ? static_cast<unsigned char>(right.at(i)) : 0;
        diff |= a ^ b;
    }
    return diff == 0;
}

bool ShareFolder::resolveInside(const QString &root, const QString &relative,
                                QString *absolute, QString *error)
{
    const QString rootCanon = rootCanonical(root, error);
    if (rootCanon.isEmpty()) {
        return false;
    }

    QString rel = relative;
    rel.replace(QLatin1Char('\\'), QLatin1Char('/'));
    if (rel.startsWith(QLatin1Char('/'))
            || (rel.size() >= 2 && rel.at(1) == QLatin1Char(':'))) {
        if (error) {
            *error = QStringLiteral("badpath");
        }
        return false;
    }
    const QStringList parts = rel.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    for (int i = 0; i < parts.size(); ++i) {
        const QString &part = parts.at(i);
        if (part == QLatin1String(".") || part == QLatin1String("..")
                || part.contains(QLatin1Char(':'))
                || part.contains(QChar(0))) {
            if (error) {
                *error = QStringLiteral("badpath");
            }
            return false;
        }
    }
    const QString joined = parts.join(QLatin1Char('/'));
    const QString target = joined.isEmpty()
            ? rootCanon : QDir(rootCanon).filePath(joined);
    const QFileInfo targetInfo(target);
    if (!targetInfo.exists()) {
        if (error) {
            *error = QStringLiteral("missing");
        }
        return false;
    }
    const QString canon = targetInfo.canonicalFilePath();
    if (canon.isEmpty() || !insideRoot(rootCanon, canon)) {
        if (error) {
            *error = QStringLiteral("badpath");
        }
        return false;
    }
    if (absolute) {
        *absolute = canon;
    }
    return true;
}

bool ShareFolder::listDirectory(const QString &root, const QString &relative,
                                QList<ShareEntry> *entries, bool *truncated,
                                QString *error)
{
    if (entries) {
        entries->clear();
    }
    if (truncated) {
        *truncated = false;
    }
    QString abs;
    if (!resolveInside(root, relative, &abs, error)) {
        return false;
    }
    if (!QFileInfo(abs).isDir()) {
        if (error) {
            *error = QStringLiteral("badpath");
        }
        return false;
    }
    const QString rootCanon = rootCanonical(root, error);
    QDir dir(abs);
    const QFileInfoList list = dir.entryInfoList(
                QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot,
                QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);
    int kept = 0;
    for (int i = 0; i < list.size(); ++i) {
        const QFileInfo info = list.at(i);
        const QString canon = info.canonicalFilePath();
        if (canon.isEmpty() || !insideRoot(rootCanon, canon)) {
            continue;
        }
        if (kept >= maxListEntries) {
            if (truncated) {
                *truncated = true;
            }
            break;
        }
        ShareEntry entry;
        entry.name = info.fileName();
        entry.directory = info.isDir();
        entry.size = info.isDir() ? 0 : info.size();
        entry.mtime = info.lastModified().toSecsSinceEpoch();
        if (entries) {
            entries->append(entry);
        }
        ++kept;
    }
    return true;
}

QByteArray ShareFolder::encodeList(const QList<ShareEntry> &entries,
                                   bool truncated)
{
    QByteArray out;
    out.append("QIP1\nOK\n");
    for (int i = 0; i < entries.size(); ++i) {
        const ShareEntry &entry = entries.at(i);
        out.append(entry.directory ? "D" : "F");
        out.append('\t');
        out.append(entry.name.toUtf8().toHex());
        out.append('\t');
        out.append(QByteArray::number(entry.size));
        out.append('\t');
        out.append(QByteArray::number(entry.mtime));
        out.append('\n');
    }
    out.append(truncated ? "TRUNC\n" : "END\n");
    return out;
}

bool ShareFolder::decodeList(const QByteArray &payload, QList<ShareEntry> *entries,
                             bool *truncated, QString *error)
{
    if (entries) {
        entries->clear();
    }
    if (truncated) {
        *truncated = false;
    }
    const QList<QByteArray> lines = payload.split('\n');
    if (lines.size() < 2 || lines.at(0).trimmed() != "QIP1") {
        if (error) {
            *error = QStringLiteral("bad");
        }
        return false;
    }
    const QByteArray status = lines.at(1).trimmed();
    if (status.startsWith("ERR")) {
        if (error) {
            *error = QString::fromUtf8(status.mid(3).trimmed());
        }
        return false;
    }
    if (status != "OK") {
        if (error) {
            *error = QStringLiteral("bad");
        }
        return false;
    }
    for (int i = 2; i < lines.size(); ++i) {
        const QByteArray line = lines.at(i).trimmed();
        if (line.isEmpty()) {
            continue;
        }
        if (line == "END") {
            return true;
        }
        if (line == "TRUNC") {
            if (truncated) {
                *truncated = true;
            }
            return true;
        }
        const QList<QByteArray> cols = line.split('\t');
        if (cols.size() < 4) {
            continue;
        }
        ShareEntry entry;
        entry.directory = cols.at(0) == "D";
        entry.name = QString::fromUtf8(hexToBytes(cols.at(1)));
        entry.size = cols.at(2).toLongLong();
        entry.mtime = cols.at(3).toLongLong();
        if (entry.name.isEmpty() || entry.name.contains(QLatin1Char('/'))
                || entry.name.contains(QLatin1Char('\\'))
                || entry.name == QLatin1String("..")
                || entry.name == QLatin1String(".")) {
            continue;
        }
        if (entries) {
            entries->append(entry);
        }
    }
    if (error) {
        *error = QStringLiteral("bad");
    }
    return false;
}

QByteArray ShareFolder::buildRequest(const QString &verb, const QString &relative,
                                     const QString &password, qint64 offset)
{
    QByteArray packet;
    packet.append("1:1:qip:qipmsg:");
    packet.append(QByteArray::number(126, 16));
    packet.append(':');
    packet.append(verb.toLatin1());
    packet.append(':');
    packet.append(relative.toUtf8().toHex());
    packet.append(':');
    packet.append(password.toUtf8().toHex());
    packet.append(':');
    if (verb == QLatin1String("GET")) {
        packet.append(QByteArray::number(offset < 0 ? 0 : offset, 16));
        packet.append(':');
    }
    packet.append('\n');
    return packet;
}

bool ShareFolder::parseRequest(const QByteArray &packet, QString *verb,
                               QString *relative, QString *password,
                               qint64 *offset)
{
    QByteArray body = packet;
    const int nl = body.indexOf('\n');
    if (nl >= 0) {
        body = body.left(nl);
    }
    const QList<QByteArray> parts = body.split(':');
    if (parts.size() < 8) {
        return false;
    }
    bool ok = false;
    const quint32 command = parts.at(4).toUInt(&ok, 16);
    if (!ok) {
        const quint32 decimal = parts.at(4).toUInt(&ok, 10);
        if (!ok || decimal != 126u) {
            return false;
        }
    } else if (command != 126u && command != 0x7eu) {
        // 126 decimal parsed as hex is not 0x7e. Accept both bases above.
        if (parts.at(4).toUInt(&ok, 10) != 126u) {
            return false;
        }
    }
    const QString action = QString::fromLatin1(parts.at(5));
    if (action != QLatin1String("LIST") && action != QLatin1String("GET")) {
        return false;
    }
    if (verb) {
        *verb = action;
    }
    if (relative) {
        *relative = QString::fromUtf8(hexToBytes(parts.at(6)));
    }
    if (password) {
        *password = QString::fromUtf8(hexToBytes(parts.at(7)));
    }
    qint64 off = 0;
    if (action == QLatin1String("GET") && parts.size() > 8) {
        off = parts.at(8).toLongLong(&ok, 16);
        if (!ok) {
            off = parts.at(8).toLongLong(&ok, 10);
        }
        if (!ok || off < 0) {
            return false;
        }
    }
    if (offset) {
        *offset = off;
    }
    return true;
}

ShareAnswer ShareFolder::answer(const QByteArray &request, bool enabled,
                                const QString &root,
                                const QString &expectedPassword)
{
    ShareAnswer result;
    result.offset = 0;
    result.fileSize = 0;
    result.sendFile = false;

    QString verb;
    QString relative;
    QString password;
    qint64 offset = 0;
    if (!parseRequest(request, &verb, &relative, &password, &offset)) {
        result.payload = "QIP1\nERR badpath\n";
        return result;
    }
    if (!enabled || root.trimmed().isEmpty()) {
        result.payload = "QIP1\nERR off\n";
        return result;
    }
    if (!passwordMatches(expectedPassword, password)) {
        result.payload = "QIP1\nERR auth\n";
        return result;
    }

    QString abs;
    QString error;
    if (!resolveInside(root, relative, &abs, &error)) {
        result.payload = "QIP1\nERR " + error.toUtf8() + "\n";
        return result;
    }

    if (verb == QLatin1String("LIST")) {
        if (!QFileInfo(abs).isDir()) {
            result.payload = "QIP1\nERR badpath\n";
            return result;
        }
        QList<ShareEntry> entries;
        bool truncated = false;
        if (!listDirectory(root, relative, &entries, &truncated, &error)) {
            result.payload = "QIP1\nERR " + error.toUtf8() + "\n";
            return result;
        }
        result.payload = encodeList(entries, truncated);
        return result;
    }

    const QFileInfo info(abs);
    if (!info.isFile() || info.isSymLink() && info.canonicalFilePath().isEmpty()) {
        result.payload = "QIP1\nERR badpath\n";
        return result;
    }
    if (offset > info.size()) {
        result.payload = "QIP1\nERR badpath\n";
        return result;
    }
    result.sendFile = true;
    result.filePath = info.canonicalFilePath().isEmpty()
            ? info.absoluteFilePath() : info.canonicalFilePath();
    result.offset = offset;
    result.fileSize = info.size();
    const qint64 remain = info.size() - offset;
    result.payload = "QIP1\nOK " + QByteArray::number(remain) + "\n";
    return result;
}
