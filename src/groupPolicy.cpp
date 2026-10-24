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

#include "groupPolicy.h"
#include "helper.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>

namespace {

void appendUnique(QStringList *dest, const QJsonArray &array)
{
    for (int i = 0; i < array.size(); ++i) {
        const QString name = GroupPolicy::normalizeName(array.at(i).toString());
        if (!name.isEmpty() && !dest->contains(name)) {
            dest->append(name);
        }
    }
}

QJsonArray toJsonArray(const QStringList &names)
{
    QJsonArray array;
    foreach (const QString &name, names) {
        array.append(name);
    }
    return array;
}

} // namespace

GroupPolicy::GroupPolicy(QObject *parent)
    : QObject(parent)
{
    load();
}

QString GroupPolicy::normalizeName(const QString &name)
{
    return name.trimmed();
}

QString GroupPolicy::filePath() const
{
    return Helper::appHomePath() + "/group_policy.json";
}

void GroupPolicy::load()
{
    QMutexLocker locker(&m_lock);
    m_hidden.clear();
    m_muted.clear();

    QFile file(filePath());
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return;
    }
    const QJsonObject obj = doc.object();
    appendUnique(&m_hidden, obj.value("hidden").toArray());
    appendUnique(&m_muted, obj.value("muted").toArray());
    m_hidden.sort();
    m_muted.sort();
}

void GroupPolicy::persistUnlocked() const
{
    QJsonObject obj;
    obj.insert("hidden", toJsonArray(m_hidden));
    obj.insert("muted", toJsonArray(m_muted));

    QDir().mkpath(Helper::appHomePath());
    QFile file(filePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return;
    }
    file.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

bool GroupPolicy::isHidden(const QString &name) const
{
    const QString key = normalizeName(name);
    if (key.isEmpty()) {
        return false;
    }
    QMutexLocker locker(&m_lock);
    return m_hidden.contains(key);
}

bool GroupPolicy::isMuted(const QString &name) const
{
    const QString key = normalizeName(name);
    if (key.isEmpty()) {
        return false;
    }
    QMutexLocker locker(&m_lock);
    return m_muted.contains(key);
}

bool GroupPolicy::hasHidden() const
{
    QMutexLocker locker(&m_lock);
    return !m_hidden.isEmpty();
}

bool GroupPolicy::hasMuted() const
{
    QMutexLocker locker(&m_lock);
    return !m_muted.isEmpty();
}

QStringList GroupPolicy::hiddenGroups() const
{
    QMutexLocker locker(&m_lock);
    return m_hidden;
}

QStringList GroupPolicy::mutedGroups() const
{
    QMutexLocker locker(&m_lock);
    return m_muted;
}

bool GroupPolicy::setFlag(QStringList *list, const QString &name, bool on)
{
    const QString key = normalizeName(name);
    if (key.isEmpty() || !list) {
        return false;
    }
    QMutexLocker locker(&m_lock);
    const bool has = list->contains(key);
    if (on == has) {
        return false;
    }
    if (on) {
        list->append(key);
    } else {
        list->removeAll(key);
    }
    list->sort();
    persistUnlocked();
    locker.unlock();
    emit policyChanged();
    return true;
}

bool GroupPolicy::setHidden(const QString &name, bool hidden)
{
    return setFlag(&m_hidden, name, hidden);
}

bool GroupPolicy::setMuted(const QString &name, bool muted)
{
    return setFlag(&m_muted, name, muted);
}
