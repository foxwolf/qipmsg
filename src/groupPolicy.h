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

#ifndef GROUP_POLICY_H
#define GROUP_POLICY_H

#include <QMutex>
#include <QObject>
#include <QString>
#include <QStringList>

// Department-group flags (FeiQ group field), not local discussion groups.
// Persisted as JSON under the qipmsg data directory. No Qt Sql.
//
// Hide is best-effort IPMSG: there is no standard per-group presence bit.
// While any group is hidden, subnet BR_ENTRY is suppressed and known peers
// outside those groups are told by unicast. Members of a hidden group get
// unicast BR_EXIT and no ANSENTRY. A peer that ignores unicast exit, or
// that never sent an entry packet, can still keep or miss our presence.
// Mute is local only: history stays, alerts do not.

class GroupPolicy : public QObject
{
    Q_OBJECT

public:
    explicit GroupPolicy(QObject *parent = 0);

    static QString normalizeName(const QString &name);

    bool isHidden(const QString &name) const;
    bool isMuted(const QString &name) const;
    bool hasHidden() const;
    bool hasMuted() const;

    QStringList hiddenGroups() const;
    QStringList mutedGroups() const;

    bool setHidden(const QString &name, bool hidden);
    bool setMuted(const QString &name, bool muted);

signals:
    void policyChanged();

private:
    QString filePath() const;
    void load();
    void persistUnlocked() const;
    bool setFlag(QStringList *list, const QString &name, bool on);

    mutable QMutex m_lock;
    QStringList m_hidden;
    QStringList m_muted;
};

#endif // !GROUP_POLICY_H
