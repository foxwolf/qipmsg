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

#include "msg_server.h"
#include "protocolutil.h"
#include "send_msg.h"
#include "global.h"
#include "constants.h"
#include "helper.h"
#include "transfer_codec.h"
#include "user_manager.h"
#include "msg_thread.h"
#include "preferences.h"
#include "send_file_manager.h"
#include "groupPolicy.h"

#include <QMutexLocker>
#include <QTextCodec>
#include <QNetworkInterface>
#include <QAbstractSocket>
#include <QSet>
#include <QtDebug>

#ifndef _WIN32
#include <sys/socket.h>
#endif

namespace {

bool isUsableIpv4Interface(const QNetworkInterface &networkInterface)
{
    const QNetworkInterface::InterfaceFlags flags = networkInterface.flags();
    if (!(flags & QNetworkInterface::IsUp)) {
        return false;
    }
    if (flags & QNetworkInterface::IsLoopBack) {
        return false;
    }
    return true;
}

bool isUsableIpv4Broadcast(const QNetworkAddressEntry &entry)
{
    const QHostAddress ip = entry.ip();
    if (ip.protocol() != QAbstractSocket::IPv4Protocol
        || ip.isLoopback()
        || ip.isNull()) {
        return false;
    }

    const QHostAddress broadcastAddress = entry.broadcast();
    if (broadcastAddress.isNull()
        || broadcastAddress == QHostAddress::AnyIPv4
        || broadcastAddress == QHostAddress::LocalHost) {
        return false;
    }

    return true;
}

void enableUdpBroadcast(QUdpSocket &udpSocket)
{
    const qintptr socketDescriptor = udpSocket.socketDescriptor();
    if (socketDescriptor == -1) {
        return;
    }

    int broadcastEnabled = 1;
#ifdef _WIN32
    if (::setsockopt(static_cast<SOCKET>(socketDescriptor), SOL_SOCKET,
                     SO_BROADCAST,
                     reinterpret_cast<const char *>(&broadcastEnabled),
                     sizeof(broadcastEnabled)) == SOCKET_ERROR) {
        qWarning() << "MsgServer: SO_BROADCAST failed:" << WSAGetLastError();
    }
#else
    if (::setsockopt(static_cast<int>(socketDescriptor), SOL_SOCKET,
                     SO_BROADCAST, &broadcastEnabled,
                     sizeof(broadcastEnabled)) != 0) {
        qWarning() << "MsgServer: SO_BROADCAST failed";
    }
#endif
}

} // namespace

MsgServer::MsgServer(QObject *parent)
    : QObject(parent)
{
    updateAddresses();

    connect(&m_udpSocket, SIGNAL(readyRead()),
            this, SLOT(readPacket()));
    connect(this, SIGNAL(releaseFile(QString)),
            Global::sendFileManager, SLOT(removeTransferLocked(QString)));
    connect(&m_udpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(socketEerror(QAbstractSocket::SocketError)));
}

void MsgServer::start()
{
    // IPMSG is IPv4-centric. QHostAddress::Any is IPv6 :: on Windows, and
    // writing IPv4 broadcasts from that socket fails with NetworkError.
    const QAbstractSocket::BindMode bindMode =
        QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint;
    const QHostAddress listenAddress = Global::preferences->listenAddress();
    const quint16 listenPort = Global::preferences->listenPort();
    if (!m_udpSocket.bind(listenAddress, listenPort, bindMode)) {
        qWarning() << "MsgServer::start: bind"
                   << listenAddress.toString() << listenPort
                   << "failed:" << m_udpSocket.errorString()
                   << "(continuing without UDP listen)";
        return;
    }

    enableUdpBroadcast(m_udpSocket);

    qInfo() << "MsgServer::start: UDP listen"
            << m_udpSocket.localAddress().toString()
            << m_udpSocket.localPort();
}

void MsgServer::readPacket()
{
    qDebug() << "MsgServer::readPacket";

    while (m_udpSocket.hasPendingDatagrams()) {
        QHostAddress senderIp;
        quint16 senderPort;
        QByteArray datagram;
        datagram.resize(m_udpSocket.pendingDatagramSize());
        if (m_udpSocket.readDatagram(datagram.data(), datagram.size(),
                                         &senderIp, &senderPort) == -1) {
            continue;
        }

        if (!isSupportedCommand(datagram)) {
            continue;
        }

        QString packet = Global::transferCodec->decodeDatagram(datagram);

        RecvMsg recvMsg(packet, senderIp, senderPort);

        processRecvMsg(Msg(recvMsg));
    }
}

void MsgServer::processRecvMsg(Msg msg)
{
    qDebug() << "MsgServer::processRecvMsg";

    switch (GET_MODE(msg->flags())) {
    case IPMSG_BR_ENTRY:
        processEntryMsg(msg);
        break;

    case IPMSG_BR_EXIT:
        emit newExitMsg(msg);
        break;

    case IPMSG_ANSENTRY:
        emit newUserMsg(msg);
        break;

    case IPMSG_BR_ABSENCE:
        processAbsenceMsg(msg);
        break;

    case IPMSG_GETABSENCEINFO:
        processGetAbsenceInfo(msg);
        break;

    case IPMSG_SENDMSG:
        processRecvSendMsg(msg);
        break;

    case IPMSG_FEIQ_TYPING:
    case IPMSG_FEIQ_TYPINGSTOP: {
        bool active = false;
        if (ProtocolUtil::isFeiqTypingCommand(msg->flags(), &active)) {
            emit peerTyping(msg->ip(), active);
        }
        return;
    }

    case IPMSG_READMSG:
        processRecvReadMsg(msg);
        break;

    case IPMSG_ANSREADMSG:
    case IPMSG_RECVMSG:
    case IPMSG_DELMSG:
        // do nothing
        // XXX TODO: do something?????????
        break;

    case IPMSG_RELEASEFILES:
        processRecvReleaseFilesMsg(msg);
        break;

    default:
        break;
    }

    emit peerFlagsSeen(msg->ip(), msg->flags());
}

void MsgServer::processEntryMsg(Msg msg)
{
    emit newUserMsg(msg);

    // Stealth / FeiQ HideMode: still record the peer so we can see them,
    // but do not answer BR_ENTRY with ANSENTRY. Directed SENDMSG and
    // GETABSENCEINFO remain answered so peers who already know our IP
    // can still reach us.
    if (Global::preferences->isStealth) {
        return;
    }

    // Per-group hide: still list the peer locally, but do not confirm we
    // are online. A directed leave covers peers who cached an earlier broadcast.
    const QString groupName = msg->owner().group();
    if (Global::groupPolicy && Global::groupPolicy->isHidden(groupName)) {
        if (Global::userManager) {
            Global::userManager->sendDirectedUserPacket(
                        msg->ip(), msg->port(), IPMSG_BR_EXIT);
        }
        return;
    }

    const quint32 flags = Global::userManager->commandFlags(IPMSG_ANSENTRY);

    SendMsg sendMsg(msg->ipAddress(), msg->port(),
                    Global::userManager->entryMessage(),
                    ""/* extendedInfo */, flags);

    Global::msgThread->addSendMsg(Msg(sendMsg));
}

void MsgServer::processAbsenceMsg(Msg msg)
{
    emit newUserMsg(msg);
}

void MsgServer::processGetAbsenceInfo(Msg msg)
{
    // Global stealth still answers (existing HideMode). Per-group hide does
    // not: an absence reply would tell that department we are online.
    QString groupName = msg->owner().group();
    if (groupName.isEmpty() && Global::userManager
            && Global::userManager->contains(msg->ip())) {
        groupName = Global::userManager->group(
                    Global::userManager->ipToRow(msg->ip()));
    }
    if (Global::groupPolicy && Global::groupPolicy->isHidden(groupName)) {
        return;
    }

    QString info;
    if (Global::preferences->isAway || Global::preferences->isAutoReply) {
        info = Global::preferences->autoReplyText.trimmed();
        if (info.isEmpty()) {
            info = tr("I am away from my computer.");
        }
    }

    SendMsg sendMsg(msg->ipAddress(), msg->port(),
                    info, ""/* extendedInfo */, IPMSG_SENDABSENCEINFO);
    Global::msgThread->addSendMsg(Msg(sendMsg));
}

void MsgServer::processRecvReleaseFilesMsg(Msg msg)
{
    emit releaseFile(msg->additionalInfo());
}

void MsgServer::processRecvReadMsg(Msg msg)
{
    if (Global::preferences->isReadCheck) {
        emit newMsg(msg);
    }

    SendMsg sendMsg(msg->ipAddress(), msg->port(), msg->packetNoString(),
                    ""/* extendedInfo */, IPMSG_ANSREADMSG);

    Global::msgThread->addSendMsg(Msg(sendMsg));
}

void MsgServer::processRecvSendMsg(Msg msg)
{
    if (GET_OPT(msg->flags()) & IPMSG_SENDCHECKOPT) {
        SendMsg sendMsg(msg->ipAddress(), msg->port(), msg->packetNoString(),
                ""/* extendedInfo */, IPMSG_RECVMSG);

        Global::msgThread->addSendMsg(Msg(sendMsg));
    }

    // If sender is not in our user list, add it.
    if (!Global::userManager->contains(msg->ip())) {
        emit newUserMsg(msg);
    }

    emit newMsg(msg);
}

void MsgServer::processSendMsg()
{
    // We are a friend of MsgThread class
    QMutexLocker locker(&Global::msgThread->m_lock);

    foreach (Msg msg, Global::msgThread->m_sendMsgMap) {
        handleMsg(msg);
    }
}

void MsgServer::handleMsg(Msg &msg)
{
    // Delete msg
    if (msg->state() == MsgBase::SendAckOk) {
        Global::msgThread->removeSendMsgNotLock(msg->packetNoString());
    }

    // Delete msg
    if (msg->sendTimes() >= MAX_RE_SEND_TIMES) {
        Global::msgThread->removeSendMsgNotLock(msg->packetNoString());
    }

    // Send msg
    if (msg->state() == MsgBase::NotSend
        || isResendNeeded(msg)) {

        msg->setState(MsgBase::Sending);
        msg->incrementSendTimes();

        switch (GET_MODE(msg->flags())) {
        case IPMSG_BR_ENTRY:
        case IPMSG_BR_EXIT:
        case IPMSG_BR_ABSENCE:
            // Null address is the subnet broadcast. A concrete address is a
            // directed presence packet (per-group hide / unhide).
            if (msg->ipAddress().isNull()) {
                broadcastUserMsg(msg);
            } else {
                broadcastMsg(msg);
            }
            Global::msgThread->removeSendMsgNotLock(msg->packetNoString());
            break;

        case IPMSG_ANSENTRY:
        case IPMSG_ANSREADMSG:
        case IPMSG_RELEASEFILES:
        case IPMSG_SENDABSENCEINFO:
        case IPMSG_FEIQ_TYPING:
        case IPMSG_FEIQ_TYPINGSTOP:
            broadcastMsg(msg);
            Global::msgThread->removeSendMsgNotLock(msg->packetNoString());
            break;


        case IPMSG_READMSG:
            broadcastMsg(msg);
            // XXX TODO: If IPMSG_READCHECKOPT is on, we need to check if
            // sucessfully sended.
            break;

        case IPMSG_RECVMSG:
            broadcastMsg(msg);
            // XXX TODO: need to check if sucessfully sended.
            break;

        case IPMSG_SENDMSG:
            broadcastMsg(msg);
            // XXX TODO: need to check if sucessfully sended.
            break;

        default:
            Global::msgThread->removeSendMsgNotLock(msg->packetNoString());
            break;
        }
    }
}

bool MsgServer::isResendNeeded(Msg &msg) const
{
    if (msg->state() == MsgBase::SendFail
        && msg->sendTimes() < MAX_RE_SEND_TIMES) {
        return true;
    }

    return false;
}

void MsgServer::broadcastMsg(Msg &msg)
{
    QByteArray datagram = Global::transferCodec->encodeForFlags(
                msg->packet(), msg->flags());

    if (m_udpSocket.writeDatagram(datagram, msg->ipAddress(),
                                       msg->port()) == -1) {
        msg->setState(MsgBase::SendFail);
    } else { // Send Ok
        msg->setState(MsgBase::SendOk);
    }
}

void MsgServer::updateAddresses()
{
    m_broadcastAddresses.clear();
    m_ipAddresses.clear();

    foreach (QNetworkInterface networkInterface, QNetworkInterface::allInterfaces()) {
        if (!isUsableIpv4Interface(networkInterface)) {
            continue;
        }

        foreach (QNetworkAddressEntry entry, networkInterface.addressEntries()) {
            if (!isUsableIpv4Broadcast(entry)) {
                continue;
            }

            m_broadcastAddresses << entry.broadcast();
            m_ipAddresses << entry.ip();
        }
    }

    // Single broadcast IPs stay one target. start-end and CIDR tokens expand
    // into unicast probes so peers off the local subnet can be found without
    // a directory server. Oversized hand-edited ranges are capped.
    int invalidTokens = 0;
    int truncatedTokens = 0;
    const QList<QHostAddress> specified =
            ProtocolUtil::targetsFromBroadcastTokens(
                Global::preferences->userSpecifiedBroadcastIpList,
                &invalidTokens, &truncatedTokens);
    foreach (const QHostAddress &hostAddress, specified) {
        if (hostAddress.protocol() == QAbstractSocket::IPv4Protocol
            && !hostAddress.isNull()
            && !hostAddress.isLoopback()) {
            m_broadcastAddresses << hostAddress;
        }
    }
    if (invalidTokens > 0 || truncatedTokens > 0) {
        qWarning() << "MsgServer::updateAddresses: broadcast list"
                   << invalidTokens << "invalid,"
                   << truncatedTokens << "truncated to"
                   << ProtocolUtil::maxIpv4RangeHosts();
    }

    const QHostAddress limitedBroadcast(QHostAddress::Broadcast);
    if (!m_broadcastAddresses.contains(limitedBroadcast)) {
        m_broadcastAddresses << limitedBroadcast;
    }

    QSet<QHostAddress> uniqueAddresses(m_broadcastAddresses.begin(),
                                       m_broadcastAddresses.end());
    m_broadcastAddresses = QList<QHostAddress>(uniqueAddresses.begin(),
                                               uniqueAddresses.end());

    qDebug() << "MsgServer::updateAddresses:" << m_broadcastAddresses;
}

void MsgServer::broadcastUserMsg(Msg &msg)
{
    qDebug() << "MsgServer::broadcastUserMsg";

    updateAddresses();

    QByteArray datagram = Global::transferCodec->encodeForFlags(
                msg->packet(), msg->flags());

    if (m_broadcastAddresses.isEmpty()) {
        qWarning() << "MsgServer::broadcastUserMsg: no usable IPv4 broadcast address";
        return;
    }

    QList<quint16> destPorts;
    destPorts << IPMSG_DEFAULT_PORT;
    const quint16 listenPort = Global::preferences->listenPort();
    if (listenPort != IPMSG_DEFAULT_PORT) {
        destPorts << listenPort;
    }

    int successCount = 0;
    foreach (QHostAddress address, m_broadcastAddresses) {
        foreach (quint16 destPort, destPorts) {
            if (m_udpSocket.writeDatagram(datagram, address, destPort) == -1) {
                qWarning() << "MsgServer::broadcastUserMsg: send failed"
                           << address.toString() << destPort
                           << m_udpSocket.errorString();
            } else {
                ++successCount;
            }
        }
    }

    if (successCount == 0) {
        qWarning() << "MsgServer::broadcastUserMsg: all IPv4 broadcasts failed";
        updateAddresses();
    }
}

bool MsgServer::isSupportedCommand(QByteArray &datagram) const
{
    QList<QByteArray> bl = datagram.split(':');

    if (bl.size() < MSG_NORMAL_FIELD_COUNT) {
        return false;
    }

    bool ok = false;
    const quint32 command = Helper::parseIpmsgCommand(bl.at(MSG_FLAGS_POS), &ok);
    if (!ok) {
        return false;
    }

    bool isSupport = false;
    switch (GET_MODE(command)) {
    case IPMSG_NOOPERATION:
    case IPMSG_BR_ENTRY:
    case IPMSG_BR_EXIT:
    case IPMSG_ANSENTRY:
    case IPMSG_BR_ABSENCE:
    case IPMSG_BR_ISGETLIST:
    case IPMSG_OKGETLIST:
    case IPMSG_GETLIST:
    case IPMSG_ANSLIST:
    case IPMSG_BR_ISGETLIST2:
    case IPMSG_SENDMSG:
    case IPMSG_RECVMSG:
    case IPMSG_READMSG:
    case IPMSG_DELMSG:
    case IPMSG_ANSREADMSG:
    case IPMSG_GETINFO:
    case IPMSG_SENDINFO:
    case IPMSG_GETABSENCEINFO:
    case IPMSG_SENDABSENCEINFO:
    case IPMSG_GETFILEDATA:
    case IPMSG_RELEASEFILES:
    case IPMSG_GETDIRFILES:
    case IPMSG_GETPUBKEY:
    case IPMSG_ANSPUBKEY:
    case IPMSG_FEIQ_TYPING:
    case IPMSG_FEIQ_TYPINGSTOP:
        isSupport = true;
        break;

    default:
        isSupport = false;
    }

    return isSupport;
}

void MsgServer::socketEerror(QAbstractSocket::SocketError errorCode)
{
    emit error(errorCode, m_udpSocket.errorString());
}

