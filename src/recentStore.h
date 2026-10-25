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

#ifndef RECENTSTORE_H
#define RECENTSTORE_H

#include <QDateTime>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QString>

// Lightweight recent-contact index. No Qt Sql.
// Updated from chat history writes and stored as recent.json.
// Discussion transcripts (disc-*) are not contacts.

struct RecentContact
{
    QString ip;
    QString name;
    QString text;
    QDateTime when;
};

class RecentStore : public QObject
{
    Q_OBJECT

public:
    explicit RecentStore(QObject *parent = 0);

    QList<RecentContact> items() const;
    void note(const QString &ip, const QString &name, const QString &text,
              const QDateTime &when);
    void seedFromHistoryDir(const QString &historyDir);

signals:
    void contactsChanged();

private:
    QString filePath() const;
    void load();
    void persistUnlocked() const;
    static QString snippetOf(const QString &text);
    static bool isContactIp(const QString &ip);

    mutable QMutex m_lock;
    QList<RecentContact> m_items;
    static const int m_maxItems;
};

#endif // !RECENTSTORE_H
