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

#include "discussion_store.h"
#include "global.h"
#include "helper.h"
#include "user_manager.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>

QString DiscussionMember::key() const
{
    const QString user = login.isEmpty() ? name : login;
    return QString("%1@%2/%3").arg(user, host, ip);
}

QString DiscussionMember::displayName() const
{
    if (!name.isEmpty()) {
        return name;
    }
    if (!login.isEmpty()) {
        return login;
    }
    return ip;
}

QStringList DiscussionGroup::memberIps() const
{
    QStringList ips;
    foreach (const DiscussionMember &member, members) {
        if (!member.ip.isEmpty() && !ips.contains(member.ip)) {
            ips << member.ip;
        }
    }
    return ips;
}

QStringList DiscussionGroup::mentionNames() const
{
    QStringList names;
    foreach (const DiscussionMember &member, members) {
        const QString n = member.displayName();
        if (!n.isEmpty() && !names.contains(n)) {
            names << n;
        }
    }
    return names;
}

QString DiscussionGroup::memberSummary() const
{
    QStringList names = mentionNames();
    if (names.size() > 4) {
        return QString("%1 +%2").arg(names.mid(0, 4).join(QString::fromUtf8("、")))
                .arg(names.size() - 4);
    }
    return names.join(QString::fromUtf8("、"));
}

DiscussionStore::DiscussionStore(QObject *parent)
    : QObject(parent), m_nextId(1)
{
    load();
}

QString DiscussionStore::filePath() const
{
    return Helper::appHomePath() + "/discussions.json";
}

QString DiscussionStore::historyKey(const QString &id)
{
    return QString("disc-%1").arg(id);
}

void DiscussionStore::load()
{
    QMutexLocker locker(&m_lock);
    m_groups.clear();
    m_nextId = 1;

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
        DiscussionGroup group;
        group.id = obj.value("id").toString();
        group.name = obj.value("name").toString();
        const QJsonArray mems = obj.value("members").toArray();
        for (int m = 0; m < mems.size(); ++m) {
            const QJsonObject mo = mems.at(m).toObject();
            DiscussionMember member;
            member.ip = mo.value("ip").toString();
            member.login = mo.value("login").toString();
            member.host = mo.value("host").toString();
            member.name = mo.value("name").toString();
            if (!member.ip.isEmpty()) {
                group.members.append(member);
            }
        }
        if (group.id.isEmpty() || group.name.isEmpty()) {
            continue;
        }
        m_groups.append(group);
        bool ok = false;
        const quint64 n = group.id.section('-', -1).toULongLong(&ok);
        if (ok && n >= m_nextId) {
            m_nextId = n + 1;
        }
    }
}

void DiscussionStore::persist() const
{
    QJsonArray array;
    foreach (const DiscussionGroup &group, m_groups) {
        QJsonObject obj;
        obj.insert("id", group.id);
        obj.insert("name", group.name);
        QJsonArray mems;
        foreach (const DiscussionMember &member, group.members) {
            QJsonObject mo;
            mo.insert("ip", member.ip);
            mo.insert("login", member.login);
            mo.insert("host", member.host);
            mo.insert("name", member.name);
            mo.insert("key", member.key());
            mems.append(mo);
        }
        obj.insert("members", mems);
        array.append(obj);
    }
    QDir().mkpath(Helper::appHomePath());
    QFile file(filePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return;
    }
    file.write(QJsonDocument(array).toJson(QJsonDocument::Compact));
}

int DiscussionStore::indexOfId(const QString &id) const
{
    for (int i = 0; i < m_groups.size(); ++i) {
        if (m_groups.at(i).id == id) {
            return i;
        }
    }
    return -1;
}

QList<DiscussionGroup> DiscussionStore::groups() const
{
    QMutexLocker locker(&m_lock);
    return m_groups;
}

DiscussionGroup DiscussionStore::groupById(const QString &id) const
{
    QMutexLocker locker(&m_lock);
    const int i = indexOfId(id);
    return i == -1 ? DiscussionGroup() : m_groups.at(i);
}

QString DiscussionStore::addGroup(const QString &name,
                                  const QList<DiscussionMember> &members)
{
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty() || members.isEmpty()) {
        return QString();
    }
    DiscussionGroup group;
    {
        QMutexLocker locker(&m_lock);
        group.id = QString("%1-%2")
                .arg(QDateTime::currentMSecsSinceEpoch())
                .arg(m_nextId++);
        group.name = trimmed;
        QStringList seen;
        foreach (const DiscussionMember &member, members) {
            if (member.ip.isEmpty() || seen.contains(member.ip)) {
                continue;
            }
            seen << member.ip;
            group.members.append(member);
        }
        if (group.members.size() < 2) {
            return QString();
        }
        m_groups.append(group);
        persist();
    }
    emit groupsChanged();
    return group.id;
}

bool DiscussionStore::renameGroup(const QString &id, const QString &name)
{
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty()) {
        return false;
    }
    {
        QMutexLocker locker(&m_lock);
        const int i = indexOfId(id);
        if (i < 0) {
            return false;
        }
        m_groups[i].name = trimmed;
        persist();
    }
    emit groupsChanged();
    return true;
}

bool DiscussionStore::removeGroup(const QString &id)
{
    {
        QMutexLocker locker(&m_lock);
        const int i = indexOfId(id);
        if (i < 0) {
            return false;
        }
        m_groups.removeAt(i);
        persist();
    }
    emit groupsChanged();
    return true;
}

bool DiscussionStore::addMembers(const QString &id,
                                 const QList<DiscussionMember> &members)
{
    bool changed = false;
    {
        QMutexLocker locker(&m_lock);
        const int i = indexOfId(id);
        if (i < 0) {
            return false;
        }
        QStringList seen;
        foreach (const DiscussionMember &m, m_groups.at(i).members) {
            seen << m.ip;
        }
        foreach (const DiscussionMember &member, members) {
            if (member.ip.isEmpty() || seen.contains(member.ip)) {
                continue;
            }
            seen << member.ip;
            m_groups[i].members.append(member);
            changed = true;
        }
        if (changed) {
            persist();
        }
    }
    if (changed) {
        emit groupsChanged();
    }
    return changed;
}

int DiscussionStore::count() const
{
    QMutexLocker locker(&m_lock);
    return m_groups.size();
}

DiscussionMember DiscussionStore::memberFromIp(const QString &ip)
{
    DiscussionMember member;
    member.ip = ip;
    if (!Global::userManager) {
        member.name = ip;
        return member;
    }
    const int row = Global::userManager->ipToRow(ip);
    member.name = Global::userManager->displayNameForIp(ip);
    if (row != -1) {
        member.login = Global::userManager->loginName(row);
        member.host = Global::userManager->host(row);
        if (member.name.isEmpty()) {
            member.name = Global::userManager->name(row);
        }
    }
    if (member.name.isEmpty()) {
        member.name = ip;
    }
    return member;
}
