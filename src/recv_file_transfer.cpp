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

#include "recv_file_transfer.h"
#include "recv_file_handle.h"
#include "recv_file_map.h"
#include "constants.h"
#include "helper.h"
#include "global.h"
#include "user_manager.h"
#include "transfer_codec.h"
#include "preferences.h"
#include "resume_store.h"
#include "fileHashIndex.h"
#include "ipmsg.h"
#include "msg_thread.h"
#include "send_msg.h"
#include "msg.h"
#include "window_manager.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextCodec>

#ifdef Q_OS_WIN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <sys/utime.h>
#else
#include <sys/socket.h>
#include <utime.h>
#endif

struct TransferFile
{
    QString name;
    qint64 size;
    int type;
    QMap<int, QString> extendAttr;
};

void RecvFileTransfer::startTransfer()
{
    m_recvFileMap->setStartTime();
    m_recvFileMap->resetStats();

    foreach (RecvFileHandle h, m_recvFileMap->m_map) {
        if (h->state() == RecvFile::RecvOk
            || h->state() == RecvFile::NotRecv) {
            continue;
        }

        h->resetStats();

        m_recvFileMap->setCurrentId(h->fileId());
        m_recvFileMap->startTimer();

        if (tryInstant(h)) {
            continue;
        }

        m_tcpSocket.disconnectFromHost();
        quint16 port = h->peerPort();
        if (port == 0) {
            port = Global::userManager->peerPort(h->ip());
        }
        if (port == 0) {
            port = IPMSG_DEFAULT_PORT;
        }
        m_tcpSocket.connectToHost(h->ipAddress(), port);
        {
            int waited = 0;
            bool connected = false;
            while (waited < TCP_CONNECT_TIMEOUT_MS) {
                if (isAbortTransfer.load()) {
                    goto transfer_fail;
                }
                if (m_tcpSocket.waitForConnected(200)) {
                    connected = true;
                    break;
                }
                if (m_tcpSocket.state() == QAbstractSocket::UnconnectedState) {
                    break;
                }
                waited += 200;
            }
            if (!connected) {
                m_errorString = m_tcpSocket.errorString();
                goto transfer_fail;
            }
        }

        if (h->type() == IPMSG_FILE_REGULAR) {
            applyResumeIfNeeded(h);
            if (!recvFileRegular(h)) {
                goto transfer_fail;
            }
        } else if (h->type() == IPMSG_FILE_DIR) {
            if (!recvFileDir(h)) {
                goto transfer_fail;
            }
        }

        m_recvFileMap->stopTimer();
    }

    m_tcpSocket.disconnectFromHost();
    m_recvFileMap->setEndTime();
    emit recvFileFinished();
    return;

transfer_fail:
    m_tcpSocket.disconnectFromHost();
    m_recvFileMap->setEndTime();
    m_recvFileMap->stopTimer();
    // if 'isAbortTransfer' is true, it's a manually stop, not emit error
    if (!isAbortTransfer.load()) {
        emit recvFileError(m_errorString);
    }
    return;
}

bool RecvFileTransfer::recvFileRegular(RecvFileHandle h)
{
    h->setStartTime();

    QByteArray sendBlock(constructRecvFileDatagram(h));
    m_tcpSocket.write(sendBlock);
    if (!waitForSocket(3000, false)) {
        return false;
    }

    m_recvFileMap->setCurrentChildName(h->saveName());
    QFile file(m_recvFileMap->saveFilePath() + "/" + h->saveName());
    QFlags<QIODevice::OpenModeFlag> flags;
    if (m_recvFileMap->state() == RecvFileMap::Retry) {
        flags = QIODevice::WriteOnly | QIODevice::Append;
    } else {
        flags = QIODevice::WriteOnly;
    }
    if (!file.open(flags)) {
        m_errorString = file.errorString();
        return false;
    }

    qint64 bytesReaded = 0;
    const qint64 startOffset = h->offset();
    qint64 bytesToRead;
    if (m_recvFileMap->state() == RecvFileMap::Retry) {
        bytesToRead = h->size() - startOffset;
    } else {
        bytesToRead = h->size();
    }
    qint64 lastPersist = 0;
    while (bytesReaded < bytesToRead) {
        if (!waitForSocket(3000, true)) {
            goto recv_file_error;
        }

        const qint64 remain = bytesToRead - bytesReaded;
        const qint64 toRead = qMin(m_tcpSocket.bytesAvailable(), remain);
        if (toRead <= 0) {
            break;
        }

        const QByteArray chunk = m_tcpSocket.read(toRead);
        if (chunk.isEmpty()) {
            m_errorString = m_tcpSocket.errorString();
            goto recv_file_error;
        }
        bytesReaded += chunk.size();

        if (!saveData(chunk, file)) {
            goto recv_file_error;
        }

        // we need this because QTcpSocket error signal may happend any time
        h->setOffset(startOffset + bytesReaded);
        h->setBytesReaded(startOffset + bytesReaded);
        if (bytesReaded - lastPersist >= 256 * 1024) {
            persistResume(h);
            lastPersist = bytesReaded;
        }

        if (isStopTransfer) {
            m_lock.lock();
            m_cond.wait(&m_lock);
            m_lock.unlock();
        }

        if (isAbortTransfer.load()) {
            goto recv_file_error;
        }
    }

    file.close();
    // set modify time
    setLastModified(h);
    noteReceivedFile(h, m_recvFileMap->saveFilePath() + "/" + h->saveName());
    h->setState(RecvFile::RecvOk);
    clearResume(h);
    m_recvFileMap->addBytesReaded(bytesReaded);

    h->incrRegularFileCount();
    m_recvFileMap->incrRegularFileCount();
    m_recvFileMap->incrTotalRegularFileCount();

    return true;

recv_file_error:
    file.close();
    h->setState(RecvFile::RecvFail);
    persistResume(h);
    m_recvFileMap->addBytesReaded(bytesReaded);

    return false;
}

void RecvFileTransfer::setLastModified(QString path, QString secondString)
{
    if (!secondString.isEmpty()) {
        bool ok;
        quint64 secs = secondString.toULongLong(&ok, 16);
        if (ok) {
            struct utimbuf buf;
            // XXX NOTE: we set both access time and modify time to modify time
            buf.actime = secs;
            buf.modtime = secs;
            utime(path.toLocal8Bit(), &buf);
        }
    }
}

void RecvFileTransfer::setLastModified(RecvFileHandle h)
{
    QString path = m_recvFileMap->saveFilePath() + "/" + h->saveName();
    QString s = h->attrMap().value(IPMSG_FILE_MTIME);
    if (!s.isEmpty()) {
        bool ok;
        quint64 secs = s.toULongLong(&ok, 16);
        if (ok) {
            struct utimbuf buf;
            // XXX NOTE: we set both access time and modify time to modify time
            buf.actime = secs;
            buf.modtime = secs;
            utime(path.toLocal8Bit(), &buf);
        }
    }
}

bool RecvFileTransfer::recvFileDir(RecvFileHandle h)
{
    h->setStartTime();

    QByteArray sendBlock(constructRecvFileDatagram(h));
    m_tcpSocket.write(sendBlock);
    if (!waitForSocket(3000, false)) {
        return false;
    }

    QDir dir(m_recvFileMap->saveFilePath());
    QByteArray recvBlock;
    qint64 bytesToWrite = 0;
    bool isRecvContentData = false;
    QFile file;
    struct TransferFile transferFile;
    forever {
        if (isStopTransfer) {
            m_lock.lock();
            m_cond.wait(&m_lock);
            m_lock.unlock();
        }
        if (isAbortTransfer.load()) {
            h->setState(RecvFile::RecvFail);
            return false;
        }

        if (!waitForSocket(3000, true)) {
            return false;
        }

        recvBlock.append(m_tcpSocket.read(m_tcpSocket.bytesAvailable()));

        if (isRecvContentData) {
            if (bytesToWrite > recvBlock.size()) {
                saveData(recvBlock, file);
                h->addBytesReaded(recvBlock.size());
                m_recvFileMap->addBytesReaded(recvBlock.size());
                bytesToWrite -= recvBlock.size();
                recvBlock.clear();
                continue;
            } else {
                isRecvContentData = false;
                saveData(recvBlock.left(bytesToWrite), file);
                h->addBytesReaded(bytesToWrite);
                m_recvFileMap->addBytesReaded(bytesToWrite);
                recvBlock.remove(0, bytesToWrite);
                bytesToWrite = 0;
                m_recvFileMap->incrTotalRegularFileCount();
                h->incrRegularFileCount();
                file.close();   // successfully get file
                setLastModified(dir.absolutePath() + "/" + transferFile.name,
                                transferFile.extendAttr.value(IPMSG_FILE_MTIME));
            }
        }

        while (!isRecvContentData && canParseHeader(recvBlock)) {
            if (!parseHeader(recvBlock, transferFile)) {
                return false;
            }

            if (transferFile.type == IPMSG_FILE_REGULAR) {
                bytesToWrite = transferFile.size;
                m_recvFileMap->setCurrentChildName(transferFile.name);
                file.setFileName(dir.absolutePath() + "/" + transferFile.name);
                if (!file.open(QIODevice::WriteOnly)) {
                    m_errorString = "RecvFileTransfer::recvFileDir:"
                        + file.errorString();
                    return false;
                }

                if (bytesToWrite <= recvBlock.size()) {
                    saveData(recvBlock.left(bytesToWrite), file);
                    h->addBytesReaded(bytesToWrite);
                    m_recvFileMap->addBytesReaded(bytesToWrite);
                    recvBlock.remove(0, bytesToWrite);
                    m_recvFileMap->incrTotalRegularFileCount();
                    h->incrRegularFileCount();
                    file.close();   // successfully get file
                    setLastModified(dir.absolutePath() + "/"
                                    + transferFile.name,
                                    transferFile.extendAttr.value(IPMSG_FILE_MTIME));
                } else {
                    isRecvContentData = true;
                    saveData(recvBlock, file);
                    h->addBytesReaded(recvBlock.size());
                    m_recvFileMap->addBytesReaded(recvBlock.size());
                    bytesToWrite -= recvBlock.size();
                    recvBlock.clear();
                }
            } else if (transferFile.type == IPMSG_FILE_DIR) {
                if (m_recvFileMap->state() == RecvFileMap::Retry
                    && dir.exists(transferFile.name)) {
                    // nothing to do
                } else if (!dir.mkdir(transferFile.name)) {
                    m_errorString
                        = "RecvFileTransfer::recvFileDir: mkdir error";
                    return false;
                }
                dir.cd(transferFile.name);
                // set folder modify name
                setLastModified(dir.absolutePath(),
                                transferFile.extendAttr.value(IPMSG_FILE_MTIME));
            } else if (transferFile.type == IPMSG_FILE_RETPARENT) {
                if (!dir.cdUp()) {
                    m_errorString
                        = "RecvFileTransfer::recvFileDir: cdUp error";
                    return false;
                }

                QString path;
                if (!dir.isRoot()) {
                    path = dir.absolutePath() + "/";
                } else {
                    path = dir.absolutePath();
                }
                if (QDir::cleanPath(path) == QDir::cleanPath(m_recvFileMap->saveFilePath())) {
                    h->setState(RecvFile::RecvOk);
                    m_recvFileMap->incrDirCount();
                    return true;
                }
            } else {
                m_errorString
                    = "RecvFileTransfer::recvFileDir: unsupported file type";
                return false;
            }
        }
    }
}

bool RecvFileTransfer::parseHeader(QByteArray &recvBlock,
                                   struct TransferFile &transferFile)
{
    bool ok;
    int headerSize
        = recvBlock.left(TRANSFER_FILE_HEADER_SIZE_LENGTH).toInt(&ok, 16);
    if (!ok) {
        m_errorString
            = "RecvFileTransfer::parseHeader: get headerSize error";
        return false;
    }

    QByteArray header = recvBlock.left(headerSize);

    QList<QByteArray> list = header.split(':');

#define TRANSFERFILE_NAME_POS           1
#define TRANSFERFILE_SIZE_POS           2
#define TRANSFERFILE_TYPE_POS           3
#define TRANSFERFILE_ATTR_BEGIN_POS     4

    // XXX NOTE: canParseHeader() make sure we have enough items in list,
    // so we do not need to check size of list before call list.at()
    transferFile.name = Global::transferCodec->decode(
            list.at(TRANSFERFILE_NAME_POS));
    transferFile.size = list.at(TRANSFERFILE_SIZE_POS).toLongLong(&ok, 16);
    if (!ok) {
        m_errorString = "RecvFileTransfer::parseHeader: get file size error";
        return false;
    }
    transferFile.type = list.at(TRANSFERFILE_TYPE_POS).toInt(&ok, 16);
    if (!ok) {
        m_errorString = "RecvFileTransfer::parseHeader: get file type error";
        return false;
    }

    // Extended file attribution like mtime, atime...
    for (int i = TRANSFERFILE_ATTR_BEGIN_POS; i < list.size(); ++i) {
        QString s = list.at(i);
        QStringList l = s.split(QChar('='));
        if (l.size() == 2) {
            int i = l.at(0).toInt(&ok, 16);
            if (ok) {
                transferFile.extendAttr.insert(i, l.at(1));
            }
        }
    }

    recvBlock.remove(0, headerSize);

    return true;
}

bool RecvFileTransfer::canParseHeader(QByteArray &recvBlock)
{
    if (recvBlock.size() < TRANSFER_FILE_HEADER_SIZE_LENGTH)
        return false;

    bool ok;
    int headerSize
        = recvBlock.left(TRANSFER_FILE_HEADER_SIZE_LENGTH).toInt(&ok, 16);

    if (recvBlock.size() >= headerSize) {
        return true;
    }

    return false;
}

bool RecvFileTransfer::saveData(QByteArray recvBlock, QFile &file)
{
    qint64 bytesToWrite = recvBlock.size();

    forever {
        qint64 bytesWrited = file.write(recvBlock);

        if (bytesWrited == -1) {
            m_errorString = file.errorString();
            return false;
        }
        bytesToWrite -= bytesWrited;
        if (bytesToWrite == 0) {
            break;
        }
        recvBlock.remove(0, bytesWrited);
    }

    return true;
}

QByteArray RecvFileTransfer::constructRecvFileDatagram(RecvFileHandle h)
{
    QString s = QString("%1:%2:%3:%4").arg(IPMSG_VERSION)
        .arg(Helper::packetNoString())
        .arg(Global::userManager->ourself().loginName())
        .arg(Global::userManager->ourself().host());

    int flags = 0;
    if (h->type() == IPMSG_FILE_REGULAR) {
        flags = IPMSG_GETFILEDATA;
    } else if (h->type() == IPMSG_FILE_DIR) {
        flags = IPMSG_GETDIRFILES;
    }

    s.append(":");
    // FeiQ 2013 TCP GETFILEDATA/GETDIRFILES uses a hex command field.
    // ServeSocket also accepts classic decimal, so qipmsg↔qipmsg still works.
    s.append(QString("%1:%2:%3:").arg(flags, 0, 16)
            .arg(h->packetNo(), 0, 16)
            .arg(h->fileId(), 0, 16));

    if (h->type() == IPMSG_FILE_REGULAR) {
        if (m_recvFileMap->state() == RecvFileMap::Retry) {
            s.append(QString("%1:").arg(h->offset(), 0, 16));
        } else {
            s.append(QString("%1:").arg(0/* offset */, 0, 16));
        }
    }

    return Global::transferCodec->encode(s);
}

QString RecvFileTransfer::resumeKey(RecvFileHandle h) const
{
    return ResumeStore::makeKey(h->ip(), h->packetNoString(), h->fileId(),
                                h->name(), h->size());
}

void RecvFileTransfer::applyResumeIfNeeded(RecvFileHandle h)
{
    if (!Global::preferences->isResumeTransFile || !Global::resumeStore) {
        return;
    }
    if (h->type() != IPMSG_FILE_REGULAR) {
        return;
    }

    const QString key = resumeKey(h);
    const ResumeRecord record = Global::resumeStore->lookup(key);
    const QString dir = m_recvFileMap->saveFilePath();
    QString localName = h->saveName();
    QFileInfo info(dir + "/" + localName);
    qint64 existing = info.exists() ? info.size() : 0;

    if (record.offset > 0 && existing > 0 && existing < h->size()) {
        h->setOffset(existing);
        h->setBytesReaded(existing);
        m_recvFileMap->setState(RecvFileMap::Retry);
        return;
    }

    if (existing > 0 && existing < h->size()
            && Global::preferences->isResumeTransFile) {
        h->setOffset(existing);
        h->setBytesReaded(existing);
        m_recvFileMap->setState(RecvFileMap::Retry);
        return;
    }

    if (existing > 0 && Global::preferences->isAutoRecvRename) {
        localName = Helper::uniqueFileName(dir, h->name());
        h->setSaveName(localName);
        h->setOffset(0);
    }
}

void RecvFileTransfer::persistResume(RecvFileHandle h)
{
    if (!Global::preferences->isResumeTransFile || !Global::resumeStore) {
        return;
    }
    if (h->type() != IPMSG_FILE_REGULAR || h->offset() <= 0) {
        return;
    }
    ResumeRecord record;
    record.key = resumeKey(h);
    record.ip = h->ip();
    record.packetNo = h->packetNoString();
    record.fileId = h->fileId();
    record.name = h->name();
    record.size = h->size();
    record.offset = h->offset();
    record.saveDir = m_recvFileMap->saveFilePath();
    Global::resumeStore->save(record);
}

bool RecvFileTransfer::tryInstant(RecvFileHandle h)
{
    if (h->type() != IPMSG_FILE_REGULAR) {
        return false;
    }
    const QString sha = h->attrMap().value(IPMSG_FILE_QIPHASH).trimmed().toLower();
    if (sha.size() != 64 || h->size() < 0) {
        return false;
    }
    const QString dir = m_recvFileMap->saveFilePath();
    QString src = FileHashIndex::findExisting(sha, h->size());
    if (src.isEmpty()) {
        src = FileHashIndex::findInDirectory(dir, h->name(), sha, h->size());
    }
    if (src.isEmpty()) {
        return false;
    }

    QDir().mkpath(dir);
    QString destName = h->name();
    QString destPath = QDir(dir).filePath(destName);
    const QString srcCanon = QFileInfo(src).canonicalFilePath();
    if (QFileInfo(destPath).exists()) {
        const QString destCanon = QFileInfo(destPath).canonicalFilePath();
        if (srcCanon.isEmpty() || destCanon != srcCanon) {
            if (Global::preferences && Global::preferences->isAutoRecvRename) {
                destName = Helper::uniqueFileName(dir, h->name());
                destPath = QDir(dir).filePath(destName);
            } else if (!QFile::remove(destPath)) {
                return false;
            }
        }
    }
    if (srcCanon.isEmpty()
            || QFileInfo(destPath).canonicalFilePath() != srcCanon) {
        if (QFileInfo(destPath).exists() && !QFile::remove(destPath)) {
            return false;
        }
        if (!QFile::copy(src, destPath)) {
            return false;
        }
    }
    h->setSaveName(destName);
    h->setOffset(h->size());
    h->setBytesReaded(h->size());
    h->setState(RecvFile::RecvOk);
    h->incrRegularFileCount();
    m_recvFileMap->incrRegularFileCount();
    m_recvFileMap->incrTotalRegularFileCount();
    m_recvFileMap->addBytesReaded(h->size());
    noteReceivedFile(h, destPath);

    quint16 port = h->peerPort();
    if (port == 0 && Global::userManager) {
        port = Global::userManager->peerPort(h->ip());
    }
    if (port == 0) {
        port = IPMSG_DEFAULT_PORT;
    }
    if (Global::msgThread && Global::userManager) {
        const QString body = QString("INSTANT:%1:%2:%3")
                .arg(h->packetNo(), 0, 16)
                .arg(h->fileId(), 0, 16)
                .arg(sha);
        SendMsg sendMsg(h->ipAddress(), port, body, QString(), IPMSG_QIPMSG_EXT);
        Global::msgThread->addSendMsg(Msg(sendMsg));
    }
    if (Global::windowManager) {
        QMetaObject::invokeMethod(Global::windowManager, "noteInstantSkip",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, h->ip()),
                                  Q_ARG(QString, h->name()),
                                  Q_ARG(bool, true));
    }
    return true;
}

void RecvFileTransfer::noteReceivedFile(RecvFileHandle h, const QString &path)
{
    qint64 size = -1;
    const QString sha = FileHashIndex::hashFile(path, &size);
    if (sha.size() != 64) {
        return;
    }
    const QString advertised = h->attrMap().value(IPMSG_FILE_QIPHASH)
            .trimmed().toLower();
    if (!advertised.isEmpty() && advertised != sha) {
        qWarning("RecvFileTransfer: content hash does not match the offer");
    }
    FileHashIndex::note(path, sha, size, QStringLiteral("recv"), h->ip());
}

void RecvFileTransfer::clearResume(RecvFileHandle h)
{
    if (Global::resumeStore) {
        Global::resumeStore->remove(resumeKey(h));
    }
}

void RecvFileTransfer::stopTransfer()
{
    qDebug() << "RecvFileTransfer::stopTransfer";

    isStopTransfer = true;
}

bool RecvFileTransfer::waitForSocket(int totalMs, bool readyRead)
{
    int waited = 0;
    while (waited < totalMs) {
        if (isAbortTransfer.load()) {
            m_errorString = tr("Transfer cancelled");
            return false;
        }
        const int step = qMin(200, totalMs - waited);
        const bool ready = readyRead
                ? m_tcpSocket.waitForReadyRead(step)
                : m_tcpSocket.waitForBytesWritten(step);
        if (isAbortTransfer.load()) {
            m_errorString = tr("Transfer cancelled");
            return false;
        }
        if (ready) {
            return true;
        }
        if (m_tcpSocket.state() == QAbstractSocket::UnconnectedState
                || (m_tcpSocket.error() != QAbstractSocket::SocketTimeoutError
                    && m_tcpSocket.error() != QAbstractSocket::UnknownSocketError)) {
            m_errorString = m_tcpSocket.errorString();
            return false;
        }
        waited += step;
    }
    m_errorString = m_tcpSocket.errorString();
    return false;
}

void RecvFileTransfer::abortTransfer()
{
    qDebug() << "RecvFileTransfer::abortTransfer";

    isAbortTransfer.store(true);
    const qintptr fd = m_tcpSocket.socketDescriptor();
    if (fd >= 0) {
#ifdef _WIN32
        shutdown(static_cast<SOCKET>(fd), SD_BOTH);
#else
        ::shutdown(static_cast<int>(fd), SHUT_RDWR);
#endif
    }
}

void RecvFileTransfer::resumeTransfer()
{
    qDebug() << "RecvFileTransfer::resumeTransfer";

    if (isStopTransfer) {
        // XXX NOTE: we need first set 'isStopTransfer' to false to avoid
        // m_cond.wait() be called.
        m_lock.lock();
        isStopTransfer = false;
        m_cond.wakeOne();
        m_lock.unlock();
    }
}

