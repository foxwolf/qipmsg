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

#include "recentStore.h"
#include "helper.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>

#include <algorithm>

const int RecentStore::m_maxItems = 30;

namespace {

RecentContact contactFromJson(const QJsonObject &obj)
{
    RecentContact item;
    item.ip = obj.value("ip").toString().trimmed();
    item.name = obj.value("n").toString();
    item.text = obj.value("x").toString();
    item.when = QDateTime::fromString(obj.value("t").toString(), Qt::ISODate);
    return item;
}

} // namespace

RecentStore::RecentStore(QObject *parent)
    : QObject(parent)
{
    load();
}

QString RecentStore::filePath() const
{
    return Helper::appHomePath() + "/recent.json";
}

bool RecentStore::isContactIp(const QString &ip)
{
    const QString key = ip.trimmed();
    if (key.isEmpty() || key.startsWith(QLatin1String("disc-"))) {
        return false;
    }
    return key.contains(QLatin1Char('.'));
}

QString RecentStore::snippetOf(const QString &text)
{
    QString flat = text;
    flat.replace(QLatin1Char('\r'), QLatin1Char(' '));
    flat.replace(QLatin1Char('\n'), QLatin1Char(' '));
    flat = flat.simplified();
    if (flat.size() > 80) {
        flat = flat.left(79) + QStringLiteral("…");
    }
    return flat;
}

void RecentStore::load()
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
        const RecentContact item = contactFromJson(array.at(i).toObject());
        if (!isContactIp(item.ip) || !item.when.isValid()) {
            continue;
        }
        m_items.append(item);
    }
}

void RecentStore::persistUnlocked() const
{
    QDir dir(Helper::appHomePath());
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QJsonArray array;
    for (int i = 0; i < m_items.size(); ++i) {
        const RecentContact &item = m_items.at(i);
        QJsonObject obj;
        obj.insert("ip", item.ip);
        obj.insert("n", item.name);
        obj.insert("x", item.text);
        obj.insert("t", item.when.toString(Qt::ISODate));
        array.append(obj);
    }
    QFile file(filePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }
    file.write(QJsonDocument(array).toJson(QJsonDocument::Compact));
}

QList<RecentContact> RecentStore::items() const
{
    QMutexLocker locker(&m_lock);
    return m_items;
}

void RecentStore::note(const QString &ip, const QString &name,
                       const QString &text, const QDateTime &when)
{
    if (!isContactIp(ip)) {
        return;
    }
    QDateTime stamp = when;
    if (!stamp.isValid()) {
        stamp = QDateTime::currentDateTime();
    }

    {
        QMutexLocker locker(&m_lock);
        RecentContact item;
        item.ip = ip.trimmed();
        item.name = name;
        item.text = snippetOf(text);
        item.when = stamp;
        for (int i = 0; i < m_items.size(); ++i) {
            if (m_items.at(i).ip == item.ip) {
                if (item.name.isEmpty()) {
                    item.name = m_items.at(i).name;
                }
                m_items.removeAt(i);
                break;
            }
        }
        m_items.prepend(item);
        while (m_items.size() > m_maxItems) {
            m_items.removeLast();
        }
        persistUnlocked();
    }
    emit contactsChanged();
}

void RecentStore::seedFromHistoryDir(const QString &historyDir)
{
    {
        QMutexLocker locker(&m_lock);
        if (!m_items.isEmpty()) {
            return;
        }
    }

    QDir dir(historyDir);
    if (!dir.exists()) {
        return;
    }
    const QFileInfoList files = dir.entryInfoList(QStringList() << "*.jsonl",
                                                  QDir::Files);
    QList<RecentContact> found;
    foreach (const QFileInfo &info, files) {
        if (info.fileName().startsWith(QLatin1String("disc-"))) {
            continue;
        }
        QFile file(info.absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly)) {
            continue;
        }
        const qint64 size = file.size();
        const qint64 window = 16384;
        if (size > window) {
            file.seek(size - window);
        }
        QByteArray tail = file.readAll();
        if (size > window) {
            const int nl = tail.indexOf('\n');
            if (nl >= 0) {
                tail = tail.mid(nl + 1);
            }
        }
        const QList<QByteArray> lines = tail.split('\n');
        for (int i = lines.size() - 1; i >= 0; --i) {
            const QByteArray line = lines.at(i).trimmed();
            if (line.isEmpty()) {
                continue;
            }
            const QJsonDocument doc = QJsonDocument::fromJson(line);
            if (!doc.isObject()) {
                continue;
            }
            RecentContact item = contactFromJson(doc.object());
            if (item.ip.isEmpty()) {
                item.ip = info.completeBaseName();
            }
            if (!isContactIp(item.ip) || !item.when.isValid()) {
                break;
            }
            item.text = snippetOf(item.text);
            if (item.name.isEmpty()) {
                item.name = item.ip;
            }
            found.append(item);
            break;
        }
    }

    if (found.isEmpty()) {
        return;
    }
    std::sort(found.begin(), found.end(),
              [](const RecentContact &a, const RecentContact &b) {
                  return a.when > b.when;
              });

    QMutexLocker locker(&m_lock);
    if (!m_items.isEmpty()) {
        return;
    }
    for (int i = 0; i < found.size() && m_items.size() < m_maxItems; ++i) {
        bool seen = false;
        for (int j = 0; j < m_items.size(); ++j) {
            if (m_items.at(j).ip == found.at(i).ip) {
                seen = true;
                break;
            }
        }
        if (!seen) {
            m_items.append(found.at(i));
        }
    }
    persistUnlocked();
}
