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

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#endif

#include "serve_socket.h"
#include "constants.h"
#include "transfer_codec.h"
#include "send_file_map.h"
#include "send_file_manager.h"
#include "global.h"
#include "helper.h"
#include "sendThrottle.h"
#include "fileHashIndex.h"
#include "preferences.h"
#include "shareFolder.h"
#include "discussion_store.h"

#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QMutexLocker>
#include <QTextCodec>

#include <QHostAddress>

#ifndef _WIN32
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#define REQUST_FILE_PACKET_ID_POSITION      5
#define REQUST_FILE_FILE_ID_POSITION        6
#define REQUST_FILE_OFFSET_POSITION         7
#define MAXBUFF                             8192

#define BLOCK_SIZE                          1024*16

struct RequsetFile
{
    bool isFileSended;
    int fileType;
    QString filePath;
    qint64 offset;
    int fileId;
};

ServeSocket::ServeSocket(qintptr socketDescriptor, QObject *parent)
    : QObject(parent), m_sockfd(socketDescriptor), m_lastProgressMs(0)
{
#if 0
    connect(&m_tcpSocket, SIGNAL(bytesWritten(qint64)),
            this, SLOT(updateBytesWrited(qint64)));
    connect(&m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(socketError(QAbstractSocket::SocketError)));
#endif
}
ServeSocket::~ServeSocket()
{
#ifdef _WIN32
  closesocket(static_cast<SOCKET>(m_sockfd));
#else
  close(m_sockfd);
#endif
}

bool ServeSocket::startSendFile()
{
    qDebug() << "ServeSocket::startSendFile";

    QByteArray recvBlock;

    int  readlen = 0;
    char buff[MAXBUFF];
    while ( 1 ) {
#ifdef _WIN32
      readlen = recv(static_cast<SOCKET>(m_sockfd), buff, MAXBUFF-1, 0);
      if (readlen == SOCKET_ERROR || readlen <= 0) {
        return false;
      }
#else
      readlen = read( m_sockfd, buff, MAXBUFF-1 );
      if ( errno == EINTR ) {
        continue;
      }
      if ( readlen <= 0 ) {
        return false;
      }
#endif
      buff[readlen] = '\0';
      recvBlock.append( buff );
      if (recvBlock.size() > 256 * 1024) {
        return false;
      }
      if ( canParsePacket(recvBlock) ) {
        break;
      }
    }

//    forever {
//        if (!m_tcpSocket.waitForReadyRead(3000)) {
//            m_errorString = m_tcpSocket.errorString();
//            return false;
//        }
//
//        recvBlock.append(m_tcpSocket.read(m_tcpSocket.bytesAvailable()));
//
//        if (canParsePacket(recvBlock)) {
//            break;
//        }
//    }

    return handleRequest(recvBlock);
}

bool ServeSocket::canParsePacket(const QByteArray &requestPacket) const
{
    QList<QByteArray> bl = requestPacket.split(':');
    if (bl.count() < 8) {
        return false;
    }

    bool ok = false;
    const quint32 command = Helper::parseIpmsgCommand(bl.at(MSG_FLAGS_POS), &ok);
    if (ok && GET_MODE(command) == IPMSG_QIPMSG_EXT) {
        return requestPacket.contains('\n');
    }
    if (ok && GET_MODE(command) == IPMSG_GETFILEDATA && bl.count() < 9) {
        return false;
    }

    return true;
}

bool ServeSocket::writeRaw(const QByteArray &block, bool pace)
{
    const char *buff = block.constData();
    qint64 nbytes = block.size();
    qint64 sent = 0;
    while (sent < nbytes) {
        qint64 wrote = 0;
#ifdef _WIN32
        const int chunk = static_cast<int>(qMin(nbytes - sent, (qint64)1024 * 1024));
        int n = send(static_cast<SOCKET>(m_sockfd), buff + sent, chunk, 0);
        if (n == SOCKET_ERROR || n <= 0) {
            return false;
        }
        wrote = n;
#else
        ssize_t n = send(m_sockfd, buff + sent,
                         static_cast<size_t>(nbytes - sent), 0);
        if (n > 0) {
            wrote = n;
        } else if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            return false;
        } else {
            return false;
        }
#endif
        sent += wrote;
        if (pace && !SendThrottle::pace(wrote, []() { return false; })) {
            return false;
        }
    }
    return true;
}

namespace {

QString peerIpv4(qintptr descriptor)
{
    sockaddr_storage storage;
#ifdef _WIN32
    int length = static_cast<int>(sizeof(storage));
    if (::getpeername(static_cast<SOCKET>(descriptor),
                      reinterpret_cast<sockaddr *>(&storage), &length) != 0) {
        return QString();
    }
#else
    socklen_t length = sizeof(storage);
    if (::getpeername(static_cast<int>(descriptor),
                      reinterpret_cast<sockaddr *>(&storage), &length) != 0) {
        return QString();
    }
#endif
    if (storage.ss_family != AF_INET) {
        return QString();
    }
    const sockaddr_in *ipv4 = reinterpret_cast<const sockaddr_in *>(&storage);
    return QHostAddress(ntohl(ipv4->sin_addr.s_addr)).toString();
}

bool sameIpv4(const QString &left, const QString &right)
{
    const QHostAddress a(left);
    const QHostAddress b(right);
    if (a.isNull() || b.isNull()) {
        return false;
    }
    if (a.protocol() == QAbstractSocket::IPv4Protocol
            && b.protocol() == QAbstractSocket::IPv4Protocol) {
        return a.toIPv4Address() == b.toIPv4Address();
    }
    return a == b;
}

} // namespace

bool ServeSocket::handleShare(const QByteArray &requestPacket)
{
    QString groupId;
    QString verb;
    QString relative;
    QString password;
    qint64 offset = 0;
    const bool parsed = ShareFolder::parseRequest(
                requestPacket, &verb, &relative, &password, &offset, &groupId);
    ShareAnswer answer;
    answer.offset = 0;
    answer.fileSize = 0;
    answer.sendFile = false;
    if (!parsed) {
        answer.payload = "QIP1\nERR badpath\n";
    } else if (!groupId.isEmpty()) {
        QString root;
        bool member = false;
        bool known = false;
        if (Global::discussionStore) {
            const DiscussionGroup group = Global::discussionStore->groupById(groupId);
            known = !group.id.isEmpty() && !group.sharePath.trimmed().isEmpty();
            root = group.sharePath;
            const QString peer = peerIpv4(m_sockfd);
            for (int i = 0; i < group.members.size(); ++i) {
                if (sameIpv4(group.members.at(i).ip, peer)) {
                    member = true;
                    break;
                }
            }
        }
        if (!known) {
            answer.payload = "QIP1\nERR off\n";
        } else {
            answer = ShareFolder::answerGroup(requestPacket, root, member);
        }
    } else {
        const bool enabled = Global::preferences && Global::preferences->shareEnabled;
        const QString root = Global::preferences
                ? Global::preferences->sharePath : QString();
        const QString expected = Global::preferences
                ? Global::preferences->sharePassword : QString();
        answer = ShareFolder::answer(requestPacket, enabled, root, expected);
    }
    if (!writeRaw(answer.payload, false)) {
        return false;
    }
    if (!answer.sendFile) {
        return true;
    }
    QFile file(answer.filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    if (answer.offset > 0 && !file.seek(answer.offset)) {
        return false;
    }
    while (!file.atEnd()) {
        QByteArray block = file.read(BLOCK_SIZE);
        if (block.isEmpty()) {
            return false;
        }
        if (!writeRaw(block, true)) {
            return false;
        }
    }
    return true;
}

bool ServeSocket::handleRequest(const QByteArray &requestPacket)
{
    qDebug() << "ServeSocket::handleRequest";

    {
        const QList<QByteArray> head = requestPacket.split(':');
        if (head.size() > MSG_FLAGS_POS) {
            bool commandOk = false;
            const quint32 command = Helper::parseIpmsgCommand(
                        head.at(MSG_FLAGS_POS), &commandOk);
            if (commandOk && GET_MODE(command) == IPMSG_QIPMSG_EXT) {
                return handleShare(requestPacket);
            }
        }
    }

    struct RequsetFile requestFile;
    bool regular = false;
    QString donePath;
    qint64 doneSize = 0;
    QString doneHash;
    parseRequestPacket(requestPacket, requestFile);

    SendFileMap *map = 0;
    Global::sendFileManager->m_lock.lock();
    map = Global::sendFileManager->transferFileMap.value(m_packetNoString);
    if (map) {
        if (map->cancelRequested()) {
            Global::sendFileManager->removeTransfer(m_packetNoString);
            Global::sendFileManager->m_lock.unlock();
            return false;
        }
        map->attachSocket(m_sockfd);
        map->setState(SendFileMap::Transfer);
        Global::sendFileManager
            ->transferFileModel.updateTransfer(m_packetNoString);
    } else {
        Global::sendFileManager->m_lock.unlock();
        return false;
    }
    map->sem.release();
    Global::sendFileManager->m_lock.unlock();

    if (!requestFile.isFileSended) {
        m_errorString = "ServeSocket::handleRequest: Request file not sended";
        goto handle_request_fail;
    }

    if (requestFile.fileType == IPMSG_FILE_REGULAR) {
        if (!tcpSendFile(requestFile.filePath, requestFile.offset)) {
            if (Global::sendFileManager->finishIfCancelled(m_packetNoString)) {
                return false;
            }
            goto handle_request_fail;
        }
    } else if (requestFile.fileType == IPMSG_FILE_DIR) {
        if (!tcpSendDir(requestFile.filePath)) {
            if (Global::sendFileManager->finishIfCancelled(m_packetNoString)) {
                return false;
            }
            goto handle_request_fail;
        }
    } else {
        // unsupported type
        goto handle_request_fail;
    }

    Global::sendFileManager->m_lock.lock();
    map = Global::sendFileManager->transferFileMap.value(m_packetNoString);
    if (!map) {
        Global::sendFileManager->m_lock.unlock();
        return false;
    }
    map->detachSocket();
    map->m_map[requestFile.fileId]->setState(SendFile::SendOk);
    regular = requestFile.fileType == IPMSG_FILE_REGULAR;
    donePath = requestFile.filePath;
    doneSize = regular ? map->m_map[requestFile.fileId]->size() : 0;
    doneHash = regular ? map->m_map[requestFile.fileId]->contentHash() : QString();
    map->incrTransferCount();
    map->setState(SendFileMap::NotTransfer);
    Global::sendFileManager
        ->transferFileModel.updateTransfer(m_packetNoString);
    // if transfer finished, delete transfer
    if (map->isFinished()) {
        Global::sendFileManager->removeTransfer(m_packetNoString);
    }
    Global::sendFileManager->m_lock.unlock();

    if (regular && !donePath.isEmpty()) {
        if (doneHash.size() != 64) {
            doneHash = FileHashIndex::hashFile(donePath, 0);
        }
        if (doneHash.size() == 64) {
            FileHashIndex::note(donePath, doneHash, doneSize,
                                QStringLiteral("send"), QString());
        }
    }

    return true;

handle_request_fail:
    qDebug() << "ServeSocket::handleRequest: send file error";

    // when this error happed, the file may have finished transfer by a
    // client's retry transfer, then file has been deleted from sendFileMap's
    // m_map.
    // we user handle class, so we can not test if recvFileHandle is bounded
    // how to test if a handle class is bounded????????????????
    Global::sendFileManager->m_lock.lock();
    map = Global::sendFileManager->transferFileMap.value(m_packetNoString);
    if (map) {
        qDebug() << "sem.available:" << map->sem.available();
        if (map->sem.available() == 1) {
            map->setState(SendFileMap::NotTransfer);
            Global::sendFileManager
                ->transferFileModel.updateTransfer(m_packetNoString);
            map->m_map[requestFile.fileId]->setState(SendFile::SendFail);
        }
        map->sem.acquire();
    }
    Global::sendFileManager->m_lock.unlock();

    return false;
}

void ServeSocket::parseRequestPacket(const QByteArray &requestPacket,
        struct RequsetFile &requestFile)
{
    QList<QByteArray> list = requestPacket.split(':');

    // XXX NOTE: canParsePacket() make sure we have enough items in list,
    // so we do not need to check size of list before call list.at()
    bool ok = false;
    bool fileIdOk = false;
    int fileId = list.at(REQUST_FILE_FILE_ID_POSITION).toInt(&fileIdOk, 16);
    if (!fileIdOk) {
        fileId = list.at(REQUST_FILE_FILE_ID_POSITION).toInt(&fileIdOk, 10);
    }
    const quint32 command
        = Helper::parseIpmsgCommand(list.at(MSG_FLAGS_POS), &ok);

    QMutexLocker locker(&Global::sendFileManager->m_lock);

    SendFileMap *sendFileMap = 0;
    const QStringList packetKeys = Helper::packetNoLookupKeys(
                list.at(REQUST_FILE_PACKET_ID_POSITION));
    foreach (const QString &key, packetKeys) {
        sendFileMap = Global::sendFileManager->transferFileMap.value(key);
        if (sendFileMap) {
            m_packetNoString = key;
            break;
        }
    }

    requestFile.isFileSended = false;
    if (sendFileMap && fileIdOk && sendFileMap->canSendFile(fileId)) {
        requestFile.isFileSended = true;
        requestFile.fileType = sendFileMap->m_map[fileId]->type();
        requestFile.filePath = sendFileMap->m_map[fileId]->absoluteFilePath();
        requestFile.fileId = fileId;
        if (GET_MODE(command) == IPMSG_GETFILEDATA) {
            requestFile.offset
                = list.at(REQUST_FILE_OFFSET_POSITION).toLongLong(&ok, 16);
        } else {
            requestFile.offset = 0;
        }
    }
}

bool ServeSocket::tcpSendFile(QString filePath, qint64 offset)
{
    if (transferCancelled()) {
        return false;
    }

    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    if (offset > 0) {
        noteProgress(0, offset);
    }
    file.seek(offset);
    while (!file.atEnd()) {
        if (transferCancelled()) {
            return false;
        }
        QByteArray block = file.read(BLOCK_SIZE);
        if (!tcpWriteBlock(block)) {
            return false;
        }
    }

    return true;
}

void ServeSocket::noteProgress(qint64 written, qint64 skipped)
{
    QMutexLocker locker(&Global::sendFileManager->m_lock);
    SendFileMap *map = Global::sendFileManager->transferFileMap.value(m_packetNoString);
    if (!map) {
        return;
    }
    if (skipped > 0) {
        map->addBytesSkipped(skipped);
    }
    if (written > 0) {
        map->addBytesSent(written);
    }
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (m_lastProgressMs != 0 && now - m_lastProgressMs < 250) {
        return;
    }
    m_lastProgressMs = now;
    Global::sendFileManager->transferFileModel.updateTransfer(m_packetNoString);
}

bool ServeSocket::transferCancelled() const
{
    QMutexLocker locker(&Global::sendFileManager->m_lock);
    SendFileMap *map = Global::sendFileManager->transferFileMap.value(m_packetNoString);
    return !map || map->cancelRequested();
}

bool ServeSocket::tcpWriteBlock(QByteArray &block)
{
  if (transferCancelled()) {
    return false;
  }
  size_t  nbytes = block.size();
  char*   buff   = block.data();

  size_t  sent = 0;
  while ( sent < nbytes ) {
    if (transferCancelled()) {
      return false;
    }
    qint64 wrote = 0;
#ifdef _WIN32
    int n = send(static_cast<SOCKET>(m_sockfd), buff + sent,
                 static_cast<int>(nbytes - sent), 0);
    if (n == SOCKET_ERROR || n <= 0) {
      return false;
    }
    wrote = n;
#else
    ssize_t n = send(m_sockfd, buff+sent, nbytes-sent, 0);
    if ( n > 0 ) {
      wrote = n;
    } else if ( n < 0 ) {
      if ( errno == EINTR ) {
        continue;
      }
      return false;
    } else {
      return false;
    }
#endif
    sent += static_cast<size_t>(wrote);
    noteProgress(wrote, 0);
    if (!SendThrottle::pace(wrote, [this]() { return transferCancelled(); })) {
      return false;
    }
  }
  return !transferCancelled();
//    qint32 bytesToWrite = block.size();
//    forever {
//        qint32 n = m_tcpSocket.write(block);
//        if (n == -1) {
//            return false;
//        }
//        if (!m_tcpSocket.waitForBytesWritten(-1)) {
//            m_errorString = m_tcpSocket.errorString();
//            return false;
//        }
//        bytesToWrite -= n;
//        block.remove(0, n);
//        if (bytesToWrite == 0) {
//            break;
//        }
//    }
//    return true;
}

bool ServeSocket::tcpSendDir(QString filePath)
{
    QByteArray dirBlock = constructDirSendBlock(filePath, NormalBlockMode);
    if (!tcpWriteBlock(dirBlock)) {
        return false;
    }

    QDir dir(filePath);
    QFileInfoList fileInfoList = dir.entryInfoList();

    foreach (QFileInfo fi, fileInfoList) {
        // Skip '.' and '..' directory
        if (fi.fileName() == "." || fi.fileName() == "..") {
            continue;
        }

        if (fi.isFile()) {
            QByteArray fileBlock
                = constructFileSendBlock(fi.absoluteFilePath());
            if (!tcpWriteBlock(fileBlock)) {
                return false;
            }
            if (!tcpSendFile(fi.absoluteFilePath(), 0)) {
                return false;
            }
        }
        if (fi.isDir()) {
            if (!tcpSendDir(fi.absoluteFilePath())) {
                return false;
            }
        }
    }

    QByteArray retParentBlock
        = constructDirSendBlock(filePath, RetParentBlockMode);
    if (!tcpWriteBlock(retParentBlock)) {
        return false;
    }

    return true;
}

QByteArray ServeSocket::constructDirSendBlock(QString filePath,
        DirBlockModes mode)
{
    QFileInfo fi(filePath);

    QString fileName;
    if (mode == NormalBlockMode) {
        fileName = fi.fileName();
        // If send directory is root directory, we change its name to
        // "(root-directory)"
        if (fileName == "/") {
            fileName = "(root-directory)";
        }
    } else if (mode == RetParentBlockMode) {
        fileName = ".";
    }

    QString str(":");
    str.append(fileName);
    str.append(":");
    str.append(QString("%1").arg(0, TRANSFER_FILE_FILE_SIZE_LENGTH, 16, QChar('0')));
    str.append(":");
    if (mode == RetParentBlockMode) {
        str.append(QString("%1").arg(IPMSG_FILE_RETPARENT, 0, 16));
    } else if (mode == NormalBlockMode) {
        str.append(QString("%1").arg(IPMSG_FILE_DIR, 0, 16));
    }
    str.append(":");
    str.append(QString("%1=%2").arg(IPMSG_FILE_MTIME, 0, 16)
            .arg(Helper::dateTimeToSecsSinceEpoch(fi.lastModified()), 0, 16));
    str.append(":");
    str.append(QString("%1=%2").arg(IPMSG_FILE_CREATETIME, 0, 16)
            .arg(Helper::dateTimeToSecsSinceEpoch(
                    fi.birthTime().isValid() ? fi.birthTime() : fi.lastModified()), 0, 16));
    str.append(":");

    QByteArray block = Global::transferCodec->encode(str);
    QString s = QString("%1").arg(block.size() + TRANSFER_FILE_HEADER_SIZE_LENGTH,
            TRANSFER_FILE_HEADER_SIZE_LENGTH, 16, QChar('0'));

    QByteArray ba = Global::transferCodec->encode(s);
    return ba + block;
}

QByteArray ServeSocket::constructFileSendBlock(QString filePath) const
{
    QFileInfo fi(filePath);

    QString fileName;
    fileName = fi.fileName();

    QString str(":");
    str.append(fileName);
    str.append(":");
    str.append(QString("%1").arg(fi.size(), TRANSFER_FILE_FILE_SIZE_LENGTH,
                16, QChar('0')));
    str.append(":");
    str.append(QString("%1").arg(IPMSG_FILE_REGULAR, 0, 16));
    str.append(":");
    str.append(QString("%1=%2").arg(IPMSG_FILE_MTIME, 0, 16)
            .arg(Helper::dateTimeToSecsSinceEpoch(fi.lastModified()), 0, 16));
    str.append(":");
    str.append(QString("%1=%2").arg(IPMSG_FILE_CREATETIME, 0, 16)
            .arg(Helper::dateTimeToSecsSinceEpoch(
                    fi.birthTime().isValid() ? fi.birthTime() : fi.lastModified()), 0, 16));
    str.append(":");

    QByteArray block = Global::transferCodec->encode(str);
    QString s = QString("%1").arg(block.size() + TRANSFER_FILE_HEADER_SIZE_LENGTH,
            TRANSFER_FILE_HEADER_SIZE_LENGTH, 16, QChar('0'));

    QByteArray ba = Global::transferCodec->encode(s);
    return ba + block;
}

