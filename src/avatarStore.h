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

#include <QByteArray>
#include <QColor>
#include <QHash>
#include <QMutex>
#include <QObject>
#include <QPixmap>
#include <QString>

class QWidget;

// PNG copies under ~/.qipmsg/avatars. A manual file for an IP overrides
// the wire copy in avatars/wire/. "self" is this user. self-wire.png is
// the small PNG advertised to other qipmsg peers.

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

    QString selfWireId();
    QByteArray selfWireBytes();

    bool hasManual(const QString &key) const;
    bool hasWire(const QString &key) const;
    QString wireId(const QString &key) const;
    QString skipId(const QString &key) const;
    QString status(const QString &key) const;

    void setStatus(const QString &key, const QString &text);
    void setWireHidden(const QString &key, bool hidden);
    void writeSkip(const QString &key, const QString &id);
    void clearManual(const QString &key);
    void clearWire(const QString &key);
    bool saveWire(const QString &key, const QByteArray &png, const QString &id,
                  QString *error);

signals:
    void changed();
    void selfPhotoChanged();

private:
    QString fileForKey(const QString &key) const;
    QString wirePathFor(const QString &key) const;
    QString wireIdPath(const QString &key) const;
    QString skipPath(const QString &key) const;
    QString selfWirePath() const;
    bool ensureSelfWire();

    mutable QMutex m_lock;
    mutable QHash<QString, QPixmap> m_cache;
    QHash<QString, QString> m_status;
    QHash<QString, bool> m_wireHidden;
};

#endif // !AVATARSTORE_H
