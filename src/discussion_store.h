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

#ifndef DISCUSSION_STORE_H
#define DISCUSSION_STORE_H

#include <QList>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QStringList>

class DiscussionMember
{
public:
    QString ip;
    QString login;
    QString host;
    QString name;

    QString key() const;
    QString displayName() const;
};

class DiscussionGroup
{
public:
    QString id;
    QString name;
    QString notice;
    QList<DiscussionMember> members;

    QStringList memberIps() const;
    QStringList mentionNames() const;
    QString memberSummary() const;
};

class DiscussionStore : public QObject
{
    Q_OBJECT

public:
    DiscussionStore(QObject *parent = 0);

    QList<DiscussionGroup> groups() const;
    DiscussionGroup groupById(const QString &id) const;
    QString addGroup(const QString &name, const QList<DiscussionMember> &members);
    bool renameGroup(const QString &id, const QString &name);
    bool removeGroup(const QString &id);
    bool addMembers(const QString &id, const QList<DiscussionMember> &members);
    bool setNotice(const QString &id, const QString &notice);
    int count() const;

    static void setStorageDirForTest(const QString &dir);

    static DiscussionMember memberFromIp(const QString &ip);
    static QString historyKey(const QString &id);

signals:
    void groupsChanged();

private:
    QString filePath() const;
    void load();
    void persist() const;
    int indexOfId(const QString &id) const;

    mutable QMutex m_lock;
    QList<DiscussionGroup> m_groups;
    quint64 m_nextId;
};

#endif // !DISCUSSION_STORE_H
