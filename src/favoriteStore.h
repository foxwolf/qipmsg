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

#ifndef FAVORITESTORE_H
#define FAVORITESTORE_H

#include <QDateTime>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QString>

// Local favorites in ~/.qipmsg/favorites.json. No cloud and no Qt Sql.
// kind is "text", "file", or "note".

struct FavoriteEntry
{
    QString id;
    QString kind;
    QString title;
    QString body;
    QString peerIp;
    QString peerName;
    QDateTime when;
};

class FavoriteStore : public QObject
{
    Q_OBJECT

public:
    explicit FavoriteStore(QObject *parent = 0);

    static bool kindOk(const QString &kind);

    QList<FavoriteEntry> items() const;
    QList<FavoriteEntry> search(const QString &needle) const;
    QString add(const QString &kind, const QString &title, const QString &body,
                const QString &peerIp, const QString &peerName);
    bool remove(const QString &id);

signals:
    void favoritesChanged();

private:
    QString filePath() const;
    void load();
    bool persistUnlocked() const;
    static QString cleanField(const QString &text, int limit);

    mutable QMutex m_lock;
    QList<FavoriteEntry> m_items;
    static const int m_maxItems;
    static const int m_maxBody;
    static const int m_maxTitle;
};

#endif // !FAVORITESTORE_H
