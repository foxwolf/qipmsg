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

#include "send_file_map.h"
#include "send_file.h"
#include "constants.h"
#include "helper.h"
#include "protocolutil.h"
#include "fileHashIndex.h"
#include "ipmsg.h"

#include <QStringList>
#include <QList>
#include <QDateTime>
#include <QtDebug>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

SendFileMap::SendFileMap(QObject *parent)
    : QObject(parent), m_transferedCount(0), m_state(NotTransfer),
      m_cancel(false), m_activeSocket(-1), m_bytesSent(0), m_bytesSkipped(0),
      m_bytesTotal(0), m_sizeUnknown(false), m_startedAtMs(0)
{
}

void SendFileMap::addFile(const QStringList &pathList)
{
    int id = 0;
    foreach (QString path, pathList) {
        SendFile sendFile(path);
        m_map.insert(id, SendFileHandle(sendFile));
        if (sendFile.isDir()) {
            m_sizeUnknown = true;
        } else {
            m_bytesTotal += sendFile.size();
        }
        ++id;
    }
}

void SendFileMap::setState(States state)
{
    if (state == Transfer && m_startedAtMs == 0) {
        m_startedAtMs = QDateTime::currentMSecsSinceEpoch();
    }
    m_state = state;
}

void SendFileMap::requestCancel()
{
    m_cancel = true;
    if (m_activeSocket < 0) {
        return;
    }
#ifdef _WIN32
    shutdown(static_cast<SOCKET>(m_activeSocket), SD_BOTH);
#else
    ::shutdown(static_cast<int>(m_activeSocket), SHUT_RDWR);
#endif
}

QString SendFileMap::etaText() const
{
    if (m_cancel) {
        return tr("Cancelling…");
    }
    if (m_bytesSent <= 0 && m_bytesSkipped <= 0) {
        return isTransfer() ? tr("Estimating…") : tr("Waiting");
    }
    const qint64 elapsed = m_startedAtMs <= 0
            ? 0 : QDateTime::currentMSecsSinceEpoch() - m_startedAtMs;
    const qint64 done = m_bytesSent + m_bytesSkipped;
    QString speed;
    if (m_bytesSent > 0 && elapsed >= 400) {
        const double perSec = (double)m_bytesSent * 1000.0 / (double)elapsed;
        speed = Helper::sizeStringUnit(perSec) + tr("/s");
    }
    if (m_sizeUnknown || m_bytesTotal <= 0) {
        return speed.isEmpty() ? tr("Estimating…")
                               : tr("%1 · remaining unknown").arg(speed);
    }
    const int secs = ProtocolUtil::etaSeconds(m_bytesSent,
                                              m_bytesTotal - m_bytesSkipped,
                                              elapsed);
    if (secs < 0) {
        if (done >= m_bytesTotal && !isTransfer()) {
            return tr("Done");
        }
        return speed.isEmpty() ? tr("Estimating…")
                               : tr("%1 · estimating").arg(speed);
    }
    QString remain;
    if (secs < 60) {
        remain = tr("%1 s left").arg(secs);
    } else if (secs < 3600) {
        remain = tr("%1 min %2 s left").arg(secs / 60).arg(secs % 60);
    } else {
        remain = tr("%1 h %2 min left").arg(secs / 3600).arg((secs % 3600) / 60);
    }
    if (speed.isEmpty()) {
        return remain;
    }
    return QString("%1 · %2").arg(speed, remain);
}

bool SendFileMap::markFileInstant(int fileId, QString *nameOut, QString *pathOut,
                                  qint64 *sizeOut, QString *hashOut)
{
    if (!m_map.contains(fileId)) {
        return false;
    }
    SendFileHandle handle = m_map.value(fileId);
    if (!handle->isFile() || handle->state() == SendFile::SendOk) {
        return false;
    }
    handle->setState(SendFile::SendOk);
    addBytesSkipped(handle->size());
    incrTransferCount();
    if (nameOut) {
        *nameOut = handle->fileName();
    }
    if (pathOut) {
        *pathOut = handle->absoluteFilePath();
    }
    if (sizeOut) {
        *sizeOut = handle->size();
    }
    if (hashOut) {
        *hashOut = handle->contentHash();
    }
    return true;
}

QString SendFileMap::packetString(bool withContentHash)
{
    QList<int> keys = m_map.keys();
    QString fileInfoString;

    foreach (int key, keys) {
        SendFileHandle h = m_map.value(key);

        QString s = QString("%1").arg(key);
        s.append(COMMAND_SEPERATOR);
        s.append(h->fileName().replace(":", "::"));
        s.append(COMMAND_SEPERATOR);
        s.append(QString("%1").arg(h->size(), 2, 16, QChar('0')));
        s.append(COMMAND_SEPERATOR);
        s.append(QString("%1").arg(Helper::dateTimeToSecsSinceEpoch(h->lastModified()),
                    0, 16));
        s.append(COMMAND_SEPERATOR);
        if (h->isFile()) {
            s.append(QString("%1").arg(IPMSG_FILE_REGULAR));
        } else if (h->isDir()) {
            s.append(QString("%1").arg(IPMSG_FILE_DIR));
        }
        s.append(COMMAND_SEPERATOR);
        s.append(QString("%1=").arg(IPMSG_FILE_MTIME, 0, 16));
        s.append(QString("%1").arg(Helper::dateTimeToSecsSinceEpoch(h->lastModified()), 0, 16));
        s.append(COMMAND_SEPERATOR);
        s.append(QString("%1=").arg(IPMSG_FILE_CREATETIME, 0, 16));
        s.append(QString("%1").arg(Helper::dateTimeToSecsSinceEpoch(
                    h->birthTime().isValid() ? h->birthTime() : h->lastModified()), 0, 16));
        s.append(COMMAND_SEPERATOR);
        if (withContentHash && h->isFile()) {
            QString sha = h->contentHash();
            if (sha.isEmpty()) {
                sha = FileHashIndex::hashFile(h->absoluteFilePath(), 0);
                h->setContentHash(sha);
            }
            if (sha.size() == 64) {
                s.append(QString("%1=%2").arg(IPMSG_FILE_QIPHASH, 0, 16).arg(sha));
                s.append(COMMAND_SEPERATOR);
            }
        }
        s.append(FILELIST_SEPARATOR);

        fileInfoString.append(s);
    }

    return fileInfoString;
}

QString SendFileMap::fileNames() const
{
    QStringList items;

    foreach (SendFileHandle h, m_map) {
        items << h->fileName();
    }

    return items.join(" ");
}

QString SendFileMap::sizeInfo() const
{
    int dirCount = 0;
    int regularFileCount = 0;
    qint64 size = 0;

    foreach (SendFileHandle h, m_map) {
        if (h->isDir()) {
            ++dirCount;
        } else {
            ++regularFileCount;
            size += h->size();
        }
    }

    QString s;
    if (regularFileCount > 0) {
        s = Helper::sizeStringUnit(size);
    }

    if (dirCount > 0) {
        s = s + (regularFileCount > 0 ? "/" : "") + QString("%1").arg(dirCount)
            + " " + (dirCount > 1 ? tr("Folders") : tr("Folder"));
    }

    return s;
}

QString SendFileMap::sendStats() const
{
    return QString("%1/%2/%3/").arg(m_map.count(), 1, 10)
        .arg(m_transferedCount)
        .arg(isTransfer() ? 1 : 0);
}

bool SendFileMap::isTransfer() const
{
    return (m_state == Transfer);
}

QString SendFileMap::recvUserInfo() const
{
    return m_recvUser + "(" + m_recvHostname + ")";
}

bool SendFileMap::canSendFile(int fileId) const
{
    if (m_map.contains(fileId)
        && m_map[fileId]->state() != SendFile::SendOk) {
        return true;
    }

    return false;
}

bool SendFileMap::isFinished() const
{
    foreach (SendFileHandle h, m_map) {
        if (h->state() != SendFile::SendOk) {
            return false;
        }
    }

    return true;
}

