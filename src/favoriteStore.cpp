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

#include "favoriteStore.h"
#include "helper.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>
#include <QSaveFile>
#include <QUuid>

const int FavoriteStore::m_maxItems = 300;
const int FavoriteStore::m_maxBody = 4000;
const int FavoriteStore::m_maxTitle = 80;

namespace {

FavoriteEntry entryFromJson(const QJsonObject &obj)
{
    FavoriteEntry item;
    item.id = obj.value(QStringLiteral("id")).toString().trimmed();
    item.kind = obj.value(QStringLiteral("kind")).toString().trimmed();
    item.title = obj.value(QStringLiteral("title")).toString();
    item.body = obj.value(QStringLiteral("body")).toString();
    item.peerIp = obj.value(QStringLiteral("ip")).toString().trimmed();
    item.peerName = obj.value(QStringLiteral("name")).toString();
    item.when = QDateTime::fromString(obj.value(QStringLiteral("t")).toString(),
                                      Qt::ISODate);
    return item;
}

bool containsNeedle(const QString &hay, const QString &needle)
{
    return hay.contains(needle, Qt::CaseInsensitive);
}

} // namespace

FavoriteStore::FavoriteStore(QObject *parent)
    : QObject(parent)
{
    load();
}

bool FavoriteStore::kindOk(const QString &kind)
{
    return kind == QLatin1String("text")
            || kind == QLatin1String("file")
            || kind == QLatin1String("note");
}

QString FavoriteStore::cleanField(const QString &text, int limit)
{
    QString cleaned = text;
    cleaned.remove(QChar('\0'));
    cleaned = cleaned.trimmed();
    if (limit > 0 && cleaned.size() > limit) {
        cleaned = cleaned.left(limit);
    }
    return cleaned;
}

QString FavoriteStore::filePath() const
{
    return Helper::appHomePath() + QStringLiteral("/favorites.json");
}

void FavoriteStore::load()
{
    QMutexLocker locker(&m_lock);
    m_items.clear();
    QFile file(filePath());
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) {
        return;
    }
    const QJsonArray array = doc.array();
    for (int i = 0; i < array.size() && m_items.size() < m_maxItems; ++i) {
        if (!array.at(i).isObject()) {
            continue;
        }
        FavoriteEntry item = entryFromJson(array.at(i).toObject());
        item.title = cleanField(item.title, m_maxTitle);
        item.body = cleanField(item.body, m_maxBody);
        if (item.id.isEmpty() || !kindOk(item.kind) || item.body.isEmpty()) {
            continue;
        }
        if (item.title.isEmpty()) {
            item.title = cleanField(item.body, m_maxTitle);
        }
        if (!item.when.isValid()) {
            item.when = QDateTime::currentDateTime();
        }
        m_items.append(item);
    }
}

bool FavoriteStore::persistUnlocked() const
{
    QDir dir(Helper::appHomePath());
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        return false;
    }
    QJsonArray array;
    for (int i = 0; i < m_items.size(); ++i) {
        const FavoriteEntry &item = m_items.at(i);
        QJsonObject obj;
        obj.insert(QStringLiteral("id"), item.id);
        obj.insert(QStringLiteral("kind"), item.kind);
        obj.insert(QStringLiteral("title"), item.title);
        obj.insert(QStringLiteral("body"), item.body);
        if (!item.peerIp.isEmpty()) {
            obj.insert(QStringLiteral("ip"), item.peerIp);
        }
        if (!item.peerName.isEmpty()) {
            obj.insert(QStringLiteral("name"), item.peerName);
        }
        obj.insert(QStringLiteral("t"), item.when.toString(Qt::ISODate));
        array.append(obj);
    }
    QSaveFile file(filePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
    return file.commit();
}

QList<FavoriteEntry> FavoriteStore::items() const
{
    QMutexLocker locker(&m_lock);
    return m_items;
}

QList<FavoriteEntry> FavoriteStore::search(const QString &needle) const
{
    const QString key = needle.trimmed();
    QMutexLocker locker(&m_lock);
    if (key.isEmpty()) {
        return m_items;
    }
    QList<FavoriteEntry> found;
    for (int i = 0; i < m_items.size(); ++i) {
        const FavoriteEntry &item = m_items.at(i);
        if (containsNeedle(item.title, key)
                || containsNeedle(item.body, key)
                || containsNeedle(item.peerName, key)
                || containsNeedle(item.kind, key)
                || containsNeedle(item.peerIp, key)) {
            found.append(item);
        }
    }
    return found;
}

QString FavoriteStore::add(const QString &kind, const QString &title,
                           const QString &body, const QString &peerIp,
                           const QString &peerName)
{
    if (!kindOk(kind)) {
        return QString();
    }
    const QString cleanedBody = cleanField(body, m_maxBody);
    if (cleanedBody.isEmpty()) {
        return QString();
    }
    QString cleanedTitle = cleanField(title, m_maxTitle);
    if (cleanedTitle.isEmpty()) {
        cleanedTitle = cleanField(cleanedBody, m_maxTitle);
    }
    QMutexLocker locker(&m_lock);
    const QList<FavoriteEntry> snapshot = m_items;
    if (m_items.size() >= m_maxItems) {
        m_items.removeFirst();
    }
    FavoriteEntry item;
    item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    item.kind = kind;
    item.title = cleanedTitle;
    item.body = cleanedBody;
    item.peerIp = peerIp.trimmed();
    item.peerName = cleanField(peerName, m_maxTitle);
    item.when = QDateTime::currentDateTime();
    m_items.append(item);
    if (!persistUnlocked()) {
        m_items = snapshot;
        return QString();
    }
    const QString id = item.id;
    locker.unlock();
    emit favoritesChanged();
    return id;
}

bool FavoriteStore::remove(const QString &id)
{
    if (id.isEmpty()) {
        return false;
    }
    QMutexLocker locker(&m_lock);
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i).id != id) {
            continue;
        }
        const QList<FavoriteEntry> snapshot = m_items;
        m_items.removeAt(i);
        if (!persistUnlocked()) {
            m_items = snapshot;
            return false;
        }
        locker.unlock();
        emit favoritesChanged();
        return true;
    }
    return false;
}
