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

#include "peerFold.h"

#include <QHash>
#include <QHostAddress>
#include <QStringList>

namespace {

bool sameText(const QString &left, const QString &right)
{
    return left.trimmed().compare(right.trimmed(), Qt::CaseInsensitive) == 0;
}

bool ipv4Host(const QString &text, quint32 *host)
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

QString canonicalIp(const QString &text)
{
    quint32 host = 0;
    if (!ipv4Host(text, &host)) {
        return text.trimmed();
    }
    return QHostAddress(host).toString();
}

// Ranks used by peerAddrSendRank. Freshness is applied by the caller
// before this number, so a live 10/8 address still beats a stale LAN.
const int kRankUnusable = 0;
const int kRankLinkLocal = 10;
const int kRankPublic = 100;
const int kRankTenSlash = 200;
const int kRankLan = 300;

bool endBeats(const PeerFoldEnd &cand, const PeerFoldEnd &incumbent)
{
    // Online or away means we just heard this NIC. A grey row may be
    // yesterday's WLAN and must not steal the send target.
    if (cand.fresh != incumbent.fresh) {
        return cand.fresh;
    }

    const int candRank = peerAddrSendRank(cand.ip);
    const int oldRank = peerAddrSendRank(incumbent.ip);
    if (candRank != oldRank) {
        return candRank > oldRank;
    }

    if (cand.seen.isValid() || incumbent.seen.isValid()) {
        if (!incumbent.seen.isValid()) {
            return cand.seen.isValid();
        }
        if (!cand.seen.isValid()) {
            return false;
        }
        if (cand.seen != incumbent.seen) {
            return cand.seen > incumbent.seen;
        }
    }

    // Same class and the same timestamp: keep the numerically lower
    // IPv4 so two fresh LAN addresses do not trade places every refresh.
    quint32 candHost = 0;
    quint32 oldHost = 0;
    const bool candOk = ipv4Host(cand.ip, &candHost);
    const bool oldOk = ipv4Host(incumbent.ip, &oldHost);
    if (candOk && oldOk && candHost != oldHost) {
        return candHost < oldHost;
    }
    return canonicalIp(cand.ip) < canonicalIp(incumbent.ip);
}

int preferredAmong(const QList<PeerFoldInput> &peers, const QList<int> &indexes)
{
    int bestPos = -1;
    for (int n = 0; n < indexes.size(); ++n) {
        const int index = indexes.at(n);
        if (index < 0 || index >= peers.size()) {
            continue;
        }
        if (peers.at(index).ip.trimmed().isEmpty()) {
            continue;
        }
        if (bestPos < 0) {
            bestPos = n;
            continue;
        }
        const PeerFoldInput &cand = peers.at(index);
        const PeerFoldInput &old = peers.at(indexes.at(bestPos));
        PeerFoldEnd candEnd;
        candEnd.ip = cand.ip;
        candEnd.port = cand.port;
        candEnd.fresh = cand.fresh;
        candEnd.seen = cand.seen;
        PeerFoldEnd oldEnd;
        oldEnd.ip = old.ip;
        oldEnd.port = old.port;
        oldEnd.fresh = old.fresh;
        oldEnd.seen = old.seen;
        if (endBeats(candEnd, oldEnd)) {
            bestPos = n;
        }
    }
    if (bestPos < 0) {
        return indexes.isEmpty() ? -1 : indexes.first();
    }
    return indexes.at(bestPos);
}

} // namespace

QString peerFoldKey(const QString &login, const QString &host)
{
    const QString loginKey = login.trimmed().toLower();
    const QString hostKey = host.trimmed().toLower();
    // Both halves are required. Folding on login alone would merge
    // foxwolf@hw with FoxWolf@WORK-LAPTOP. Folding on host alone would
    // merge two accounts on one shared machine name.
    if (loginKey.isEmpty() || hostKey.isEmpty()) {
        return QString();
    }
    return loginKey + QLatin1Char('\n') + hostKey;
}

QString peerMachineId(const QString &qipToken)
{
    // BR_ENTRY field 4 from qipmsg looks like "qipmsg", "qipmsg,share",
    // "qipmsg,share,lock", or the same plus ",av:<12 hex>". FeiQ sends
    // nothing here. "qipmsg" is every qipmsg peer. "av:" hashes the
    // photo, so two people with the same picture would collide, and
    // one machine's two NICs can briefly disagree while a photo is
    // published. Neither piece may become the identity.
    //
    // "id:<value>" is reserved for a stable install id. Current builds
    // do not emit it. When both sides have one and they differ, the
    // caller keeps them apart (a cloned hostname). Comparison is
    // case-insensitive and ignores the product tags around it.
    const QStringList parts = qipToken.split(QLatin1Char(','));
    for (int i = 0; i < parts.size(); ++i) {
        const QString part = parts.at(i).trimmed();
        if (part.size() <= 3) {
            continue;
        }
        if (!part.startsWith(QLatin1String("id:"), Qt::CaseInsensitive)) {
            continue;
        }
        const QString value = part.mid(3).trimmed().toLower();
        if (value.isEmpty()) {
            continue;
        }
        return QLatin1String("id:") + value;
    }
    return QString();
}

int peerAddrSendRank(const QString &ip)
{
    quint32 host = 0;
    if (!ipv4Host(ip, &host)) {
        return kRankUnusable;
    }
    const quint32 lead = host >> 24;
    // 0.0.0.0, 255.255.255.255, multicast, and loopback are not places
    // to send a chat. 127.0.0.1 is our own test leftover.
    if (host == 0 || host == 0xFFFFFFFFu || lead == 0 || lead >= 224
            || lead == 127) {
        return kRankUnusable;
    }
    // 169.254/16 is link-local. It comes and goes with the cable.
    if ((host & 0xFFFF0000u) == 0xA9FE0000u) {
        return kRankLinkLocal;
    }
    // Site LAN. The hw Ethernet address 192.168.4.170 and the FeiQ
    // address 192.168.14.164 both land here, above ZeroTier.
    if ((host & 0xFFFF0000u) == 0xC0A80000u) {
        return kRankLan;
    }
    // 172.16.0.0/12, same class as 192.168/16.
    if ((host & 0xFFF00000u) == 0xAC100000u) {
        return kRankLan;
    }
    // 10/8 is where this deployment's ZeroTier lives (10.147.19.0/24)
    // and also where many office LANs live. Rank it under 192.168 and
    // 172.16 so "LAN over ZeroTier when both are fresh" holds, without
    // pinning a ZeroTier prefix that the user can change.
    if (lead == 10) {
        return kRankTenSlash;
    }
    return kRankPublic;
}

int preferredPeerIndex(const QList<PeerFoldEnd> &ends)
{
    int best = -1;
    for (int i = 0; i < ends.size(); ++i) {
        if (ends.at(i).ip.trimmed().isEmpty()) {
            continue;
        }
        if (best < 0) {
            best = i;
            continue;
        }
        if (endBeats(ends.at(i), ends.at(best))) {
            best = i;
        }
    }
    return best;
}

QList<PeerFoldGroup> peerFoldGroups(const QList<PeerFoldInput> &peers)
{
    QList<PeerFoldGroup> groups;
    QHash<QString, QList<int>> byKey;
    QStringList keyOrder;

    for (int i = 0; i < peers.size(); ++i) {
        const QString key = peerFoldKey(peers.at(i).login, peers.at(i).host);
        if (key.isEmpty()) {
            // No shared identity. Keep the row so a half-parsed packet
            // is still visible, but do not attach it to anyone else.
            PeerFoldGroup alone;
            alone.indexes.append(i);
            alone.preferred = i;
            groups.append(alone);
            continue;
        }
        if (!byKey.contains(key)) {
            keyOrder.append(key);
        }
        byKey[key].append(i);
    }

    for (int k = 0; k < keyOrder.size(); ++k) {
        const QList<int> rows = byKey.value(keyOrder.at(k));
        QHash<QString, QList<int>> byId;
        QStringList idOrder;
        QList<int> noId;
        for (int n = 0; n < rows.size(); ++n) {
            const int index = rows.at(n);
            const QString id = peerMachineId(peers.at(index).qipToken);
            if (id.isEmpty()) {
                noId.append(index);
                continue;
            }
            if (!byId.contains(id)) {
                idOrder.append(id);
            }
            byId[id].append(index);
        }

        // Zero or one install id: every NIC of this login+host is one
        // person. A FeiQ row has an empty token and still joins the
        // qipmsg row of the same login and host. Today's "qipmsg" and
        // "qipmsg,av:..." tokens have no id: segment, so they take this
        // path and the two foxwolf NICs stay one contact.
        if (idOrder.size() <= 1) {
            PeerFoldGroup group;
            group.indexes = rows;
            group.preferred = preferredAmong(peers, rows);
            groups.append(group);
            continue;
        }

        // Two cloned hostnames with different id: values. Split them.
        // Rows that never sent an id (an old peers.json line, or FeiQ)
        // hang off the id that already has the most addresses, which is
        // the machine we have actually seen twice. Tie: the first id.
        int hostSlot = 0;
        int hostCount = -1;
        for (int n = 0; n < idOrder.size(); ++n) {
            const int count = byId.value(idOrder.at(n)).size();
            if (count > hostCount) {
                hostCount = count;
                hostSlot = n;
            }
        }
        for (int n = 0; n < idOrder.size(); ++n) {
            QList<int> indexes = byId.value(idOrder.at(n));
            if (n == hostSlot) {
                for (int m = 0; m < noId.size(); ++m) {
                    indexes.append(noId.at(m));
                }
            }
            PeerFoldGroup group;
            group.indexes = indexes;
            group.preferred = preferredAmong(peers, indexes);
            groups.append(group);
        }
    }
    return groups;
}

bool peerIsSelf(const QString &login, const QString &host, const QString &ip,
                const QString &ourLogin, const QString &ourHost,
                const QStringList &localIps)
{
    quint32 hostNum = 0;
    const bool isIpv4 = ipv4Host(ip, &hostNum);
    // The peers.json row 127.0.0.1:0 is a local test leftover. Any
    // login on plain loopback is this machine. 127.0.0.2 is left
    // alone so a unit-test peer such as pytest can still be listed.
    if (isIpv4 && hostNum == 0x7F000001u) {
        return true;
    }

    const bool loginOk = !ourLogin.trimmed().isEmpty()
            && sameText(login, ourLogin);
    const bool hostOk = !ourHost.trimmed().isEmpty()
            && sameText(host, ourHost);
    // foxwolf@hw on 192.168.4.170, 10.147.19.113, the deprecated WLAN
    // 10.168.1.210, and the old 192.168.1.50 are all this PC. Case
    // does not matter. FoxWolf@WORK-LAPTOP fails the host check.
    if (loginOk && hostOk) {
        return true;
    }

    if (!isIpv4 || !hostOk) {
        return false;
    }
    // A datagram that left one of our NICs and names our host is us,
    // even when the login field is the nick rather than the OS user.
    // A neighbor is not removed just because the login matches.
    const QString canon = QHostAddress(hostNum).toString();
    for (int i = 0; i < localIps.size(); ++i) {
        if (canonicalIp(localIps.at(i)) == canon) {
            return true;
        }
    }
    return false;
}
