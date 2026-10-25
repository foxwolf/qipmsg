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

#include "send_file_manager.h"
#include "fileHashIndex.h"
#include "global.h"
#include "helper.h"
#include "window_manager.h"

#include <QMutexLocker>

SendFileManager::~SendFileManager()
{
    // XXX NOTE: program is abort to quit. so we let the os to care about the
    // memory.
    // foreach (SendFileMap *m, transferFileMap) {
    //     delete m;
    //     m = 0;
    // }
}

void SendFileManager::addTransfer(QString key, SendFileMap *value)
{
    transferFileMap.insert(key, value);
    transferFileModel.insertTransfer(value);

    emit transferCountChanged(transferFileMap.count());
}

void SendFileManager::addTransferLocked(QString key, SendFileMap *value)
{
    // XXX NOTE: we need lock here, 'SendFileThread' call removeTransfer()
    // directly
    QMutexLocker locker(&m_lock);

    transferFileMap.insert(key, value);
    transferFileModel.insertTransfer(value);

    emit transferCountChanged(transferFileMap.count());
}

void SendFileManager::removeTransfer(QString key)
{
    // XXX NOTE: we need lock here, 'SendFileThread' call this directly.
    // QMutexLocker locker(&m_lock);

    SendFileMap *map = transferFileMap.value(key);
    transferFileMap.remove(key);
    if (map) {
        delete map;
    }

    transferFileModel.removeRow(key);

    emit transferCountChanged(transferFileMap.count());
}

void SendFileManager::cancelTransfer(const QString &key)
{
    QMutexLocker locker(&m_lock);
    SendFileMap *map = transferFileMap.value(key);
    if (!map) {
        return;
    }
    map->requestCancel();
    transferFileModel.updateTransfer(key);
    if (!map->isTransfer() && map->activeSocket() < 0) {
        removeTransfer(key);
    }
}

bool SendFileManager::finishIfCancelled(const QString &key)
{
    QMutexLocker locker(&m_lock);
    SendFileMap *map = transferFileMap.value(key);
    if (!map) {
        return true;
    }
    if (!map->cancelRequested()) {
        return false;
    }
    removeTransfer(key);
    return true;
}

void SendFileManager::markInstant(QString packetField, int fileId, QString sha,
                                  QString ip)
{
    QString key;
    QString name;
    QString path;
    qint64 size = 0;
    QString cached;
    {
        QMutexLocker locker(&m_lock);
        const QStringList keys = Helper::packetNoLookupKeys(packetField.toLatin1());
        SendFileMap *map = 0;
        foreach (const QString &candidate, keys) {
            map = transferFileMap.value(candidate);
            if (map) {
                key = candidate;
                break;
            }
        }
        if (!map || !map->markFileInstant(fileId, &name, &path, &size, &cached)) {
            return;
        }
        transferFileModel.updateTransfer(key);
        if (map->isFinished()) {
            removeTransfer(key);
        }
    }
    const QString digest = cached.isEmpty() ? sha : cached;
    if (!digest.isEmpty()) {
        FileHashIndex::note(path, digest, size, QStringLiteral("send"), ip);
    }
    if (Global::windowManager) {
        Global::windowManager->noteInstantSkip(ip, name, false);
    }
}

void SendFileManager::removeTransferLocked(QString key)
{
    QMutexLocker locker(&m_lock);

    SendFileMap *map = transferFileMap.value(key);
    transferFileMap.remove(key);
    if (map) {
        delete map;
    }

    transferFileModel.removeRow(key);

    emit transferCountChanged(transferFileMap.count());
}

