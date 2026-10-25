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

#ifndef PORTBIND_H
#define PORTBIND_H

#include <QAbstractSocket>
#include <QHostAddress>
#include <QList>
#include <QString>

class QTcpServer;
class QUdpSocket;

// Why a UDP or TCP listen on the shared IPMSG port failed.
enum class portFailKind {
    none = 0,
    inUse,
    accessDenied,
    addressUnavailable,
    other
};

struct portSideResult {
    bool ok;
    portFailKind kind;
    QString error;
    bool windowsExcluded;

    portSideResult()
        : ok(false), kind(portFailKind::other), windowsExcluded(false) {}
};

struct portListenResult {
    quint16 port;
    portSideResult udp;
    portSideResult tcp;

    portListenResult()
        : port(0) {}

    bool ok() const { return udp.ok && tcp.ok; }
};

// Probe and classify bind failures. The real sockets still use the
// existing share flags; a probe binds exclusively so FeiQ holding 2425
// is visible, then releases the port before the caller listens for real.
class PortBind {
public:
    static portFailKind classify(QAbstractSocket::SocketError error,
                                 const QString &errorString);
    static bool textContainsPort(const QString &text, quint16 port);
    static QList<quint16> candidatePorts(quint16 avoid);

    static portSideResult tryUdp(QUdpSocket *socket,
                                 const QHostAddress &address,
                                 quint16 port,
                                 QAbstractSocket::BindMode mode,
                                 bool retryInUse);
    static portSideResult tryTcp(QTcpServer *server,
                                 const QHostAddress &address,
                                 quint16 port,
                                 bool retryInUse);

    static portListenResult probe(const QHostAddress &address, quint16 port);
    static void annotateExcluded(portListenResult *result);
    static quint16 suggestFree(const QHostAddress &address, quint16 avoid);
};

#endif // !PORTBIND_H
