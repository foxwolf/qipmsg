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

#ifndef PEERFOLD_H
#define PEERFOLD_H

#include <QDateTime>
#include <QList>
#include <QString>
#include <QStringList>

// One address of a peer, as stored per UDP source. The contact list
// folds several of these into one row. File receive still sees each
// address, so a FeiQ box that announces 192.168.14.164 and also listens
// on 192.168.14.3 can be tried on both.
struct PeerFoldEnd {
    QString ip;
    quint16 port;
    bool fresh;
    QDateTime seen;

    PeerFoldEnd() : port(0), fresh(false) {}
};

// One input row for peerFoldGroups. Indexes in the result point back
// into the list the caller passed in (the UserManager model row, when
// that list was built in row order).
struct PeerFoldInput {
    QString ip;
    QString login;
    QString host;
    QString qipToken;
    quint16 port;
    bool fresh;
    QDateTime seen;

    PeerFoldInput() : port(0), fresh(false) {}
};

struct PeerFoldGroup {
    QList<int> indexes;
    int preferred;

    PeerFoldGroup() : preferred(-1) {}
};

// Case-insensitive "login\\nhost". Empty when login or host is blank,
// so two anonymous rows are not glued together. Display nick is not
// part of the key: FeiQ FoxWolf shows as "win" on WORK-LAPTOP and must
// still match its other NIC. A different host stays a different peer
// even when the login compares equal (foxwolf@hw versus
// FoxWolf@WORK-LAPTOP versus FoxWolf@MXGS-WORK).
QString peerFoldKey(const QString &login, const QString &host);

// Machine id carried in the qipmsg extra field, or empty.
// The usual token is "qipmsg" plus share/avatar tags. Those tags are
// the product and the picture hash, not the computer, so they do not
// split two NICs. An optional "id:<value>" segment does: two different
// ids are two installs that happen to share a hostname.
QString peerMachineId(const QString &qipToken);

// Higher is a better send target. 192.168/16 and 172.16/12 rank above
// 10/8. The captured ZeroTier network is 10.147.19.0/24; that prefix
// is not hard-coded, so a site whose only LAN is 10/8 still works.
// Loopback, link-local, multicast and empty addresses rank at the bottom.
int peerAddrSendRank(const QString &ip);

// Pick the index to use for chat and file send.
// Order: a fresh address (online or away) beats a stale one, then a
// higher peerAddrSendRank, then a newer lastSeen, then the lower IPv4
// so the choice does not flap. Returns -1 when ends is empty.
int preferredPeerIndex(const QList<PeerFoldEnd> &ends);

// Group rows that are the same login+host. Rows with no login or no
// host stay alone. When one login+host carries two different id:
// values, those installs are split; a row with no id stays with the
// install that already has the most rows.
QList<PeerFoldGroup> peerFoldGroups(const QList<PeerFoldInput> &peers);

// True when this announcement is our own process, or a leftover of it.
// 127.0.0.1 is always ours (including a port-0 test row). The same
// login and host as this machine is ours on every NIC, including an
// address that is no longer configured. A local NIC plus our host
// name is ours even when the login field was a nick. FeiQ is not us
// just because the login compares equal: the host must match too.
bool peerIsSelf(const QString &login, const QString &host, const QString &ip,
                const QString &ourLogin, const QString &ourHost,
                const QStringList &localIps);

#endif // !PEERFOLD_H
