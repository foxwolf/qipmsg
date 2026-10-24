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

#ifndef STICKER_STORE_H
#define STICKER_STORE_H

#include <QList>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QStringList>

class StickerGroup
{
public:
    QString id;
    QString name;
};

class StickerItem
{
public:
    QString id;
    QString groupId;
    QString fileName;
    QString title;
};

// User stickers live under ~/.qipmsg/stickers with an index at stickers.json.
// Built-in Unicode emoji are not stored here. Files are sent as IPMSG
// attachments (the same path as a pasted image), not as a private emoji opcode.
class StickerStore : public QObject
{
    Q_OBJECT

public:
    explicit StickerStore(QObject *parent = 0);

    QList<StickerGroup> groups() const;
    QList<StickerItem> stickers(const QString &groupId) const;
    QString pathFor(const StickerItem &item) const;
    bool containsGroup(const QString &id) const;

    QString addGroup(const QString &name);
    bool renameGroup(const QString &id, const QString &name);
    bool removeGroup(const QString &id);
    QString ensureDefaultGroup();

    QStringList importFiles(const QStringList &paths, const QString &groupId,
                            QString *usedGroupId = 0);
    bool removeSticker(const QString &id);
    bool moveSticker(const QString &id, const QString &groupId);

    static int maxStickerBytes();

signals:
    void stickersChanged();

private:
    QString indexPath() const;
    QString filesDir() const;
    void load();
    void persistUnlocked() const;
    int indexOfGroup(const QString &id) const;
    QString makeGroupId();
    QString makeStickerId();

    mutable QMutex m_lock;
    QList<StickerGroup> m_groups;
    QList<StickerItem> m_items;
    quint64 m_nextGroup;
    quint64 m_nextSticker;
};

#endif // !STICKER_STORE_H
