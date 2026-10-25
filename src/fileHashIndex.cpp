// This file is part of QIpMsg.
//
// QIpMsg is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "fileHashIndex.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QMutexLocker>
#include <QSaveFile>

#include <algorithm>

namespace {

const int DefaultCap = 2000;

struct HashRow
{
    QString sha256;
    qint64 size;
    QString path;
    QString name;
    QString kind;
    QString peer;
    qint64 seen;
};

QMutex g_hashLock;
QList<HashRow> g_rows;
bool g_loaded = false;
QString g_storeOverride;
int g_entryCap = DefaultCap;
qint64 g_lastSeen = 0;

QString defaultStoreFile()
{
    return QDir::homePath() + QLatin1String("/.qipmsg/filehash.json");
}

QString activeStoreFile()
{
    if (!g_storeOverride.isEmpty()) {
        return g_storeOverride;
    }
    return defaultStoreFile();
}

void pruneRows()
{
    QList<HashRow> kept;
    for (int i = 0; i < g_rows.size(); ++i) {
        const HashRow &row = g_rows.at(i);
        if (row.sha256.size() != 64 || row.size < 0 || row.path.isEmpty()) {
            continue;
        }
        if (!QFileInfo::exists(row.path)) {
            continue;
        }
        kept.append(row);
    }
    g_rows = kept;
    if (g_rows.size() <= g_entryCap) {
        return;
    }
    std::sort(g_rows.begin(), g_rows.end(),
              [](const HashRow &a, const HashRow &b) {
        return a.seen > b.seen;
    });
    g_rows = g_rows.mid(0, g_entryCap);
}

void persistUnlocked()
{
    pruneRows();
    QJsonArray array;
    for (int i = 0; i < g_rows.size(); ++i) {
        const HashRow &row = g_rows.at(i);
        QJsonObject object;
        object.insert(QStringLiteral("sha256"), row.sha256);
        object.insert(QStringLiteral("size"), QString::number(row.size));
        object.insert(QStringLiteral("path"), row.path);
        object.insert(QStringLiteral("name"), row.name);
        object.insert(QStringLiteral("kind"), row.kind);
        object.insert(QStringLiteral("peer"), row.peer);
        object.insert(QStringLiteral("seen"), QString::number(row.seen));
        array.append(object);
    }
    QJsonObject root;
    root.insert(QStringLiteral("v"), 1);
    root.insert(QStringLiteral("entries"), array);

    const QString path = activeStoreFile();
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.commit();
}

void loadUnlocked()
{
    if (g_loaded) {
        return;
    }
    g_loaded = true;
    g_rows.clear();
    QFile file(activeStoreFile());
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    const QJsonArray array = doc.object().value(QStringLiteral("entries")).toArray();
    for (int i = 0; i < array.size(); ++i) {
        const QJsonObject object = array.at(i).toObject();
        HashRow row;
        row.sha256 = object.value(QStringLiteral("sha256")).toString().toLower();
        row.size = object.value(QStringLiteral("size")).toString().toLongLong();
        if (row.size <= 0) {
            row.size = static_cast<qint64>(object.value(QStringLiteral("size")).toDouble());
        }
        row.path = object.value(QStringLiteral("path")).toString();
        row.name = object.value(QStringLiteral("name")).toString();
        row.kind = object.value(QStringLiteral("kind")).toString();
        row.peer = object.value(QStringLiteral("peer")).toString();
        row.seen = object.value(QStringLiteral("seen")).toString().toLongLong();
        if (row.seen <= 0) {
            row.seen = static_cast<qint64>(object.value(QStringLiteral("seen")).toDouble());
        }
        if (row.sha256.size() == 64 && row.size >= 0 && !row.path.isEmpty()) {
            g_rows.append(row);
        }
    }
}

QString indexedPathUnlocked(const QString &sha256, qint64 size, const QString &dir)
{
    QString best;
    qint64 bestSeen = -1;
    const QString prefix = dir.isEmpty()
            ? QString()
            : QDir(dir).absolutePath() + QLatin1Char('/');
    for (int i = 0; i < g_rows.size(); ++i) {
        const HashRow &row = g_rows.at(i);
        if (row.sha256 != sha256 || row.size != size) {
            continue;
        }
        if (!prefix.isEmpty()) {
            const QString abs = QFileInfo(row.path).absoluteFilePath();
            if (abs != QDir(dir).absolutePath() && !abs.startsWith(prefix)) {
                continue;
            }
        }
        if (!QFileInfo::exists(row.path) || QFileInfo(row.path).size() != size) {
            continue;
        }
        if (row.seen >= bestSeen) {
            bestSeen = row.seen;
            best = row.path;
        }
    }
    return best;
}

} // namespace

QString FileHashIndex::hashFile(const QString &path, qint64 *sizeOut)
{
    if (sizeOut) {
        *sizeOut = -1;
    }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    QCryptographicHash hash(QCryptographicHash::Sha256);
    while (!file.atEnd()) {
        const QByteArray chunk = file.read(1024 * 1024);
        if (chunk.isEmpty() && !file.atEnd()) {
            return QString();
        }
        hash.addData(chunk);
    }
    if (sizeOut) {
        *sizeOut = file.size();
    }
    return QString::fromLatin1(hash.result().toHex());
}

void FileHashIndex::note(const QString &path, const QString &sha256, qint64 size,
                         const QString &kind, const QString &peer)
{
    const QString sha = sha256.trimmed().toLower();
    if (sha.size() != 64 || size < 0 || path.isEmpty()) {
        return;
    }
    if (!QFileInfo::exists(path)) {
        return;
    }
    QMutexLocker locker(&g_hashLock);
    loadUnlocked();
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now <= g_lastSeen) {
        now = g_lastSeen + 1;
    }
    g_lastSeen = now;
    bool updated = false;
    for (int i = 0; i < g_rows.size(); ++i) {
        HashRow &row = g_rows[i];
        if (row.sha256 == sha && row.size == size) {
            row.path = QFileInfo(path).absoluteFilePath();
            row.name = QFileInfo(path).fileName();
            row.kind = kind;
            row.peer = peer;
            row.seen = now;
            updated = true;
            break;
        }
    }
    if (!updated) {
        HashRow row;
        row.sha256 = sha;
        row.size = size;
        row.path = QFileInfo(path).absoluteFilePath();
        row.name = QFileInfo(path).fileName();
        row.kind = kind;
        row.peer = peer;
        row.seen = now;
        g_rows.append(row);
    }
    persistUnlocked();
}

QString FileHashIndex::findExisting(const QString &sha256, qint64 size)
{
    const QString sha = sha256.trimmed().toLower();
    if (sha.size() != 64 || size < 0) {
        return QString();
    }
    QString indexed;
    {
        QMutexLocker locker(&g_hashLock);
        loadUnlocked();
        indexed = indexedPathUnlocked(sha, size, QString());
    }
    if (indexed.isEmpty()) {
        return QString();
    }
    qint64 hashedSize = -1;
    const QString got = hashFile(indexed, &hashedSize);
    if (got == sha && hashedSize == size) {
        return QFileInfo(indexed).absoluteFilePath();
    }
    return QString();
}

QString FileHashIndex::findInDirectory(const QString &dir, const QString &fileName,
                                       const QString &sha256, qint64 size)
{
    const QString sha = sha256.trimmed().toLower();
    if (sha.size() != 64 || size < 0 || dir.isEmpty()) {
        return QString();
    }
    if (!fileName.isEmpty() && !fileName.contains(QLatin1Char('/'))
            && !fileName.contains(QLatin1Char('\\'))
            && fileName != QLatin1String("..")
            && fileName != QLatin1String(".")) {
        const QString named = QDir(dir).filePath(fileName);
        const QFileInfo info(named);
        if (info.isFile() && info.size() == size) {
            qint64 hashedSize = -1;
            const QString got = hashFile(info.absoluteFilePath(), &hashedSize);
            if (got == sha && hashedSize == size) {
                return info.absoluteFilePath();
            }
        }
    }
    QMutexLocker locker(&g_hashLock);
    loadUnlocked();
    const QString indexed = indexedPathUnlocked(sha, size, dir);
    locker.unlock();
    if (indexed.isEmpty()) {
        return QString();
    }
    qint64 hashedSize = -1;
    const QString got = hashFile(indexed, &hashedSize);
    if (got == sha && hashedSize == size) {
        return QFileInfo(indexed).absoluteFilePath();
    }
    return QString();
}

void FileHashIndex::setStoreFile(const QString &path)
{
    QMutexLocker locker(&g_hashLock);
    g_storeOverride = path;
    g_loaded = false;
    g_rows.clear();
}

void FileHashIndex::setEntryCap(int cap)
{
    QMutexLocker locker(&g_hashLock);
    g_entryCap = cap < 1 ? 1 : cap;
}

int FileHashIndex::entryCount()
{
    QMutexLocker locker(&g_hashLock);
    loadUnlocked();
    return g_rows.size();
}

QString FileHashIndex::storeFile()
{
    QMutexLocker locker(&g_hashLock);
    return activeStoreFile();
}
