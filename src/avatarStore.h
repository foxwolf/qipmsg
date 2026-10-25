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

#ifndef AVATARSTORE_H
#define AVATARSTORE_H

#include <QColor>
#include <QHash>
#include <QObject>
#include <QPixmap>
#include <QString>

class QWidget;

// Local PNG copies under ~/.qipmsg/avatars. Nothing is put on the wire.
// "self" is this user. Any other key is a peer address stored only here.

class AvatarStore : public QObject
{
    Q_OBJECT

public:
    explicit AvatarStore(QObject *parent = 0);

    static QString selfKey();

    QPixmap rounded(const QString &key, int size, const QColor &status) const;
    bool setFromFile(const QString &key, const QString &source, QString *error);
    void clearKey(const QString &key);
    bool chooseAndSet(const QString &key, QWidget *parent);
    void noteFilesChanged();

signals:
    void changed();

private:
    QString fileForKey(const QString &key) const;

    mutable QHash<QString, QPixmap> m_cache;
};

#endif // !AVATARSTORE_H
