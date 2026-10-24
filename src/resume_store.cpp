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

#include "resume_store.h"
#include "helper.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace {

qint64 jsonInt64(const QJsonValue &value)
{
    if (value.isString()) {
        bool ok = false;
        const qint64 parsed = value.toString().toLongLong(&ok);
        if (ok) {
            return parsed;
        }
        return 0;
    }
    if (value.isDouble()) {
        return static_cast<qint64>(value.toDouble());
    }
    return static_cast<qint64>(value.toVariant().toLongLong());
}

} // namespace

ResumeStore::ResumeStore()
{
    load();
}

QString ResumeStore::makeKey(const QString &ip, const QString &packetNo,
                             int fileId, const QString &name, qint64 size)
{
    return QString("%1|%2|%3|%4|%5").arg(ip, packetNo)
            .arg(fileId).arg(name).arg(size);
}

QString ResumeStore::filePath() const
{
    return Helper::appHomePath() + "/resume.json";
}

void ResumeStore::load()
{
    QMutexLocker locker(&m_lock);
    m_items.clear();

    QFile file(filePath());
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) {
        return;
    }

    const QJsonArray array = doc.array();
    for (int i = 0; i < array.size(); ++i) {
        const QJsonObject obj = array.at(i).toObject();
        ResumeRecord record;
        record.key = obj.value("key").toString();
        record.ip = obj.value("ip").toString();
        record.packetNo = obj.value("packetNo").toString();
        record.fileId = obj.value("fileId").toInt();
        record.name = obj.value("name").toString();
        record.size = jsonInt64(obj.value("size"));
        record.offset = jsonInt64(obj.value("offset"));
        record.saveDir = obj.value("saveDir").toString();
        if (!record.key.isEmpty()) {
            m_items.insert(record.key, record);
        }
    }
}

void ResumeStore::persist() const
{
    QJsonArray array;
    QMapIterator<QString, ResumeRecord> it(m_items);
    while (it.hasNext()) {
        it.next();
        const ResumeRecord &record = it.value();
        QJsonObject obj;
        obj.insert("key", record.key);
        obj.insert("ip", record.ip);
        obj.insert("packetNo", record.packetNo);
        obj.insert("fileId", record.fileId);
        obj.insert("name", record.name);
        obj.insert("size", QString::number(record.size));
        obj.insert("offset", QString::number(record.offset));
        obj.insert("saveDir", record.saveDir);
        array.append(obj);
    }

    QFile file(filePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return;
    }
    file.write(QJsonDocument(array).toJson(QJsonDocument::Compact));
}

ResumeRecord ResumeStore::lookup(const QString &key) const
{
    QMutexLocker locker(&m_lock);
    return m_items.value(key);
}

void ResumeStore::save(const ResumeRecord &record)
{
    QMutexLocker locker(&m_lock);
    if (record.key.isEmpty()) {
        return;
    }
    m_items.insert(record.key, record);
    persist();
}

void ResumeStore::remove(const QString &key)
{
    QMutexLocker locker(&m_lock);
    if (m_items.remove(key) > 0) {
        persist();
    }
}
