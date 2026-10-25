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

#ifndef PROTOCOLUTIL_H
#define PROTOCOLUTIL_H

#include "ipmsg.h"

#include <QByteArray>
#include <QHostAddress>
#include <QList>
#include <QString>
#include <QStringList>

// Pure helpers for cross-subnet discovery tokens and IPMSG option bits.
// No sockets and no preferences, so the self-check can link this alone.
class ProtocolUtil
{
public:
    enum Ipv4TokenStatus {
        Ipv4TokenOk = 0,
        Ipv4TokenEmpty = 1,
        Ipv4TokenInvalid = 2,
        Ipv4TokenBadPrefix = 3,
        Ipv4TokenTooLarge = 4,
        Ipv4TokenUnusable = 5
    };

    static int maxIpv4RangeHosts();

    // Accept a single IPv4, "start-end" / "start~end", or CIDR "addr/prefix".
    // On success canonical is a stable list key. On TooLarge, start/end are
    // still filled so a hand-edited list can be capped at probe time.
    static int inspectIpv4Token(const QString &raw, QString *canonical,
                                quint32 *startHost, quint32 *endHost);

    static QStringList expandIpv4Hosts(quint32 startHost, quint32 endHost,
                                       bool *truncated);

    static QList<QHostAddress> targetsFromBroadcastTokens(
            const QStringList &tokens, int *invalidCount, int *truncatedCount);

    // Flags field is ASCII decimal before the message body, so the codec
    // can be chosen without decoding the body first.
    static bool commandFromDatagram(const QByteArray &datagram, quint32 *command);

    // SECRETOPT shares its bit with SERVEROPT. Only SENDMSG is an envelope.
    // READCHECKOPT matches the existing seal (MsgWindow::isSealed).
    static bool isSecretEnvelope(quint32 flags);

    // READMSG carries the original SENDMSG packet number in the message body
    // (IPMSG "extended area"). Some peers leave the body empty and reuse the
    // READMSG packet-number field. Returns empty when neither is a packet id.
    static QString readReceiptPacketNo(const QString &additionalInfo,
                                       const QString &packetNoField);

    // FeiQ typing commands are decimal 121 / 122 with no option bits.
    static bool isFeiqTypingCommand(quint32 command, bool *active);

    // Seconds left from observed throughput. Negative when unknown.
    static int etaSeconds(qint64 bytesSent, qint64 bytesTotal,
                          qint64 elapsedMs);
};

#endif // !PROTOCOLUTIL_H
