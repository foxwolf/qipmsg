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

#ifndef RECV_FILE_TRANSFER_H
#define RECV_FILE_TRANSFER_H

#include "recv_file_handle.h"

#include <QMutex>
#include <QWaitCondition>
#include <QObject>
#include <QTcpSocket>

#include <atomic>

class RecvFileMap;
struct TransferFile;

class QFile;

class RecvFileTransfer : public QObject
{
    Q_OBJECT

public:
    RecvFileTransfer(RecvFileMap *recvFileMap, QObject *parent = 0)
        : QObject(parent), m_recvFileMap(recvFileMap),
        isStopTransfer(false), isAbortTransfer(false) {}

    void resumeTransfer();

    void startTransfer();

signals:
    void recvFileFinished();
    void recvFileError(QString);

private slots:
    void stopTransfer();
    void abortTransfer();

private:
    QByteArray constructRecvFileDatagram(RecvFileHandle h);
    bool recvFileRegular(RecvFileHandle h);
    bool recvFileDir(RecvFileHandle h);
    bool saveData(QByteArray recvBlock, QFile &file);
    bool waitForSocket(int totalMs, bool readyRead);
    bool canParseHeader(QByteArray &recvBlock);
    bool parseHeader(QByteArray &recvBlock,
                     struct TransferFile &transferFile);
    void setLastModified(QString path, QString secondString);
    void setLastModified(RecvFileHandle h);
    void applyResumeIfNeeded(RecvFileHandle h);
    void persistResume(RecvFileHandle h);
    void clearResume(RecvFileHandle h);
    QString resumeKey(RecvFileHandle h) const;
    bool tryInstant(RecvFileHandle h);
    void noteReceivedFile(RecvFileHandle h, const QString &path);

    QMutex m_lock;
    QWaitCondition m_cond;
    RecvFileMap *m_recvFileMap;
    QString m_errorString;

    QTcpSocket m_tcpSocket;

    bool isStopTransfer;
    std::atomic<bool> isAbortTransfer;
};

#endif // !RECV_FILE_TRANSFER_H

