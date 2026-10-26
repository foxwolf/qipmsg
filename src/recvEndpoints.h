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

#ifndef RECVENDPOINTS_H
#define RECVENDPOINTS_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QtGlobal>

// One known contact. Used only to pick a TCP file port when the UDP
// source address is not the address that is listening.
struct RecvPeerHint {
    QString ip;
    quint16 port;
    QString login;
    QString host;
    QString group;
};

struct RecvEndpoint {
    QString ip;
    quint16 port;
};

// Targets for an incoming file offer.
// The UDP source is first. Later entries are tried only after connect
// fails, times out, or the peer closes before any file bytes:
//   - other contacts with the same login, host, and group
//   - one IPv4 from the user broadcast list (not a range or CIDR)
// Addresses in the same /24 as the offer, or as another address of that
// contact, come first. At most four targets are returned.
QList<RecvEndpoint> recvFileEndpoints(const QString &sourceIp,
                                      quint16 sourcePort,
                                      const QList<RecvPeerHint> &peers,
                                      const QStringList &configuredTokens,
                                      quint16 defaultPort);

#endif // !RECVENDPOINTS_H
