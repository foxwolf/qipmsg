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

#include "msgCrypto.h"
#include "global.h"
#include "groupPolicy.h"
#include "ipmsg.h"
#include "msg_thread.h"
#include "preferences.h"
#include "send_msg.h"
#include "transfer_codec.h"
#include "user_manager.h"

#include <QHostAddress>
#include <QSet>

namespace {

QSet<QString> g_asked;

bool encryptionWanted()
{
    return Global::preferences && Global::preferences->isEncryptMsg;
}

bool mayProbe(const QString &ip)
{
    if (!encryptionWanted() || ip.isEmpty() || !Global::userManager) {
        return false;
    }
    if (Global::preferences->isStealth) {
        return false;
    }
    const int row = Global::userManager->ipToRow(ip);
    if (row < 0) {
        return true;
    }
    const QString group = Global::userManager->group(row);
    if (Global::groupPolicy && Global::groupPolicy->isHidden(group)) {
        return false;
    }
    return true;
}

} // namespace

bool MsgCrypto::prepareOutgoing(const QString &ip, QString *additionalInfo,
                                quint32 *flags)
{
    if (!additionalInfo || !flags || additionalInfo->isEmpty()) {
        return false;
    }
    if ((*flags & IPMSG_FILEATTACHOPT) != 0) {
        return false;
    }
    const int nul = additionalInfo->indexOf(QChar('\0'));
    if (nul >= 0 && nul < additionalInfo->size() - 1) {
        return false;
    }
    if (!encryptionWanted() || !hasPeerKey(ip)) {
        if (encryptionWanted()) {
            const quint16 port = Global::userManager
                    ? Global::userManager->sendPortForIp(ip) : 0;
            requestPeerKey(ip, port);
        }
        return false;
    }
    QByteArray plain;
    if (Global::transferCodec) {
        plain = Global::transferCodec->encodeForFlags(*additionalInfo, *flags);
    } else if ((*flags & IPMSG_UTF8OPT) != 0) {
        plain = additionalInfo->toUtf8();
    } else {
        plain = additionalInfo->toUtf8();
    }
    if (plain.isEmpty()) {
        return false;
    }
    QString envelope;
    if (!sealForPeer(ip, plain, &envelope)) {
        return false;
    }
    *additionalInfo = envelope;
    *flags |= IPMSG_ENCRYPTOPT;
    return true;
}

bool MsgCrypto::unwrapIncoming(quint32 flags, QString *additionalInfo,
                               QString *extendedInfo)
{
    if (!additionalInfo) {
        return false;
    }
    QByteArray plain;
    if (!openEnvelope(*additionalInfo, &plain)) {
        if (extendedInfo) {
            extendedInfo->clear();
        }
        additionalInfo->clear();
        return false;
    }
    QString text;
    if ((flags & IPMSG_UTF8OPT) != 0) {
        text = QString::fromUtf8(plain);
    } else if (Global::transferCodec) {
        text = Global::transferCodec->decode(plain);
    } else {
        text = QString::fromUtf8(plain);
    }
    const int nul = text.indexOf(QChar('\0'));
    if (nul < 0) {
        *additionalInfo = text;
        if (extendedInfo) {
            extendedInfo->clear();
        }
    } else {
        *additionalInfo = text.left(nul);
        if (extendedInfo) {
            *extendedInfo = text.mid(nul + 1);
        }
    }
    return true;
}

void MsgCrypto::requestPeerKey(const QString &ip, quint16 port)
{
    if (!mayProbe(ip) || !Global::msgThread || port == 0) {
        return;
    }
    if (g_asked.contains(ip) && hasPeerKey(ip)) {
        return;
    }
    g_asked.insert(ip);
    const QString body = localAnswerBody().section(QLatin1Char(':'), 0, 0);
    if (body.isEmpty()) {
        g_asked.remove(ip);
        return;
    }
    SendMsg sendMsg(QHostAddress(ip), port, body, QString(), IPMSG_GETPUBKEY);
    Global::msgThread->addSendMsg(Msg(sendMsg));
}

void MsgCrypto::requestReachablePeers()
{
    if (!encryptionWanted() || !Global::userManager) {
        return;
    }
    const int rows = Global::userManager->userCount();
    for (int row = 0; row < rows; ++row) {
        const QString ip = Global::userManager->ip(row);
        if (!Global::userManager->isReachable(ip)) {
            continue;
        }
        requestPeerKey(ip, Global::userManager->sendPortForIp(ip));
    }
}
