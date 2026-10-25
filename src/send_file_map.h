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

#ifndef SEND_FILE_MAP_H
#define SEND_FILE_MAP_H

#include "send_file_handle.h"

#include <QMap>
#include <QString>
#include <QStringList>
#include <QSemaphore>
#include <QtGlobal>


class SendFileMap : public QObject
{
    Q_OBJECT

public:
    friend class ServeSocket;

    enum States { NotTransfer, Transfer };

    SendFileMap(QObject *parent = 0);

    void addFile(const QStringList&);
    QString packetString() const;

    void setRecvUser(QString user) { m_recvUser = user; }
    void setRecvHostname(QString hostname) { m_recvHostname = hostname; }

    void setPacketNoString(QString s) { m_packetNoString = s; }
    QString packetNoString() const { return m_packetNoString; }

    QString fileNames() const;
    QString sizeInfo() const;
    QString sendStats() const;
    QString recvUserInfo() const;

    bool canSendFile(int fileId) const;

    void setState(States state);
    States state() const { return m_state; }

    void requestCancel();
    bool cancelRequested() const { return m_cancel; }
    void attachSocket(qintptr socketFd) { m_activeSocket = socketFd; }
    void detachSocket() { m_activeSocket = -1; }
    qintptr activeSocket() const { return m_activeSocket; }
    void addBytesSent(qint64 count) { m_bytesSent += count; }
    void addBytesSkipped(qint64 count) { m_bytesSkipped += count; }
    QString etaText() const;

    bool isFinished() const;
    bool isTransfer() const;

    int transferCount() const { return m_transferedCount; }
    void incrTransferCount() { ++m_transferedCount; }

private:
    QMap<int, SendFileHandle> m_map;

    QString m_recvUser;
    QString m_recvHostname;
    QString m_packetNoString;
    int m_transferedCount;
    States m_state;
    QSemaphore sem;
    bool m_cancel;
    qintptr m_activeSocket;
    qint64 m_bytesSent;
    qint64 m_bytesSkipped;
    qint64 m_bytesTotal;
    bool m_sizeUnknown;
    qint64 m_startedAtMs;
};

#endif // !SEND_FILE_MAP_H

