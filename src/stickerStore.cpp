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

#include "stickerStore.h"
#include "helper.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>
#include <QRegularExpression>

namespace {

const int kMaxGroups = 40;
const int kMaxStickers = 400;

bool safeFileName(const QString &name)
{
    if (name.isEmpty() || name.startsWith(QLatin1Char('.'))) {
        return false;
    }
    if (name.contains(QLatin1Char('/')) || name.contains(QLatin1Char('\\'))) {
        return false;
    }
    return name != QLatin1String("..");
}

QString storedFileName(const QFileInfo &info, const QString &dirPath, const QString &id)
{
    QString base = info.fileName().trimmed();
    base.replace(QRegularExpression(QStringLiteral("[\\\\/:*?\"<>|\\r\\n\\t]")), QString());
    if (base.isEmpty() || base.startsWith(QLatin1Char('.'))) {
        base = id + QLatin1Char('.') + info.suffix().toLower();
    }
    if (base.size() > 80) {
        const QString suffix = QFileInfo(base).suffix();
        QString stem = QFileInfo(base).completeBaseName().left(60);
        base = suffix.isEmpty() ? stem : stem + QLatin1Char('.') + suffix;
    }
    return Helper::uniqueFileName(dirPath, base);
}

quint64 trailingNumber(const QString &id)
{
    int i = id.size();
    while (i > 0 && id.at(i - 1).isDigit()) {
        --i;
    }
    bool ok = false;
    const quint64 n = id.mid(i).toULongLong(&ok);
    return ok ? n : 0;
}

QString cleanTitle(const QString &raw, const QString &fallback)
{
    QString title = raw.trimmed();
    title.replace(QRegularExpression(QStringLiteral("[\\r\\n\\t]")), QStringLiteral(" "));
    if (title.size() > 40) {
        title = title.left(40);
    }
    if (title.isEmpty()) {
        return fallback;
    }
    return title;
}

} // namespace

int StickerStore::maxStickerBytes()
{
    return 8 * 1024 * 1024;
}

StickerStore::StickerStore(QObject *parent)
    : QObject(parent), m_nextGroup(1), m_nextSticker(1)
{
    load();
}

QString StickerStore::indexPath() const
{
    return Helper::appHomePath() + "/stickers.json";
}

QString StickerStore::filesDir() const
{
    return Helper::appHomePath() + "/stickers";
}

QString StickerStore::pathFor(const StickerItem &item) const
{
    if (!safeFileName(item.fileName)) {
        return QString();
    }
    return filesDir() + "/" + item.fileName;
}

void StickerStore::load()
{
    QMutexLocker locker(&m_lock);
    m_groups.clear();
    m_items.clear();
    m_nextGroup = 1;
    m_nextSticker = 1;

    QFile file(indexPath());
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return;
    }
    const QJsonObject root = doc.object();
    const QJsonArray groups = root.value("groups").toArray();
    for (int i = 0; i < groups.size(); ++i) {
        const QJsonObject obj = groups.at(i).toObject();
        StickerGroup group;
        group.id = obj.value("id").toString().trimmed();
        group.name = obj.value("name").toString().trimmed();
        if (group.id.isEmpty() || group.name.isEmpty() || !safeFileName(group.id)) {
            continue;
        }
        if (indexOfGroup(group.id) >= 0) {
            continue;
        }
        m_groups.append(group);
        const quint64 n = trailingNumber(group.id);
        if (n >= m_nextGroup) {
            m_nextGroup = n + 1;
        }
    }

    const QJsonArray items = root.value("stickers").toArray();
    for (int i = 0; i < items.size(); ++i) {
        const QJsonObject obj = items.at(i).toObject();
        StickerItem item;
        item.id = obj.value("id").toString().trimmed();
        item.groupId = obj.value("groupId").toString().trimmed();
        item.fileName = obj.value("file").toString().trimmed();
        item.title = cleanTitle(obj.value("title").toString(), item.id);
        if (item.id.isEmpty() || !safeFileName(item.fileName)
                || indexOfGroup(item.groupId) < 0) {
            continue;
        }
        if (!QFileInfo(filesDir() + "/" + item.fileName).isFile()) {
            continue;
        }
        m_items.append(item);
        const quint64 n = trailingNumber(item.id);
        if (n >= m_nextSticker) {
            m_nextSticker = n + 1;
        }
    }
}

void StickerStore::persistUnlocked() const
{
    QJsonArray groups;
    foreach (const StickerGroup &group, m_groups) {
        QJsonObject obj;
        obj.insert("id", group.id);
        obj.insert("name", group.name);
        groups.append(obj);
    }
    QJsonArray items;
    foreach (const StickerItem &item, m_items) {
        QJsonObject obj;
        obj.insert("id", item.id);
        obj.insert("groupId", item.groupId);
        obj.insert("file", item.fileName);
        obj.insert("title", item.title);
        items.append(obj);
    }
    QJsonObject root;
    root.insert("groups", groups);
    root.insert("stickers", items);

    QDir().mkpath(Helper::appHomePath());
    QFile file(indexPath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
}

int StickerStore::indexOfGroup(const QString &id) const
{
    for (int i = 0; i < m_groups.size(); ++i) {
        if (m_groups.at(i).id == id) {
            return i;
        }
    }
    return -1;
}

QString StickerStore::makeGroupId()
{
    const QString id = QString("g%1").arg(m_nextGroup);
    ++m_nextGroup;
    return id;
}

QString StickerStore::makeStickerId()
{
    const QString id = QString("s%1").arg(m_nextSticker);
    ++m_nextSticker;
    return id;
}

QList<StickerGroup> StickerStore::groups() const
{
    QMutexLocker locker(&m_lock);
    return m_groups;
}

QList<StickerItem> StickerStore::stickers(const QString &groupId) const
{
    QMutexLocker locker(&m_lock);
    QList<StickerItem> list;
    foreach (const StickerItem &item, m_items) {
        if (item.groupId == groupId) {
            list.append(item);
        }
    }
    return list;
}

bool StickerStore::containsGroup(const QString &id) const
{
    QMutexLocker locker(&m_lock);
    return indexOfGroup(id) >= 0;
}

QString StickerStore::addGroup(const QString &name)
{
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty() || trimmed.size() > 32) {
        return QString();
    }
    QMutexLocker locker(&m_lock);
    foreach (const StickerGroup &group, m_groups) {
        if (group.name == trimmed) {
            return group.id;
        }
    }
    if (m_groups.size() >= kMaxGroups) {
        return QString();
    }
    StickerGroup group;
    group.id = makeGroupId();
    group.name = trimmed;
    m_groups.append(group);
    persistUnlocked();
    locker.unlock();
    emit stickersChanged();
    return group.id;
}

bool StickerStore::renameGroup(const QString &id, const QString &name)
{
    const QString trimmed = name.trimmed();
    if (id.isEmpty() || trimmed.isEmpty() || trimmed.size() > 32) {
        return false;
    }
    QMutexLocker locker(&m_lock);
    const int index = indexOfGroup(id);
    if (index < 0) {
        return false;
    }
    for (int i = 0; i < m_groups.size(); ++i) {
        if (i != index && m_groups.at(i).name == trimmed) {
            return false;
        }
    }
    if (m_groups[index].name == trimmed) {
        return true;
    }
    m_groups[index].name = trimmed;
    persistUnlocked();
    locker.unlock();
    emit stickersChanged();
    return true;
}

bool StickerStore::removeGroup(const QString &id)
{
    QMutexLocker locker(&m_lock);
    const int index = indexOfGroup(id);
    if (index < 0) {
        return false;
    }
    QString fallback;
    for (int i = 0; i < m_groups.size(); ++i) {
        if (i != index) {
            fallback = m_groups.at(i).id;
            break;
        }
    }
    bool used = false;
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i).groupId == id) {
            used = true;
            if (fallback.isEmpty()) {
                return false;
            }
            m_items[i].groupId = fallback;
        }
    }
    Q_UNUSED(used);
    m_groups.removeAt(index);
    persistUnlocked();
    locker.unlock();
    emit stickersChanged();
    return true;
}

QString StickerStore::ensureDefaultGroup()
{
    {
        QMutexLocker locker(&m_lock);
        if (!m_groups.isEmpty()) {
            return m_groups.first().id;
        }
    }
    return addGroup(tr("My stickers"));
}

QStringList StickerStore::importFiles(const QStringList &paths,
                                      const QString &groupId,
                                      QString *usedGroupId)
{
    QString target = groupId;
    if (!containsGroup(target)) {
        target = ensureDefaultGroup();
    }
    if (target.isEmpty()) {
        return QStringList();
    }
    if (usedGroupId) {
        *usedGroupId = target;
    }

    QDir dir(filesDir());
    if (!dir.mkpath(QStringLiteral("."))) {
        return QStringList();
    }

    QStringList added;
    QList<StickerItem> pending;
    foreach (const QString &path, paths) {
        QFileInfo info(path);
        if (!info.isFile() || !Helper::isImageFile(path)) {
            continue;
        }
        if (info.size() <= 0 || info.size() > maxStickerBytes()) {
            continue;
        }
        const QString suffix = info.suffix().toLower();
        if (suffix != QLatin1String("png")
                && suffix != QLatin1String("gif")
                && suffix != QLatin1String("webp")
                && suffix != QLatin1String("jpg")
                && suffix != QLatin1String("jpeg")
                && suffix != QLatin1String("bmp")) {
            continue;
        }

        QString id;
        {
            QMutexLocker locker(&m_lock);
            if (m_items.size() + pending.size() >= kMaxStickers) {
                break;
            }
            id = makeStickerId();
        }
        const QString fileName = storedFileName(info, dir.path(), id);
        if (!safeFileName(fileName)) {
            continue;
        }
        const QString dest = dir.filePath(fileName);
        if (!QFile::copy(path, dest)) {
            continue;
        }
        StickerItem item;
        item.id = id;
        item.groupId = target;
        item.fileName = fileName;
        item.title = cleanTitle(info.completeBaseName(), id);
        pending.append(item);
        added << id;
    }

    if (pending.isEmpty()) {
        return added;
    }
    {
        QMutexLocker locker(&m_lock);
        if (indexOfGroup(target) < 0) {
            locker.unlock();
            foreach (const StickerItem &item, pending) {
                QFile::remove(dir.filePath(item.fileName));
            }
            return QStringList();
        }
        m_items.append(pending);
        persistUnlocked();
    }
    emit stickersChanged();
    return added;
}

bool StickerStore::removeSticker(const QString &id)
{
    QString fileName;
    {
        QMutexLocker locker(&m_lock);
        for (int i = 0; i < m_items.size(); ++i) {
            if (m_items.at(i).id == id) {
                fileName = m_items.at(i).fileName;
                m_items.removeAt(i);
                persistUnlocked();
                break;
            }
        }
    }
    if (!safeFileName(fileName)) {
        return false;
    }
    QFile::remove(filesDir() + "/" + fileName);
    emit stickersChanged();
    return true;
}

bool StickerStore::moveSticker(const QString &id, const QString &groupId)
{
    QMutexLocker locker(&m_lock);
    if (indexOfGroup(groupId) < 0) {
        return false;
    }
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i).id == id) {
            if (m_items.at(i).groupId == groupId) {
                return true;
            }
            m_items[i].groupId = groupId;
            persistUnlocked();
            locker.unlock();
            emit stickersChanged();
            return true;
        }
    }
    return false;
}
