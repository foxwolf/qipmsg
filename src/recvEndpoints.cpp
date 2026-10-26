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

#include "recvEndpoints.h"
#include "protocolutil.h"

#include <QHostAddress>

#include <algorithm>

namespace {

const int kMaxRecvEndpoints = 4;

bool ipv4Value(const QString &text, quint32 *host)
{
    QHostAddress address;
    if (!address.setAddress(text.trimmed())) {
        return false;
    }
    if (address.protocol() != QAbstractSocket::IPv4Protocol) {
        return false;
    }
    if (host) {
        *host = address.toIPv4Address();
    }
    return true;
}

bool usableFileHost(quint32 host)
{
    if (host == 0 || host == 0xFFFFFFFFu) {
        return false;
    }
    const quint32 lead = host >> 24;
    if (lead == 0 || lead == 127 || lead >= 224) {
        return false;
    }
    return true;
}

bool likelyLanEdge(quint32 host)
{
    const quint32 last = host & 0xFFu;
    return last == 0u || last == 255u;
}

bool same24(quint32 left, quint32 right)
{
    return (left & 0xFFFFFF00u) == (right & 0xFFFFFF00u);
}

bool sameIdentityText(const QString &left, const QString &right)
{
    return left.trimmed().compare(right.trimmed(), Qt::CaseInsensitive) == 0;
}

struct EndpointRank {
    QString ip;
    quint16 port;
    int rank;
    int order;
};

} // namespace

QList<RecvEndpoint> recvFileEndpoints(const QString &sourceIp,
                                      quint16 sourcePort,
                                      const QList<RecvPeerHint> &peers,
                                      const QStringList &configuredTokens,
                                      quint16 defaultPort)
{
    quint16 offerPort = sourcePort == 0 ? defaultPort : sourcePort;
    if (offerPort == 0) {
        offerPort = 2425;
    }

    quint32 sourceHost = 0;
    const bool sourceOk = ipv4Value(sourceIp, &sourceHost)
            && usableFileHost(sourceHost);
    const QString source = sourceOk ? QHostAddress(sourceHost).toString()
                                    : QString();

    QList<RecvEndpoint> out;
    if (sourceOk) {
        RecvEndpoint primary;
        primary.ip = source;
        primary.port = offerPort;
        out.append(primary);
    }

    QString login;
    QString hostName;
    QString group;
    bool haveIdentity = false;
    for (int i = 0; i < peers.size(); ++i) {
        const RecvPeerHint &peer = peers.at(i);
        quint32 host = 0;
        if (!ipv4Value(peer.ip, &host) || !usableFileHost(host)) {
            continue;
        }
        if (!sourceOk || host != sourceHost) {
            continue;
        }
        const QString peerLogin = peer.login.trimmed();
        const QString peerHost = peer.host.trimmed();
        if (peerLogin.isEmpty() || peerHost.isEmpty()) {
            continue;
        }
        login = peerLogin;
        hostName = peerHost;
        group = peer.group.trimmed();
        haveIdentity = true;
        break;
    }

    QList<quint32> identityHosts;
    if (haveIdentity) {
        for (int i = 0; i < peers.size(); ++i) {
            const RecvPeerHint &peer = peers.at(i);
            if (!sameIdentityText(peer.login, login)
                    || !sameIdentityText(peer.host, hostName)
                    || !sameIdentityText(peer.group, group)) {
                continue;
            }
            quint32 host = 0;
            if (!ipv4Value(peer.ip, &host) || !usableFileHost(host)) {
                continue;
            }
            identityHosts.append(host);
        }
    }

    QList<EndpointRank> extra;
    int seq = 0;
    auto appendUnique = [&](const QString &ip, quint16 port, int rank) -> bool {
        if (sourceOk && ip == source && port == offerPort) {
            return false;
        }
        for (int i = 0; i < extra.size(); ++i) {
            if (extra.at(i).ip == ip && extra.at(i).port == port) {
                return false;
            }
        }
        EndpointRank item;
        item.ip = ip;
        item.port = port;
        item.rank = rank;
        item.order = seq++;
        extra.append(item);
        return true;
    };

    if (haveIdentity) {
        for (int i = 0; i < peers.size(); ++i) {
            const RecvPeerHint &peer = peers.at(i);
            if (!sameIdentityText(peer.login, login)
                    || !sameIdentityText(peer.host, hostName)
                    || !sameIdentityText(peer.group, group)) {
                continue;
            }
            quint32 host = 0;
            if (!ipv4Value(peer.ip, &host) || !usableFileHost(host)) {
                continue;
            }
            const QString ip = QHostAddress(host).toString();
            quint16 port = peer.port == 0 ? offerPort : peer.port;
            const int rank = (sourceOk && same24(host, sourceHost)) ? 0 : 3;
            appendUnique(ip, port, rank);
        }
    }

    bool addedLoose = false;
    for (int i = 0; i < configuredTokens.size(); ++i) {
        quint32 start = 0;
        quint32 end = 0;
        const int status = ProtocolUtil::inspectIpv4Token(
                    configuredTokens.at(i), 0, &start, &end);
        if (status != ProtocolUtil::Ipv4TokenOk || start != end) {
            continue;
        }
        if (!usableFileHost(start) || likelyLanEdge(start)) {
            continue;
        }
        const QString ip = QHostAddress(start).toString();
        const bool nearSource = sourceOk && same24(start, sourceHost);
        bool nearSibling = false;
        for (int s = 0; s < identityHosts.size(); ++s) {
            if (sourceOk && identityHosts.at(s) == sourceHost) {
                continue;
            }
            if (same24(start, identityHosts.at(s))) {
                nearSibling = true;
                break;
            }
        }
        int rank = 4;
        if (nearSource) {
            rank = 1;
        } else if (nearSibling) {
            rank = 2;
        } else if (addedLoose) {
            continue;
        }
        if (appendUnique(ip, offerPort, rank) && rank == 4) {
            addedLoose = true;
        }
    }

    bool haveNearConfig = false;
    for (int i = 0; i < extra.size(); ++i) {
        if (extra.at(i).rank == 1 || extra.at(i).rank == 2) {
            haveNearConfig = true;
            break;
        }
    }
    if (haveNearConfig) {
        QList<EndpointRank> kept;
        for (int i = 0; i < extra.size(); ++i) {
            if (extra.at(i).rank != 4) {
                kept.append(extra.at(i));
            }
        }
        extra = kept;
    }

    std::sort(extra.begin(), extra.end(),
              [](const EndpointRank &left, const EndpointRank &right) {
        if (left.rank != right.rank) {
            return left.rank < right.rank;
        }
        return left.order < right.order;
    });

    // Keep a configured same-subnet address even when several sibling
    // NICs would otherwise fill the list.
    const int rankLimit[] = { 2, 1, 1, 4, 1 };
    for (int rank = 0; rank < 5; ++rank) {
        if (rank == 4 && haveNearConfig) {
            continue;
        }
        int taken = 0;
        for (int i = 0; i < extra.size() && taken < rankLimit[rank]; ++i) {
            if (out.size() >= kMaxRecvEndpoints) {
                break;
            }
            if (extra.at(i).rank != rank) {
                continue;
            }
            RecvEndpoint ep;
            ep.ip = extra.at(i).ip;
            ep.port = extra.at(i).port;
            out.append(ep);
            ++taken;
        }
    }

    if (out.isEmpty()) {
        RecvEndpoint ep;
        ep.ip = sourceIp.trimmed();
        ep.port = offerPort;
        out.append(ep);
    }
    return out;
}
