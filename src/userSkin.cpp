// This file is part of QIpMsg.
//
// QIpMsg is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "userSkin.h"
#include "helper.h"
#include "themeName.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

int puff(unsigned char *dest, unsigned long *destlen,
         const unsigned char *source, unsigned long *sourcelen);

namespace {

const int kMaxQssBytes = 256 * 1024;
const int kMaxJsonBytes = 64 * 1024;
const int kMaxImageBytes = 4 * 1024 * 1024;
const int kMaxZipBytes = 8 * 1024 * 1024;
const int kMaxZipMembers = 32;

QString &rootOverride()
{
    static QString dir;
    return dir;
}

QString &problemSlot()
{
    static QString text;
    return text;
}

QString skinsRoot()
{
    if (!rootOverride().isEmpty()) {
        return rootOverride();
    }
    return Helper::appHomePath() + QLatin1String("/skins");
}

QString cleanId(const QString &raw)
{
    QString out;
    const QString trimmed = raw.trimmed();
    for (int i = 0; i < trimmed.size() && out.size() < 64; ++i) {
        const QChar c = trimmed.at(i);
        if (c.isLetterOrNumber() || c == QLatin1Char('-') || c == QLatin1Char('.')) {
            out.append(c);
        } else if (c.isSpace() || c == QLatin1Char('_')) {
            if (!out.isEmpty() && !out.endsWith(QLatin1Char('-'))) {
                out.append(QLatin1Char('-'));
            }
        }
    }
    while (out.endsWith(QLatin1Char('-')) || out.endsWith(QLatin1Char('.'))) {
        out.chop(1);
    }
    while (out.startsWith(QLatin1Char('.'))) {
        out.remove(0, 1);
    }
    return skinIdOk(out) ? out : QString();
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

struct ZipMember
{
    QString name;
    QByteArray data;
};

bool memberNameOk(const QString &name)
{
    if (name.isEmpty() || name.startsWith(QLatin1Char('/'))
            || name.startsWith(QLatin1Char('\\'))) {
        return false;
    }
    if (name.size() >= 2 && name.at(1) == QLatin1Char(':')) {
        return false;
    }
    const QStringList parts = name.split(QLatin1Char('/'));
    for (int i = 0; i < parts.size(); ++i) {
        const QString &part = parts.at(i);
        if (part.isEmpty() || part == QLatin1String(".")
                || part == QLatin1String("..")) {
            return false;
        }
        if (part.contains(QLatin1Char('\\')) || part.contains(QChar(0))) {
            return false;
        }
    }
    return true;
}

bool inflateRaw(const QByteArray &input, int expectSize, QByteArray *output,
                QString *error)
{
    if (expectSize < 0 || expectSize > kMaxImageBytes) {
        if (error) {
            *error = QStringLiteral("size");
        }
        return false;
    }
    output->fill('\0', expectSize);
    unsigned long destLen = static_cast<unsigned long>(expectSize);
    unsigned long srcLen = static_cast<unsigned long>(input.size());
    const int rc = puff(reinterpret_cast<unsigned char *>(output->data()),
                        &destLen,
                        reinterpret_cast<const unsigned char *>(input.constData()),
                        &srcLen);
    if (rc != 0 || destLen != static_cast<unsigned long>(expectSize)) {
        output->clear();
        if (error) {
            *error = QStringLiteral("inflate");
        }
        return false;
    }
    return true;
}

bool readZip(const QString &path, QList<ZipMember> *members, QString *error)
{
    if (members) {
        members->clear();
    }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) {
            *error = QStringLiteral("open");
        }
        return false;
    }
    if (file.size() > kMaxZipBytes || file.size() < 22) {
        if (error) {
            *error = QStringLiteral("size");
        }
        return false;
    }
    const QByteArray bytes = file.readAll();
    int eocd = -1;
    const int minStart = qMax(0, bytes.size() - (65535 + 22));
    for (int i = bytes.size() - 22; i >= minStart; --i) {
        if (bytes.at(i) == 'P' && i + 3 < bytes.size()
                && static_cast<unsigned char>(bytes.at(i + 1)) == 0x4b
                && static_cast<unsigned char>(bytes.at(i + 2)) == 0x05
                && static_cast<unsigned char>(bytes.at(i + 3)) == 0x06) {
            eocd = i;
            break;
        }
    }
    if (eocd < 0) {
        if (error) {
            *error = QStringLiteral("zip");
        }
        return false;
    }
    quint16 count = 0;
    quint32 cdSize = 0;
    quint32 cdOffset = 0;
    if (!readU16(bytes, eocd + 10, &count)
            || !readU32(bytes, eocd + 12, &cdSize)
            || !readU32(bytes, eocd + 16, &cdOffset)) {
        if (error) {
            *error = QStringLiteral("zip");
        }
        return false;
    }
    if (count > kMaxZipMembers || cdOffset >= static_cast<quint32>(bytes.size())) {
        if (error) {
            *error = QStringLiteral("zip");
        }
        return false;
    }
    int cursor = static_cast<int>(cdOffset);
    QList<ZipMember> found;
    for (int n = 0; n < count; ++n) {
        if (cursor + 46 > bytes.size()) {
            if (error) {
                *error = QStringLiteral("zip");
            }
            return false;
        }
        quint32 sig = 0;
        if (!readU32(bytes, cursor, &sig) || sig != 0x02014b50u) {
            if (error) {
                *error = QStringLiteral("zip");
            }
            return false;
        }
        quint16 flags = 0;
        quint16 method = 0;
        quint32 compSize = 0;
        quint32 rawSize = 0;
        quint16 nameLen = 0;
        quint16 extraLen = 0;
        quint16 commentLen = 0;
        quint32 localOffset = 0;
        if (!readU16(bytes, cursor + 8, &flags)
                || !readU16(bytes, cursor + 10, &method)
                || !readU32(bytes, cursor + 20, &compSize)
                || !readU32(bytes, cursor + 24, &rawSize)
                || !readU16(bytes, cursor + 28, &nameLen)
                || !readU16(bytes, cursor + 30, &extraLen)
                || !readU16(bytes, cursor + 32, &commentLen)
                || !readU32(bytes, cursor + 42, &localOffset)) {
            if (error) {
                *error = QStringLiteral("zip");
            }
            return false;
        }
        const int nameAt = cursor + 46;
        if (nameAt + nameLen > bytes.size()) {
            if (error) {
                *error = QStringLiteral("zip");
            }
            return false;
        }
        const QString name = QString::fromUtf8(bytes.mid(nameAt, nameLen));
        cursor = nameAt + nameLen + extraLen + commentLen;
        if (name.endsWith(QLatin1Char('/'))) {
            continue;
        }
        if ((flags & 0x1) != 0 || method > 8 || method == 1
                || rawSize > static_cast<quint32>(kMaxImageBytes)
                || compSize > static_cast<quint32>(kMaxImageBytes)
                || !memberNameOk(name)) {
            if (error) {
                *error = QStringLiteral("entry");
            }
            return false;
        }
        if (localOffset + 30 > static_cast<quint32>(bytes.size())) {
            if (error) {
                *error = QStringLiteral("zip");
            }
            return false;
        }
        quint16 localName = 0;
        quint16 localExtra = 0;
        if (!readU16(bytes, static_cast<int>(localOffset) + 26, &localName)
                || !readU16(bytes, static_cast<int>(localOffset) + 28, &localExtra)) {
            if (error) {
                *error = QStringLiteral("zip");
            }
            return false;
        }
        const int dataAt = static_cast<int>(localOffset) + 30 + localName + localExtra;
        if (dataAt < 0 || dataAt + static_cast<int>(compSize) > bytes.size()) {
            if (error) {
                *error = QStringLiteral("zip");
            }
            return false;
        }
        const QByteArray compressed = bytes.mid(dataAt, static_cast<int>(compSize));
        ZipMember member;
        member.name = name;
        if (method == 0) {
            if (compSize != rawSize) {
                if (error) {
                    *error = QStringLiteral("entry");
                }
                return false;
            }
            member.data = compressed;
        } else {
            QString inflateError;
            if (!inflateRaw(compressed, static_cast<int>(rawSize), &member.data,
                            &inflateError)) {
                if (error) {
                    *error = inflateError;
                }
                return false;
            }
        }
        found.append(member);
    }
    if (members) {
        *members = found;
    }
    return true;
}

QString sharedPrefix(const QList<ZipMember> &members)
{
    if (members.isEmpty()) {
        return QString();
    }
    const int slash = members.at(0).name.indexOf(QLatin1Char('/'));
    if (slash <= 0) {
        return QString();
    }
    const QString prefix = members.at(0).name.left(slash + 1);
    for (int i = 1; i < members.size(); ++i) {
        if (!members.at(i).name.startsWith(prefix)) {
            return QString();
        }
    }
    return prefix;
}

const ZipMember *findMember(const QList<ZipMember> &members, const QString &name)
{
    for (int i = 0; i < members.size(); ++i) {
        if (members.at(i).name == name) {
            return &members.at(i);
        }
    }
    return 0;
}

bool insideDir(const QString &rootAbs, const QString &destAbs)
{
    const QString root = QDir::cleanPath(rootAbs);
    const QString dest = QDir::cleanPath(destAbs);
    return dest == root || dest.startsWith(root + QLatin1Char('/'));
}

bool copyTree(const QString &fromDir, const QString &toDir, QString *error)
{
    const QString root = QFileInfo(fromDir).canonicalFilePath();
    if (root.isEmpty()) {
        if (error) {
            *error = QStringLiteral("open");
        }
        return false;
    }
    QDir().mkpath(toDir);
    QDirIterator it(root, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString source = it.next();
        const QString canon = QFileInfo(source).canonicalFilePath();
        if (canon.isEmpty() || !insideDir(root, canon)) {
            continue;
        }
        QString rel = QDir(root).relativeFilePath(canon);
        rel.replace(QLatin1Char('\\'), QLatin1Char('/'));
        if (!memberNameOk(rel)) {
            if (error) {
                *error = QStringLiteral("entry");
            }
            return false;
        }
        const QString dest = QDir(toDir).filePath(rel);
        QDir().mkpath(QFileInfo(dest).absolutePath());
        QFile::remove(dest);
        if (!QFile::copy(canon, dest)) {
            if (error) {
                *error = QStringLiteral("copy");
            }
            return false;
        }
    }
    return true;
}

QColor fallbackColor(const QJsonObject &colors, const char *key, const QColor &fallback,
                     bool *fromPack)
{
    const QJsonValue value = colors.value(QLatin1String(key));
    if (!value.isString()) {
        return fallback;
    }
    const QColor parsed(value.toString().trimmed());
    if (!parsed.isValid()) {
        return fallback;
    }
    if (fromPack) {
        *fromPack = true;
    }
    return parsed;
}

QString baseNameOf(const QString &base)
{
    return base.compare(QLatin1String("dark"), Qt::CaseInsensitive) == 0
            ? QStringLiteral("dark") : QStringLiteral("light");
}

void fillLight(SkinPack *pack)
{
    pack->window = QColor("#F5F7FA");
    pack->windowText = QColor("#1F2937");
    pack->baseColor = QColor("#FFFFFF");
    pack->text = QColor("#1F2937");
    pack->button = QColor("#FFFFFF");
    pack->buttonText = QColor("#1F2937");
    pack->highlight = QColor("#2563EB");
    pack->highlightedText = QColor("#FFFFFF");
    pack->toolTipBase = QColor("#111827");
    pack->toolTipText = QColor("#FFFFFF");
    pack->placeholder = QColor("#9CA3AF");
    pack->bubbleInBg = QColor("#FFFFFF");
    pack->bubbleInFg = QColor("#1F2937");
    pack->bubbleOutBg = QColor("#2563EB");
    pack->bubbleOutFg = QColor("#FFFFFF");
    pack->bubbleMeta = QColor("#8A8F98");
}

void fillDark(SkinPack *pack)
{
    pack->window = QColor("#111827");
    pack->windowText = QColor("#F9FAFB");
    pack->baseColor = QColor("#1F2937");
    pack->text = QColor("#F9FAFB");
    pack->button = QColor("#1F2937");
    pack->buttonText = QColor("#F9FAFB");
    pack->highlight = QColor("#2563EB");
    pack->highlightedText = QColor("#FFFFFF");
    pack->toolTipBase = QColor("#374151");
    pack->toolTipText = QColor("#F9FAFB");
    pack->placeholder = QColor("#9CA3AF");
    pack->bubbleInBg = QColor("#1F2937");
    pack->bubbleInFg = QColor("#F9FAFB");
    pack->bubbleOutBg = QColor("#2563EB");
    pack->bubbleOutFg = QColor("#FFFFFF");
    pack->bubbleMeta = QColor("#9CA3AF");
}

bool imageSuffix(const QString &name)
{
    const QString lower = name.toLower();
    return lower.endsWith(QLatin1String(".png"))
            || lower.endsWith(QLatin1String(".jpg"))
            || lower.endsWith(QLatin1String(".jpeg"))
            || lower.endsWith(QLatin1String(".bmp"))
            || lower.endsWith(QLatin1String(".webp"));
}

QString cacheImage(const QString &id, const QString &fileName, const QByteArray &bytes)
{
    if (bytes.isEmpty() || bytes.size() > kMaxImageBytes || !imageSuffix(fileName)) {
        return QString();
    }
    const QString dir = skinsRoot() + QLatin1String("/.cache/") + id;
    QDir().mkpath(dir);
    const QString base = QFileInfo(fileName).fileName();
    const QString dest = QDir(dir).filePath(base);
    QSaveFile file(dest);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return QString();
    }
    if (file.write(bytes) != bytes.size() || !file.commit()) {
        return QString();
    }
    return dest;
}

SkinPack packFromJson(const QString &id, const QByteArray &jsonBytes,
                      const QString &qssText, bool qssPresent,
                      const QString &backgroundPath)
{
    SkinPack pack;
    pack.id = id;
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(jsonBytes, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        problemSlot() = QStringLiteral("skin.json");
        return pack;
    }
    const QJsonObject obj = doc.object();
    pack.found = true;
    pack.name = obj.value(QStringLiteral("name")).toString().trimmed();
    if (pack.name.isEmpty()) {
        pack.name = id;
    }
    if (pack.name.size() > 80) {
        pack.name = pack.name.left(80);
    }
    pack.base = baseNameOf(obj.value(QStringLiteral("base")).toString());
    if (pack.base == QLatin1String("dark")) {
        fillDark(&pack);
    } else {
        fillLight(&pack);
    }
    bool fromPack = false;
    const QJsonObject colors = obj.value(QStringLiteral("colors")).toObject();
    pack.window = fallbackColor(colors, "window", pack.window, &fromPack);
    pack.windowText = fallbackColor(colors, "windowText", pack.windowText, &fromPack);
    pack.baseColor = fallbackColor(colors, "base", pack.baseColor, &fromPack);
    pack.text = fallbackColor(colors, "text", pack.text, &fromPack);
    pack.button = fallbackColor(colors, "button", pack.button, &fromPack);
    pack.buttonText = fallbackColor(colors, "buttonText", pack.buttonText, &fromPack);
    pack.highlight = fallbackColor(colors, "highlight", pack.highlight, &fromPack);
    pack.highlightedText = fallbackColor(colors, "highlightedText",
                                         pack.highlightedText, &fromPack);
    pack.toolTipBase = fallbackColor(colors, "toolTipBase", pack.toolTipBase, &fromPack);
    pack.toolTipText = fallbackColor(colors, "toolTipText", pack.toolTipText, &fromPack);
    pack.placeholder = fallbackColor(colors, "placeholder", pack.placeholder, &fromPack);
    pack.bubbleInBg = fallbackColor(colors, "bubbleIncomingBg", pack.bubbleInBg, &fromPack);
    pack.bubbleInFg = fallbackColor(colors, "bubbleIncomingFg", pack.bubbleInFg, &fromPack);
    pack.bubbleOutBg = fallbackColor(colors, "bubbleOutgoingBg", pack.bubbleOutBg, &fromPack);
    pack.bubbleOutFg = fallbackColor(colors, "bubbleOutgoingFg", pack.bubbleOutFg, &fromPack);
    pack.bubbleMeta = fallbackColor(colors, "bubbleMeta", pack.bubbleMeta, &fromPack);
    pack.colorsFromPack = fromPack;
    pack.backgroundPath = backgroundPath;
    if (!qssPresent) {
        pack.qssOk = false;
        return pack;
    }
    QString qssProblem;
    pack.qssOk = UserSkin::qssUsable(qssText, &qssProblem);
    if (pack.qssOk) {
        pack.qss = qssText;
    } else {
        pack.qssProblem = qssProblem;
        problemSlot() = qssProblem;
    }
    return pack;
}

QString textMember(const ZipMember *member, int maxBytes)
{
    if (!member || member->data.size() > maxBytes || member->data.contains('\0')) {
        return QString();
    }
    return QString::fromUtf8(member->data);
}

} // namespace

void UserSkin::setRootForTest(const QString &dir)
{
    rootOverride() = dir;
    problemSlot().clear();
}

QString UserSkin::rootDir()
{
    return skinsRoot();
}

QString UserSkin::lastProblem()
{
    return problemSlot();
}

bool UserSkin::qssUsable(const QString &qss, QString *problem)
{
    if (qss.contains(QChar(0))) {
        if (problem) {
            *problem = QStringLiteral("nul");
        }
        return false;
    }
    if (qss.toUtf8().size() > kMaxQssBytes) {
        if (problem) {
            *problem = QStringLiteral("size");
        }
        return false;
    }
    const QString lower = qss.toLower();
    if (lower.contains(QLatin1String("@import"))
            || lower.contains(QLatin1String("expression("))
            || lower.contains(QLatin1String("javascript:"))
            || lower.contains(QLatin1String("http://"))
            || lower.contains(QLatin1String("https://"))
            || lower.contains(QLatin1String("url("))) {
        if (problem) {
            *problem = QStringLiteral("unsafe");
        }
        return false;
    }
    int depth = 0;
    bool inString = false;
    QChar quote;
    for (int i = 0; i < qss.size(); ++i) {
        const QChar c = qss.at(i);
        if (inString) {
            if (c == QLatin1Char('\\') && i + 1 < qss.size()) {
                ++i;
                continue;
            }
            if (c == quote) {
                inString = false;
            }
            continue;
        }
        if (c == QLatin1Char('"') || c == QLatin1Char('\'')) {
            inString = true;
            quote = c;
            continue;
        }
        if (c == QLatin1Char('{')) {
            ++depth;
        } else if (c == QLatin1Char('}')) {
            --depth;
            if (depth < 0) {
                if (problem) {
                    *problem = QStringLiteral("brace");
                }
                return false;
            }
        }
    }
    if (inString || depth != 0) {
        if (problem) {
            *problem = QStringLiteral("brace");
        }
        return false;
    }
    if (qss.trimmed().isEmpty()) {
        if (problem) {
            *problem = QStringLiteral("empty");
        }
        return false;
    }
    return true;
}

QList<SkinInfo> UserSkin::installed()
{
    QList<SkinInfo> list;
    QDir dir(skinsRoot());
    if (!dir.exists()) {
        return list;
    }
    const QFileInfoList entries = dir.entryInfoList(
                QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    for (int i = 0; i < entries.size(); ++i) {
        const QFileInfo info = entries.at(i);
        QString id;
        if (info.isDir()) {
            id = info.fileName();
            if (!skinIdOk(id) || id == QLatin1String(".cache")) {
                continue;
            }
            if (!QFileInfo::exists(info.filePath() + QLatin1String("/skin.json"))) {
                continue;
            }
        } else if (info.isFile() && info.suffix().compare(QLatin1String("zip"),
                                                          Qt::CaseInsensitive) == 0) {
            id = cleanId(info.completeBaseName());
            if (!skinIdOk(id)) {
                continue;
            }
            if (QFileInfo::exists(dir.filePath(id) + QLatin1String("/skin.json"))) {
                continue;
            }
            QList<ZipMember> members;
            if (!readZip(info.filePath(), &members, 0)) {
                continue;
            }
            const QString prefix = sharedPrefix(members);
            bool hasJson = findMember(members, QStringLiteral("skin.json")) != 0;
            if (!hasJson && !prefix.isEmpty()) {
                hasJson = findMember(members, prefix + QLatin1String("skin.json")) != 0;
            }
            if (!hasJson) {
                continue;
            }
        } else {
            continue;
        }
        const SkinPack pack = load(id);
        if (!pack.found) {
            continue;
        }
        SkinInfo row;
        row.id = id;
        row.name = pack.name;
        row.base = pack.base;
        list.append(row);
    }
    return list;
}

SkinPack UserSkin::load(const QString &idRaw)
{
    problemSlot().clear();
    SkinPack empty;
    const QString id = skinIdOk(idRaw) ? idRaw : cleanId(idRaw);
    if (!skinIdOk(id)) {
        problemSlot() = QStringLiteral("id");
        return empty;
    }
    const QString folder = skinsRoot() + QLatin1Char('/') + id;
    if (QFileInfo::exists(folder + QLatin1String("/skin.json"))) {
        QFile jsonFile(folder + QLatin1String("/skin.json"));
        if (!jsonFile.open(QIODevice::ReadOnly)) {
            problemSlot() = QStringLiteral("skin.json");
            return empty;
        }
        const QByteArray jsonBytes = jsonFile.read(kMaxJsonBytes + 1);
        if (jsonBytes.size() > kMaxJsonBytes) {
            problemSlot() = QStringLiteral("size");
            return empty;
        }
        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(jsonBytes, &parseError);
        QString qssName = QStringLiteral("theme.qss");
        QString imageName;
        if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
            const QJsonObject obj = doc.object();
            const QString named = obj.value(QStringLiteral("qss")).toString().trimmed();
            if (!named.isEmpty()) {
                qssName = QFileInfo(named).fileName();
            }
            imageName = QFileInfo(obj.value(QStringLiteral("background")).toString())
                    .fileName();
        }
        QString qssText;
        bool qssPresent = false;
        const QString qssPath = folder + QLatin1Char('/') + qssName;
        if (memberNameOk(qssName) && QFileInfo::exists(qssPath)) {
            QFile qssFile(qssPath);
            if (qssFile.open(QIODevice::ReadOnly)) {
                const QByteArray raw = qssFile.read(kMaxQssBytes + 1);
                if (raw.size() <= kMaxQssBytes && !raw.contains('\0')) {
                    qssText = QString::fromUtf8(raw);
                    qssPresent = true;
                } else {
                    qssPresent = true;
                    qssText = QStringLiteral("{");
                }
            }
        }
        QString background;
        if (!imageName.isEmpty() && memberNameOk(imageName) && imageSuffix(imageName)) {
            const QString imagePath = folder + QLatin1Char('/') + imageName;
            const QFileInfo imageInfo(imagePath);
            const QString canon = imageInfo.canonicalFilePath();
            const QString rootCanon = QFileInfo(folder).canonicalFilePath();
            if (imageInfo.exists() && imageInfo.size() <= kMaxImageBytes
                    && !canon.isEmpty() && !rootCanon.isEmpty()
                    && insideDir(rootCanon, canon)) {
                background = canon;
            }
        }
        return packFromJson(id, jsonBytes, qssText, qssPresent, background);
    }

    const QString zipPath = skinsRoot() + QLatin1Char('/') + id + QLatin1String(".zip");
    if (!QFileInfo::exists(zipPath)) {
        problemSlot() = QStringLiteral("missing");
        return empty;
    }
    QList<ZipMember> members;
    QString zipError;
    if (!readZip(zipPath, &members, &zipError)) {
        problemSlot() = zipError.isEmpty() ? QStringLiteral("zip") : zipError;
        return empty;
    }
    QString prefix = sharedPrefix(members);
    if (findMember(members, QStringLiteral("skin.json")) != 0) {
        prefix.clear();
    }
    const ZipMember *jsonMember = findMember(
                members, prefix + QLatin1String("skin.json"));
    const QString jsonText = textMember(jsonMember, kMaxJsonBytes);
    if (jsonText.isEmpty() || !jsonMember) {
        problemSlot() = QStringLiteral("skin.json");
        return empty;
    }
    QString qssName = QStringLiteral("theme.qss");
    QString imageName;
    const QJsonDocument doc = QJsonDocument::fromJson(jsonMember->data);
    if (doc.isObject()) {
        const QString named = doc.object().value(QStringLiteral("qss")).toString().trimmed();
        if (!named.isEmpty()) {
            qssName = QFileInfo(named).fileName();
        }
        imageName = QFileInfo(doc.object().value(QStringLiteral("background")).toString())
                .fileName();
    }
    const ZipMember *qssMember = memberNameOk(qssName)
            ? findMember(members, prefix + qssName) : 0;
    QString qssText;
    bool qssPresent = false;
    if (qssMember) {
        qssPresent = true;
        if (qssMember->data.size() <= kMaxQssBytes && !qssMember->data.contains('\0')) {
            qssText = QString::fromUtf8(qssMember->data);
        } else {
            qssText = QStringLiteral("{");
        }
    }
    QString background;
    if (!imageName.isEmpty() && memberNameOk(imageName) && imageSuffix(imageName)) {
        const ZipMember *imageMember = findMember(members, prefix + imageName);
        if (imageMember) {
            background = cacheImage(id, imageName, imageMember->data);
        }
    }
    return packFromJson(id, jsonMember->data, qssText, qssPresent, background);
}

bool UserSkin::importDirectory(const QString &sourceDir, QString *idOut, QString *error)
{
    const QFileInfo info(sourceDir);
    if (!info.isDir()) {
        if (error) {
            *error = QStringLiteral("open");
        }
        return false;
    }
    const QString id = cleanId(info.fileName());
    if (!skinIdOk(id)) {
        if (error) {
            *error = QStringLiteral("id");
        }
        return false;
    }
    if (!QFileInfo::exists(info.filePath() + QLatin1String("/skin.json"))) {
        if (error) {
            *error = QStringLiteral("skin.json");
        }
        return false;
    }
    const QString dest = skinsRoot() + QLatin1Char('/') + id;
    if (QFileInfo(dest).absoluteFilePath() == info.absoluteFilePath()) {
        if (idOut) {
            *idOut = id;
        }
        return load(id).found;
    }
    QDir(dest).removeRecursively();
    QString copyError;
    if (!copyTree(info.filePath(), dest, &copyError)) {
        if (error) {
            *error = copyError;
        }
        return false;
    }
    if (!load(id).found) {
        if (error) {
            *error = QStringLiteral("skin.json");
        }
        return false;
    }
    if (idOut) {
        *idOut = id;
    }
    return true;
}

bool UserSkin::importZip(const QString &zipPath, QString *idOut, QString *error)
{
    const QFileInfo info(zipPath);
    if (!info.isFile()) {
        if (error) {
            *error = QStringLiteral("open");
        }
        return false;
    }
    const QString id = cleanId(info.completeBaseName());
    if (!skinIdOk(id)) {
        if (error) {
            *error = QStringLiteral("id");
        }
        return false;
    }
    QList<ZipMember> members;
    QString zipError;
    if (!readZip(info.filePath(), &members, &zipError)) {
        if (error) {
            *error = zipError.isEmpty() ? QStringLiteral("zip") : zipError;
        }
        return false;
    }
    QString prefix = sharedPrefix(members);
    if (findMember(members, QStringLiteral("skin.json")) != 0) {
        prefix.clear();
    }
    if (!findMember(members, prefix + QLatin1String("skin.json"))) {
        if (error) {
            *error = QStringLiteral("skin.json");
        }
        return false;
    }
    const QString dest = skinsRoot() + QLatin1Char('/') + id;
    QDir(dest).removeRecursively();
    QDir().mkpath(dest);
    for (int i = 0; i < members.size(); ++i) {
        QString name = members.at(i).name;
        if (!prefix.isEmpty() && name.startsWith(prefix)) {
            name = name.mid(prefix.size());
        }
        if (!memberNameOk(name)) {
            if (error) {
                *error = QStringLiteral("entry");
            }
            return false;
        }
        const QString filePath = QDir(dest).filePath(name);
        QDir().mkpath(QFileInfo(filePath).absolutePath());
        QSaveFile out(filePath);
        if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            if (error) {
                *error = QStringLiteral("copy");
            }
            return false;
        }
        if (out.write(members.at(i).data) != members.at(i).data.size() || !out.commit()) {
            if (error) {
                *error = QStringLiteral("copy");
            }
            return false;
        }
    }
    if (!load(id).found) {
        if (error) {
            *error = lastProblem().isEmpty() ? QStringLiteral("skin.json") : lastProblem();
        }
        return false;
    }
    if (idOut) {
        *idOut = id;
    }
    return true;
}
