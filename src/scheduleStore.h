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

#ifndef SCHEDULESTORE_H
#define SCHEDULESTORE_H

#include <QDateTime>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QString>

// Local timed reminders in ~/.qipmsg/schedule.json. No Qt Sql.
// A timer fires only while this process is running. Quit does not
// leave a daemon; the next launch catches a time that already passed.

struct ScheduleReminder
{
    QString id;
    QString text;
    QDateTime due;
    bool done;
    bool notifyPeer;
    QString peerIp;
    QString peerName;

    ScheduleReminder() : done(false), notifyPeer(false) {}
};

class ScheduleStore : public QObject
{
    Q_OBJECT

public:
    explicit ScheduleStore(QObject *parent = 0);

    QList<ScheduleReminder> items() const;
    QString add(const QString &text, const QDateTime &due, bool notifyPeer,
                const QString &peerIp, const QString &peerName);
    bool update(const QString &id, const QString &text, const QDateTime &due,
                bool notifyPeer, const QString &peerIp, const QString &peerName);
    bool remove(const QString &id);
    QList<ScheduleReminder> takeDue(const QDateTime &now);

signals:
    void remindersChanged();
    void reminderDue(QString text, QString peerIp, QString peerName,
                     bool notifyPeer);

private slots:
    void checkDue();

private:
    QString filePath() const;
    void load();
    bool persistUnlocked() const;
    bool evictUnlocked();
    static QString cleanText(const QString &text);

    mutable QMutex m_lock;
    QList<ScheduleReminder> m_items;
    static const int m_maxItems;
    static const int m_maxChars;
};

#endif // !SCHEDULESTORE_H
