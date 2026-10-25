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

#include "scheduleStore.h"
#include "helper.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>
#include <QSaveFile>
#include <QTimer>
#include <QUuid>

const int ScheduleStore::m_maxItems = 200;
const int ScheduleStore::m_maxChars = 500;

namespace {

ScheduleReminder reminderFromJson(const QJsonObject &obj)
{
    ScheduleReminder item;
    item.id = obj.value(QStringLiteral("id")).toString().trimmed();
    item.text = obj.value(QStringLiteral("text")).toString();
    item.due = QDateTime::fromString(obj.value(QStringLiteral("due")).toString(),
                                     Qt::ISODate);
    item.done = obj.value(QStringLiteral("done")).toBool();
    item.notifyPeer = obj.value(QStringLiteral("notify")).toBool();
    item.peerIp = obj.value(QStringLiteral("ip")).toString().trimmed();
    item.peerName = obj.value(QStringLiteral("name")).toString();
    return item;
}

} // namespace

ScheduleStore::ScheduleStore(QObject *parent)
    : QObject(parent)
{
    load();
    QTimer *timer = new QTimer(this);
    timer->setInterval(15000);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkDue()));
    timer->start();
    QTimer::singleShot(1500, this, SLOT(checkDue()));
}

QString ScheduleStore::cleanText(const QString &text)
{
    QString cleaned = text;
    cleaned.remove(QChar('\0'));
    cleaned = cleaned.trimmed();
    if (cleaned.size() > m_maxChars) {
        cleaned = cleaned.left(m_maxChars);
    }
    return cleaned;
}

QString ScheduleStore::filePath() const
{
    return Helper::appHomePath() + QStringLiteral("/schedule.json");
}

void ScheduleStore::load()
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
        ScheduleReminder item = reminderFromJson(array.at(i).toObject());
        item.text = cleanText(item.text);
        if (item.id.isEmpty() || item.text.isEmpty() || !item.due.isValid()) {
            continue;
        }
        if (!item.notifyPeer) {
            item.peerIp.clear();
        }
        m_items.append(item);
    }
}

bool ScheduleStore::persistUnlocked() const
{
    QDir dir(Helper::appHomePath());
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        return false;
    }
    QJsonArray array;
    for (int i = 0; i < m_items.size(); ++i) {
        const ScheduleReminder &item = m_items.at(i);
        QJsonObject obj;
        obj.insert(QStringLiteral("id"), item.id);
        obj.insert(QStringLiteral("text"), item.text);
        obj.insert(QStringLiteral("due"), item.due.toString(Qt::ISODate));
        obj.insert(QStringLiteral("done"), item.done);
        obj.insert(QStringLiteral("notify"), item.notifyPeer);
        if (!item.peerIp.isEmpty()) {
            obj.insert(QStringLiteral("ip"), item.peerIp);
        }
        if (!item.peerName.isEmpty()) {
            obj.insert(QStringLiteral("name"), item.peerName);
        }
        array.append(obj);
    }
    QSaveFile file(filePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(QJsonDocument(array).toJson(QJsonDocument::Indented));
    return file.commit();
}

bool ScheduleStore::evictUnlocked()
{
    bool removed = false;
    while (m_items.size() > m_maxItems) {
        int oldestDone = -1;
        for (int i = 0; i < m_items.size(); ++i) {
            if (!m_items.at(i).done) {
                continue;
            }
            if (oldestDone < 0
                    || m_items.at(i).due < m_items.at(oldestDone).due) {
                oldestDone = i;
            }
        }
        if (oldestDone < 0) {
            break;
        }
        m_items.removeAt(oldestDone);
        removed = true;
    }
    return removed;
}

QList<ScheduleReminder> ScheduleStore::items() const
{
    QMutexLocker locker(&m_lock);
    return m_items;
}

QString ScheduleStore::add(const QString &text, const QDateTime &due,
                           bool notifyPeer, const QString &peerIp,
                           const QString &peerName)
{
    const QString cleaned = cleanText(text);
    if (cleaned.isEmpty() || !due.isValid()) {
        return QString();
    }
    QMutexLocker locker(&m_lock);
    int pending = 0;
    for (int i = 0; i < m_items.size(); ++i) {
        if (!m_items.at(i).done) {
            ++pending;
        }
    }
    if (pending >= m_maxItems) {
        return QString();
    }
    const QList<ScheduleReminder> snapshot = m_items;
    ScheduleReminder item;
    item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    item.text = cleaned;
    item.due = due;
    item.done = false;
    item.notifyPeer = notifyPeer && !peerIp.trimmed().isEmpty();
    item.peerIp = item.notifyPeer ? peerIp.trimmed() : QString();
    item.peerName = item.notifyPeer ? peerName.trimmed() : QString();
    m_items.append(item);
    evictUnlocked();
    if (!persistUnlocked()) {
        m_items = snapshot;
        return QString();
    }
    const QString id = item.id;
    locker.unlock();
    emit remindersChanged();
    return id;
}

bool ScheduleStore::update(const QString &id, const QString &text,
                           const QDateTime &due, bool notifyPeer,
                           const QString &peerIp, const QString &peerName)
{
    const QString cleaned = cleanText(text);
    if (id.isEmpty() || cleaned.isEmpty() || !due.isValid()) {
        return false;
    }
    QMutexLocker locker(&m_lock);
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i).id != id) {
            continue;
        }
        const QList<ScheduleReminder> snapshot = m_items;
        ScheduleReminder &item = m_items[i];
        const QDateTime previousDue = item.due;
        const bool previousDone = item.done;
        item.text = cleaned;
        item.due = due;
        item.notifyPeer = notifyPeer && !peerIp.trimmed().isEmpty();
        item.peerIp = item.notifyPeer ? peerIp.trimmed() : QString();
        item.peerName = item.notifyPeer ? peerName.trimmed() : QString();
        if (due > QDateTime::currentDateTime() || due != previousDue) {
            item.done = false;
        } else {
            item.done = previousDone;
        }
        if (!persistUnlocked()) {
            m_items = snapshot;
            return false;
        }
        locker.unlock();
        emit remindersChanged();
        return true;
    }
    return false;
}

bool ScheduleStore::remove(const QString &id)
{
    if (id.isEmpty()) {
        return false;
    }
    QMutexLocker locker(&m_lock);
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i).id != id) {
            continue;
        }
        const QList<ScheduleReminder> snapshot = m_items;
        m_items.removeAt(i);
        if (!persistUnlocked()) {
            m_items = snapshot;
            return false;
        }
        locker.unlock();
        emit remindersChanged();
        return true;
    }
    return false;
}

QList<ScheduleReminder> ScheduleStore::takeDue(const QDateTime &now)
{
    QMutexLocker locker(&m_lock);
    QList<ScheduleReminder> due;
    if (!now.isValid()) {
        return due;
    }
    bool changed = false;
    for (int i = 0; i < m_items.size(); ++i) {
        ScheduleReminder &item = m_items[i];
        if (item.done || !item.due.isValid() || item.due > now) {
            continue;
        }
        due.append(item);
        item.done = true;
        changed = true;
    }
    if (changed) {
        persistUnlocked();
    }
    return due;
}

void ScheduleStore::checkDue()
{
    const QList<ScheduleReminder> due = takeDue(QDateTime::currentDateTime());
    if (due.isEmpty()) {
        return;
    }
    emit remindersChanged();
    for (int i = 0; i < due.size(); ++i) {
        const ScheduleReminder &item = due.at(i);
        emit reminderDue(item.text, item.peerIp, item.peerName, item.notifyPeer);
    }
}
