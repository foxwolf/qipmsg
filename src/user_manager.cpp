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
#include "owner.h"
#include "global.h"
#include "helper.h"
#include "send_msg.h"
#include "user_manager.h"
#include "shareFolder.h"
#include "avatarStore.h"
#include "avatarFetch.h"
#include "avatarWire.h"
#include "preferences.h"
#include "constants.h"
#include "msg_thread.h"
#include "transfer_codec.h"
#include "sound_thread.h"
#include "groupPolicy.h"
#include "protocolutil.h"
#include "peerFold.h"

#include <QColor>
#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkInterface>
#include <QSet>
#include <QStandardItemModel>
#include <QStringList>
#include <QTimer>

Owner UserManager::m_ourself;

UserManager::UserManager(QObject *parent)
    : QObject(parent), m_refreshTimer(0), m_saveTimer(0),
      m_suspendPeerPersist(false), m_presenceSoundArmed(false)
{
    updateOurself();
    createModel();
    // The save timer does not exist yet. Remember a dropped self row
    // and rewrite peers.json once the timer is running.
    const bool droppedSelf = loadPersistedPeers();

    // Persisted peers and the first discovery burst must not beep.
    QTimer::singleShot(PRESENCE_SOUND_ARM_MS, this, [this]() {
        m_presenceSoundArmed = true;
    });

    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, SIGNAL(timeout()),
            this, SLOT(autoRefreshPresence()));
    applyRefreshTimer();

    m_saveTimer = new QTimer(this);
    m_saveTimer->setSingleShot(true);
    connect(m_saveTimer, SIGNAL(timeout()),
            this, SLOT(savePersistedPeers()));
    if (droppedSelf) {
        // peers.json still has foxwolf@hw on every old NIC, and
        // 127.0.0.1. Those rows were not loaded. Write the file back
        // without them so the next start stays quiet.
        scheduleSavePeers();
    }
}

UserManager::~UserManager()
{
    savePersistedPeers();
}

void UserManager::updateOurself()
{
    if (Global::preferences->userName.isEmpty()) {
        m_ourself.setName(Helper::loginName());
    } else {
        m_ourself.setName(Global::preferences->userName);
    }

    m_ourself.setGroup(Global::preferences->groupName);
    m_ourself.setLoginName(Helper::loginName());
    m_ourself.setHost(Helper::hostname());
    m_ourself.setDisplayLevel(Global::preferences->displayLevel);
    m_ourself.setSignature(Global::preferences->personSign);

    // A nickname or hostname change can turn rows we already stored
    // into "this machine". Drop them after the model exists. The
    // constructor calls us before createModel, so the null check matters.
    pruneSelfRows();
}

void UserManager::createModel()
{
    QStringList labels;
    labels << tr("user") << tr("group") << tr("host") << tr("ip")
        << tr("logname") << tr("display level") << tr("status");

    m_model = new QStandardItemModel(0, labels.size(), this);
    m_model->setHorizontalHeaderLabels(labels);
}

void UserManager::newUserMsg(Msg msg)
{
    // Our own BR_ENTRY arrives once per NIC (Ethernet and ZeroTier on
    // hw, plus whatever address we used last week). Those rows are not
    // contacts. FeiQ with the same login and a different host is not
    // caught here and is inserted below like any other peer.
    if (isSelfPeer(msg->owner().loginName(), msg->owner().host(), msg->ip())) {
        const int row = ipToRow(msg->ip());
        if (row != -1) {
            m_model->removeRow(row);
            scheduleSavePeers();
            emit userCountUpdated(listedPeerCount());
            notifyUsersChanged();
        }
        return;
    }

    if (!msg->owner().group().isEmpty()
            && !Global::preferences->groupNameList
            .contains(msg->owner().group())) {
        Global::preferences->groupNameList.prepend(msg->owner().group());
    }

    const bool absence =
        GET_MODE(msg->flags()) == IPMSG_BR_ABSENCE
        || (msg->flags() & IPMSG_ABSENCEOPT);

    const bool dialup = ProtocolUtil::isDialupOption(msg->flags());
    if (contains(msg->ip())) {
        const int row = ipToRow(msg->ip());
        const QString previousGroup = group(row);
        updateUser(msg->owner(), row);
        touchUser(row, absence);
        applyDialupFlag(row, dialup);
        scheduleSavePeers();
        notifyUsersChanged();
        reconcileGroupVisibility(msg->ip(), previousGroup, msg->owner().group());
    } else {
        int row = m_model->rowCount();
        m_model->insertRow(row);
        addUser(msg->owner(), row);
        touchUser(row, absence);
        applyDialupFlag(row, dialup);
        scheduleSavePeers();

        // Tray and the level menu count people, not source addresses.
        emit userCountUpdated(listedPeerCount());
        notifyUsersChanged();
    }
    notePeerCaps(msg->ip(), msg->flags());
    if (Global::preferences && Global::preferences->isEncryptMsg) {
        MsgCrypto::requestPeerKey(msg->ip(), sendPortForIp(msg->ip()));
    }
}

void UserManager::newExitMsg(Msg msg)
{
    int row;
    if ((row = ipToRow(msg->ip())) != -1) {
        setRowPresence(row, PRESENCE_STALE);
        scheduleSavePeers();
        notifyUsersChanged();
    }
}

void UserManager::updateUser(const Owner &owner, int row)
{
    m_model->setData(m_model->index(row, USER_VIEW_NAME_COLUMN),
                       owner.name());
    m_model->setData(m_model->index(row, USER_VIEW_GROUP_COLUMN),
                       owner.group());
    m_model->setData(m_model->index(row, USER_VIEW_HOST_COLUMN),
                       owner.host());
    m_model->setData(m_model->index(row, USER_VIEW_IP_COLUMN),
                       owner.ip());
    m_model->setData(m_model->index(row, USER_VIEW_LOGIN_NAME_COLUMN),
                       owner.loginName());
    m_model->setData(m_model->index(row, USER_VIEW_DISPLAY_LEVEL_COLUMN),
                       owner.displayLevel());
    m_model->setData(m_model->index(row, USER_VIEW_IP_COLUMN),
                     owner.signature(), Qt::UserRole + 2);
    m_model->setData(m_model->index(row, USER_VIEW_IP_COLUMN),
                     owner.qipToken(), Qt::UserRole + 6);
    if (owner.port() != 0) {
        m_model->setData(m_model->index(row, USER_VIEW_IP_COLUMN),
                         owner.port(), Qt::UserRole + 3);
    }
    const QString tip = owner.signature().isEmpty()
            ? owner.name()
            : QString("%1\n%2").arg(owner.name(), owner.signature());
    m_model->setData(m_model->index(row, USER_VIEW_NAME_COLUMN),
                     tip, Qt::ToolTipRole);
    if (Global::avatarFetch && Global::userManager) {
        Global::avatarFetch->consider(owner.ip());
    }
}

void UserManager::addUser(const Owner &owner, int row)
{
    updateUser(owner, row);
}

void UserManager::touchUser(int row, bool absence)
{
    m_model->setData(m_model->index(row, USER_VIEW_IP_COLUMN),
                     QDateTime::currentDateTime(), Qt::UserRole);
    setRowPresence(row, absence ? PRESENCE_AWAY : PRESENCE_ONLINE);
}

void UserManager::setRowPresence(int row, int presence)
{
    const QModelIndex ipIndex = m_model->index(row, USER_VIEW_IP_COLUMN);
    const QVariant previous = m_model->data(ipIndex, Qt::UserRole + 1);
    const int oldPresence = previous.isValid() ? previous.toInt() : -1;

    QString text;
    QColor color;
    switch (presence) {
    case PRESENCE_AWAY:
        text = tr("Away");
        color = QColor(180, 120, 0);
        break;
    case PRESENCE_STALE:
        text = tr("Stale");
        color = QColor(128, 128, 128);
        break;
    default:
        text = tr("Online");
        break;
    }

    m_model->setData(m_model->index(row, USER_VIEW_STATUS_COLUMN), text);
    m_model->setData(m_model->index(row, USER_VIEW_IP_COLUMN),
                     presence, Qt::UserRole + 1);

    const QVariant fg = color.isValid() ? QVariant(color) : QVariant();
    for (int col = 0; col < m_model->columnCount(); ++col) {
        m_model->setData(m_model->index(row, col), fg, Qt::ForegroundRole);
    }

    if (oldPresence != presence) {
        notePresenceSound(ip(row), oldPresence, presence);
    }
}

void UserManager::notePresenceSound(const QString &ip, int oldPresence,
                                    int newPresence)
{
    if (!m_presenceSoundArmed || ip.isEmpty()) {
        return;
    }
    if (!Global::preferences || !Global::preferences->isPresenceSound) {
        return;
    }
    if (!Global::soundThread) {
        return;
    }

    const bool wasHere = oldPresence == PRESENCE_ONLINE
            || oldPresence == PRESENCE_AWAY;
    const bool nowHere = newPresence == PRESENCE_ONLINE
            || newPresence == PRESENCE_AWAY;
    if (wasHere == nowHere) {
        return;
    }

    // A second NIC of someone who is already on the list is not a new
    // person arriving or leaving. Beep only when the logical peer
    // crosses the online/offline line.
    const QList<PeerFoldGroup> folds = contactFolds();
    const int heardRow = ipToRow(ip);
    for (int g = 0; g < folds.size(); ++g) {
        const PeerFoldGroup &fold = folds.at(g);
        if (heardRow < 0 || !fold.indexes.contains(heardRow)) {
            continue;
        }
        for (int n = 0; n < fold.indexes.size(); ++n) {
            const int row = fold.indexes.at(n);
            if (row == heardRow) {
                continue;
            }
            const int sibling = presence(row);
            if (sibling == PRESENCE_ONLINE || sibling == PRESENCE_AWAY) {
                return;
            }
        }
        break;
    }

    const QDateTime now = QDateTime::currentDateTime();
    if (m_lastPresenceSoundAt.isValid()
            && m_lastPresenceSoundAt.msecsTo(now) < PRESENCE_SOUND_GLOBAL_GAP_MS) {
        return;
    }
    if (m_peerPresenceSoundAt.contains(ip)
            && m_peerPresenceSoundAt.value(ip).msecsTo(now)
            < PRESENCE_SOUND_PEER_GAP_MS) {
        return;
    }

    const QString path = nowHere
            ? Global::preferences->onlineSound
            : Global::preferences->offlineSound;
    if (path.isEmpty()) {
        return;
    }
    Global::soundThread->play(path);
    m_lastPresenceSoundAt = now;
    m_peerPresenceSoundAt.insert(ip, now);
}

void UserManager::applyRefreshTimer()
{
    if (!m_refreshTimer) {
        return;
    }

    const int seconds = qMax(5, Global::preferences->autoRefreshSeconds);
    if (Global::preferences->isAutoRefresh) {
        m_refreshTimer->start(seconds * 1000);
    } else {
        m_refreshTimer->stop();
    }
}

void UserManager::autoRefreshPresence()
{
    announcePresence();
    sweepStaleUsers();
    notifyUsersChanged();
}

void UserManager::sweepStaleUsers()
{
    const int interval = qMax(5, Global::preferences->autoRefreshSeconds);
    const int missed = qMax(1, Global::preferences->staleMissedRefreshCount);
    const qint64 staleMs = qint64(interval) * missed * 1000;
    const QDateTime now = QDateTime::currentDateTime();

    for (int row = m_model->rowCount() - 1; row >= 0; --row) {
        QDateTime lastSeen = m_model->data(
                    m_model->index(row, USER_VIEW_IP_COLUMN),
                    Qt::UserRole).toDateTime();
        if (!lastSeen.isValid()) {
            lastSeen = now;
            m_model->setData(m_model->index(row, USER_VIEW_IP_COLUMN),
                             lastSeen, Qt::UserRole);
        }

        const qint64 age = lastSeen.msecsTo(now);
        if (age >= staleMs) {
            const int presence = m_model->data(
                        m_model->index(row, USER_VIEW_IP_COLUMN),
                        Qt::UserRole + 1).toInt();
            if (presence != PRESENCE_STALE) {
                setRowPresence(row, PRESENCE_STALE);
            }
        }
    }
}

quint32 UserManager::presenceFlags(quint32 command) const
{
    quint32 flags = command | QIPMSG_CAPACITY;
    if (Global::preferences->isAway) {
        flags |= IPMSG_ABSENCEOPT;
    }
    if (Global::preferences->isEncryptMsg) {
        flags |= IPMSG_ENCRYPTOPT;
    }
    if (Global::preferences->isDialup) {
        flags |= IPMSG_DIALUPOPT;
    }
    return flags;
}

quint32 UserManager::commandFlags(quint32 command) const
{
    quint32 flags = presenceFlags(command);
    // Presence bodies stay on the configured codec. UTF8OPT is set only when
    // that codec is UTF-8, so FeiQ 2013 (GBK, no CAPUTF8OPT) still reads nicknames.
    if (Global::transferCodec
            && Global::transferCodec->codecName().contains(
                QLatin1String("UTF-8"), Qt::CaseInsensitive)) {
        flags |= IPMSG_UTF8OPT;
    }
    return flags;
}

quint32 UserManager::peerCapsForIp(const QString &ip) const
{
    const int row = ipToRow(ip);
    if (row < 0) {
        return 0;
    }
    return m_model->data(m_model->index(row, USER_VIEW_IP_COLUMN),
                         Qt::UserRole + 4).toUInt();
}

void UserManager::notePeerCaps(QString ip, quint32 flags)
{
    const int row = ipToRow(ip);
    if (row < 0) {
        return;
    }
    quint32 add = 0;
    if (flags & (IPMSG_CAPUTF8OPT | IPMSG_UTF8OPT)) {
        add |= IPMSG_CAPUTF8OPT;
    }
    if (flags & IPMSG_CLIPBOARDOPT) {
        add |= IPMSG_CLIPBOARDOPT;
    }
    if (flags & IPMSG_FILEATTACHOPT) {
        add |= IPMSG_FILEATTACHOPT;
    }
    if (add == 0) {
        return;
    }
    const quint32 caps = peerCapsForIp(ip);
    if ((caps & add) == add) {
        return;
    }
    m_model->setData(m_model->index(row, USER_VIEW_IP_COLUMN),
                     caps | add, Qt::UserRole + 4);
}

quint32 UserManager::sendFlagsForIp(const QString &ip, quint32 flags) const
{
    const bool peerUtf8 = (peerCapsForIp(ip) & IPMSG_CAPUTF8OPT) != 0;
    const bool localUtf8 = Global::transferCodec
            && Global::transferCodec->codecName().contains(
                QLatin1String("UTF-8"), Qt::CaseInsensitive);
    if (peerUtf8 || localUtf8) {
        flags |= IPMSG_UTF8OPT;
    }
    return flags;
}

void UserManager::broadcastExit() const
{
    qDebug("UserManager::broadcastExit");

    SendMsg sendMsg(QHostAddress::Null, 0/* port */,
                    exitMessage(), ""/* extendedInfo */,
                    commandFlags(IPMSG_BR_EXIT));

    Global::msgThread->addSendMsg(Msg(sendMsg));
    unicastPresenceToDialup(IPMSG_BR_EXIT);
}

QString UserManager::localQipToken() const
{
    QString token = QStringLiteral("qipmsg");
    if (Global::preferences) {
        token = ShareFolder::localToken(Global::preferences->shareEnabled,
                                       Global::preferences->sharePath,
                                       Global::preferences->sharePassword);
    }
    if (Global::avatarStore) {
        const QString id = Global::avatarStore->selfWireId();
        if (AvatarWire::idOk(id)) {
            token += QLatin1String(",av:") + id;
        }
    }
    return token;
}

QString UserManager::peerQipToken(const QString &ip) const
{
    const int row = ipToRow(ip);
    if (row < 0) {
        return QString();
    }
    return m_model->data(m_model->index(row, USER_VIEW_IP_COLUMN),
                         Qt::UserRole + 6).toString();
}

bool UserManager::peerIsQipmsg(const QString &ip) const
{
    return ShareFolder::tokenIsQipmsg(peerQipToken(ip));
}

bool UserManager::peerHasShare(const QString &ip) const
{
    return ShareFolder::tokenHasShare(peerQipToken(ip));
}

bool UserManager::peerIsDialup(const QString &ip) const
{
    const int row = ipToRow(ip);
    if (row < 0) {
        return false;
    }
    return m_model->data(m_model->index(row, USER_VIEW_IP_COLUMN),
                         Qt::UserRole + 5).toBool();
}

void UserManager::applyDialupFlag(int row, bool dialup)
{
    if (row < 0 || !m_model) {
        return;
    }
    m_model->setData(m_model->index(row, USER_VIEW_IP_COLUMN),
                     dialup, Qt::UserRole + 5);
}

void UserManager::unicastPresenceToDialup(quint32 command) const
{
    if (!Global::msgThread) {
        return;
    }
    // protocol.txt: a host with IPMSG_DIALUPOPT may miss subnet broadcasts,
    // so member-recognition commands are sent to that host individually.
    // This follows our own announce. It is not a reply to their entry, so
    // two dial-up qipmsg peers do not bounce BR_ENTRY forever. ANSENTRY
    // still answers their BR_ENTRY on the existing path.
    const bool exitPacket = GET_MODE(command) == IPMSG_BR_EXIT;
    // Every address, not the preferred one. A dial-up peer that also
    // has two NICs needs the unicast on both, because either one may
    // be the path that misses the subnet broadcast. Self rows are
    // already gone, so this does not unicast to us.
    for (int row = 0; row < userCount(); ++row) {
        const QString peerIp = ip(row);
        if (peerIp.isEmpty() || !peerIsDialup(peerIp)) {
            continue;
        }
        if (!exitPacket && Global::groupPolicy
                && Global::groupPolicy->isHidden(group(row))) {
            continue;
        }
        sendDirectedUserPacket(peerIp, 0, command);
    }
}

void UserManager::refreshLocalView()
{
    notifyUsersChanged();
}

QString UserManager::entryMessage() const
{
    return QString("%1%2%3%4%5%6%7%8")
        .arg(m_ourself.name())
        .arg(QChar('\0'))
        .arg(m_ourself.group())
        .arg(QChar('\0'))
        .arg(Global::preferences->personSign)
        .arg(QChar('\0'))
        .arg(localQipToken())
        .arg(QChar('\0'));
}

QString UserManager::exitMessage() const
{
    return entryMessage();
}

void UserManager::broadcastEntry() const
{
    qDebug("UserManager::broadcastEntry");

    SendMsg sendMsg(QHostAddress::Null, 0/* port */,
                    entryMessage(), ""/* extendedInfo */,
                    commandFlags(IPMSG_BR_ENTRY));

    Global::msgThread->addSendMsg(Msg(sendMsg));
    unicastPresenceToDialup(IPMSG_BR_ENTRY);
}

void UserManager::broadcastAbsence() const
{
    qDebug("UserManager::broadcastAbsence");

    SendMsg sendMsg(QHostAddress::Null, 0/* port */,
                    entryMessage(), ""/* extendedInfo */,
                    commandFlags(IPMSG_BR_ABSENCE) | IPMSG_ABSENCEOPT);

    Global::msgThread->addSendMsg(Msg(sendMsg));
    unicastPresenceToDialup(IPMSG_BR_ABSENCE);
}

void UserManager::sendDirectedUserPacket(const QString &ip, quint16 port,
                                         quint32 command) const
{
    if (ip.isEmpty() || !Global::msgThread) {
        return;
    }
    quint16 dest = port;
    if (dest == 0) {
        dest = sendPortForIp(ip);
    }
    quint32 flags = commandFlags(command);
    if (command == IPMSG_BR_ABSENCE) {
        flags |= IPMSG_ABSENCEOPT;
    }
    const QString body = (command == IPMSG_BR_EXIT) ? exitMessage() : entryMessage();
    SendMsg sendMsg(QHostAddress(ip), dest, body, QString(), flags);
    Global::msgThread->addSendMsg(Msg(sendMsg));
}

void UserManager::announcePresence() const
{
    if (Global::preferences->isStealth) {
        return;
    }
    if (!Global::groupPolicy || !Global::groupPolicy->hasHidden()) {
        if (Global::preferences->isAway) {
            broadcastAbsence();
        } else {
            broadcastEntry();
        }
        return;
    }

    // Subnet broadcast would also reach hidden groups. Unicast leave to
    // those peers and unicast entry/absence to everyone else we already know.
    const quint32 visibleCommand = Global::preferences->isAway
            ? IPMSG_BR_ABSENCE : IPMSG_BR_ENTRY;
    for (int row = 0; row < userCount(); ++row) {
        const QString peerIp = ip(row);
        if (peerIp.isEmpty()) {
            continue;
        }
        if (Global::groupPolicy->isHidden(group(row))) {
            sendDirectedUserPacket(peerIp, 0, IPMSG_BR_EXIT);
        } else {
            sendDirectedUserPacket(peerIp, 0, visibleCommand);
        }
    }
}

void UserManager::refreshGroupVisibility(const QString &groupName) const
{
    const QString key = GroupPolicy::normalizeName(groupName);
    if (key.isEmpty() || !Global::preferences || Global::preferences->isStealth
            || !Global::groupPolicy) {
        return;
    }
    const bool hidden = Global::groupPolicy->isHidden(key);
    for (int row = 0; row < userCount(); ++row) {
        if (GroupPolicy::normalizeName(group(row)) != key) {
            continue;
        }
        const QString peerIp = ip(row);
        if (peerIp.isEmpty()) {
            continue;
        }
        if (hidden) {
            sendDirectedUserPacket(peerIp, 0, IPMSG_BR_EXIT);
        } else if (Global::preferences->isAway) {
            sendDirectedUserPacket(peerIp, 0, IPMSG_BR_ABSENCE);
        } else {
            sendDirectedUserPacket(peerIp, 0, IPMSG_BR_ENTRY);
        }
    }
    if (!hidden && !Global::groupPolicy->hasHidden()) {
        announcePresence();
    }
}

void UserManager::reconcileGroupVisibility(const QString &ip,
                                           const QString &previousGroup,
                                           const QString &currentGroup) const
{
    if (ip.isEmpty() || !Global::preferences || Global::preferences->isStealth
            || !Global::groupPolicy) {
        return;
    }
    const QString before = GroupPolicy::normalizeName(previousGroup);
    const QString now = GroupPolicy::normalizeName(currentGroup);
    if (before == now) {
        return;
    }
    const bool wasHidden = Global::groupPolicy->isHidden(before);
    const bool nowHidden = Global::groupPolicy->isHidden(now);
    if (wasHidden == nowHidden) {
        return;
    }
    if (nowHidden) {
        sendDirectedUserPacket(ip, 0, IPMSG_BR_EXIT);
    } else if (Global::preferences->isAway) {
        sendDirectedUserPacket(ip, 0, IPMSG_BR_ABSENCE);
    } else {
        sendDirectedUserPacket(ip, 0, IPMSG_BR_ENTRY);
    }
}

void UserManager::applyStealth(bool stealth)
{
    if (Global::preferences->isStealth == stealth) {
        return;
    }
    Global::preferences->isStealth = stealth;
    if (stealth) {
        // FeiQ HideMode: leave the list first so peers who already saw us
        // mark us offline, then stay silent on later BR_ENTRY.
        broadcastExit();
    } else {
        announcePresence();
    }
    notifyUsersChanged();
}

bool UserManager::contains(QString ip) const
{
    return ipToRow(ip) != -1;
}

QString UserManager::ip(int row) const
{
    return m_model->data(m_model->index(row, USER_VIEW_IP_COLUMN))
        .toString();
}

int UserManager::ipToRow(QString ip) const
{
    for (int row = 0; row < m_model->rowCount(); ++row) {
        if (this->ip(row) == ip) {
            return row;
        }
    }

    return -1;
}

QString UserManager::name(int row) const
{
    return m_model->data(m_model->index(row, USER_VIEW_NAME_COLUMN))
        .toString();
}

QString UserManager::group(int row) const
{
    return m_model->data(m_model->index(row, USER_VIEW_GROUP_COLUMN))
        .toString();
}

QString UserManager::host(int row) const
{
    return m_model->data(m_model->index(row, USER_VIEW_HOST_COLUMN))
        .toString();
}

QString UserManager::loginName(int row) const
{
    return m_model->data(m_model->index(row, USER_VIEW_LOGIN_NAME_COLUMN))
        .toString();
}

QString UserManager::status(int row) const
{
    return m_model->data(m_model->index(row, USER_VIEW_STATUS_COLUMN))
        .toString();
}

int UserManager::presence(int row) const
{
    return m_model->data(m_model->index(row, USER_VIEW_IP_COLUMN),
                         Qt::UserRole + 1).toInt();
}

int UserManager::presenceForIp(const QString &ip) const
{
    const int row = ipToRow(ip);
    return row == -1 ? PRESENCE_STALE : presence(row);
}

bool UserManager::isReachable(const QString &ip) const
{
    const int p = presenceForIp(ip);
    return p == PRESENCE_ONLINE || p == PRESENCE_AWAY;
}

QStringList UserManager::departmentNames() const
{
    QStringList names;
    for (int row = 0; row < userCount(); ++row) {
        const QString name = GroupPolicy::normalizeName(group(row));
        if (!name.isEmpty() && !names.contains(name)) {
            names << name;
        }
    }
    names.sort(Qt::CaseInsensitive);
    return names;
}

QStringList UserManager::reachableIpsInGroup(const QString &groupName) const
{
    QStringList ips;
    const QString key = GroupPolicy::normalizeName(groupName);
    if (key.isEmpty()) {
        return ips;
    }
    // One send per person. Walking every NIC would deliver the same
    // text to FeiQ on 192.168.14.164 and again on 10.147.19.236.
    // The peer stays in the department when any of its addresses
    // announced that group. The address we return is the preferred one,
    // which is fresh when any address is fresh.
    const QList<PeerFoldGroup> folds = contactFolds();
    for (int g = 0; g < folds.size(); ++g) {
        const PeerFoldGroup &fold = folds.at(g);
        bool inGroup = false;
        for (int n = 0; n < fold.indexes.size(); ++n) {
            if (GroupPolicy::normalizeName(group(fold.indexes.at(n))) == key) {
                inGroup = true;
                break;
            }
        }
        if (!inGroup || fold.preferred < 0) {
            continue;
        }
        const QString peerIp = ip(fold.preferred);
        if (peerIp.isEmpty() || !isReachable(peerIp)) {
            continue;
        }
        if (!ips.contains(peerIp)) {
            ips << peerIp;
        }
    }
    return ips;
}

int UserManager::onlineCount() const
{
    int count = 0;
    const QList<PeerFoldGroup> folds = contactFolds();
    for (int g = 0; g < folds.size(); ++g) {
        const PeerFoldGroup &fold = folds.at(g);
        for (int n = 0; n < fold.indexes.size(); ++n) {
            const int p = presence(fold.indexes.at(n));
            if (p == PRESENCE_ONLINE || p == PRESENCE_AWAY) {
                ++count;
                break;
            }
        }
    }
    return count;
}

int UserManager::listedPeerCount() const
{
    return contactFolds().size();
}

QStringList UserManager::localIpv4List() const
{
    QStringList ips;
    const QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    for (int i = 0; i < addresses.size(); ++i) {
        if (addresses.at(i).protocol() != QAbstractSocket::IPv4Protocol) {
            continue;
        }
        const QString text = addresses.at(i).toString();
        if (!text.isEmpty() && !ips.contains(text)) {
            ips << text;
        }
    }
    return ips;
}

bool UserManager::isSelfPeer(const QString &login, const QString &host,
                             const QString &ip) const
{
    return peerIsSelf(login, host, ip,
                      m_ourself.loginName(), m_ourself.host(),
                      localIpv4List());
}

void UserManager::pruneSelfRows()
{
    if (!m_model) {
        return;
    }
    bool removed = false;
    for (int row = m_model->rowCount() - 1; row >= 0; --row) {
        if (!isSelfPeer(loginName(row), host(row), ip(row))) {
            continue;
        }
        m_model->removeRow(row);
        removed = true;
    }
    if (!removed) {
        return;
    }
    scheduleSavePeers();
    emit userCountUpdated(listedPeerCount());
    notifyUsersChanged();
}

QList<PeerFoldGroup> UserManager::contactFolds() const
{
    QList<PeerFoldInput> inputs;
    const int rows = m_model ? m_model->rowCount() : 0;
    for (int row = 0; row < rows; ++row) {
        PeerFoldInput input;
        input.ip = ip(row);
        input.login = loginName(row);
        input.host = host(row);
        input.qipToken = peerQipToken(input.ip);
        input.port = peerPort(input.ip);
        const int p = presence(row);
        input.fresh = (p == PRESENCE_ONLINE || p == PRESENCE_AWAY);
        input.seen = m_model->data(m_model->index(row, USER_VIEW_IP_COLUMN),
                                   Qt::UserRole).toDateTime();
        inputs.append(input);
    }
    // Input order is model order, so PeerFoldGroup::indexes are rows.
    return peerFoldGroups(inputs);
}

int UserManager::preferredRowForIp(const QString &ip) const
{
    const int row = ipToRow(ip);
    if (row < 0) {
        return -1;
    }
    const QList<PeerFoldGroup> folds = contactFolds();
    for (int g = 0; g < folds.size(); ++g) {
        if (folds.at(g).indexes.contains(row)) {
            return folds.at(g).preferred;
        }
    }
    return row;
}

QStringList UserManager::logicalSendIps(bool reachableOnly) const
{
    QStringList ips;
    const QList<PeerFoldGroup> folds = contactFolds();
    for (int g = 0; g < folds.size(); ++g) {
        const int row = folds.at(g).preferred;
        if (row < 0) {
            continue;
        }
        const QString peerIp = ip(row);
        if (peerIp.isEmpty()) {
            continue;
        }
        if (reachableOnly && !isReachable(peerIp)) {
            continue;
        }
        if (!ips.contains(peerIp)) {
            ips << peerIp;
        }
    }
    return ips;
}

void UserManager::notifyUsersChanged()
{
    emit usersChanged();
}

int UserManager::userCount() const
{
    return m_model->rowCount();
}

QString UserManager::memoForIp(const QString &ip) const
{
    return m_memos.value(ip);
}

void UserManager::setMemoForIp(const QString &ip, const QString &memo)
{
    if (ip.isEmpty()) {
        return;
    }
    const QString trimmed = memo.trimmed();
    if (trimmed.isEmpty()) {
        m_memos.remove(ip);
    } else {
        m_memos.insert(ip, trimmed);
    }
    scheduleSavePeers();
    notifyUsersChanged();
}

QString UserManager::realNameForIp(const QString &ip) const
{
    const int row = ipToRow(ip);
    if (row != -1) {
        const QString n = name(row);
        if (!n.isEmpty()) {
            return n;
        }
    }
    return ip;
}

QString Global::peerLabelForIp(const QString &ip)
{
    if (!userManager || ip.isEmpty()) {
        return ip;
    }
    const QString name = userManager->displayNameForIp(ip);
    return name.isEmpty() ? ip : name;
}

QString UserManager::displayNameForIp(const QString &ip) const
{
    const QString memo = memoForIp(ip);
    if (!memo.isEmpty()) {
        return memo;
    }
    return realNameForIp(ip);
}

QString UserManager::peerCaption(const QString &ip) const
{
    const int row = ipToRow(ip);
    if (row == -1) {
        return ip;
    }

    const QString groupName = group(row);
    const QString hostName = host(row);
    const QString extra = groupName.isEmpty()
            ? hostName
            : groupName + "/" + hostName;
    const QString real = name(row);
    const QString memo = memoForIp(ip);
    QString caption;
    if (!memo.isEmpty() && memo != real) {
        caption = QString("%1 [%2] (%3) %4").arg(memo, real, extra, ip);
    } else {
        caption = QString("%1 (%2) %3").arg(real, extra, ip);
    }
    const QString sign = signature(row);
    if (!sign.isEmpty()) {
        caption += "\n" + sign;
    }
    if (Global::avatarStore) {
        const QString photo = Global::avatarStore->status(ip);
        if (!photo.isEmpty()) {
            caption += "\n" + photo;
        }
    }
    if (peerIsDialup(ip)) {
        caption += QLatin1Char('\n');
        caption += tr("Dial-up host");
    }

    // Tell the open chat which NIC this window is bound to, and which
    // address the contact list would use instead. The window itself
    // stays on the IP the message arrived from, so a reply can still
    // follow the path that just worked.
    const int preferredRow = preferredRowForIp(ip);
    if (preferredRow >= 0) {
        const QString preferredIp = this->ip(preferredRow);
        const QList<PeerFoldGroup> folds = contactFolds();
        QStringList others;
        for (int g = 0; g < folds.size(); ++g) {
            if (!folds.at(g).indexes.contains(preferredRow)) {
                continue;
            }
            for (int n = 0; n < folds.at(g).indexes.size(); ++n) {
                const int row = folds.at(g).indexes.at(n);
                const QString other = this->ip(row);
                if (other.isEmpty() || other == ip) {
                    continue;
                }
                others << other;
            }
            break;
        }
        if (!others.isEmpty()) {
            caption += QLatin1Char('\n');
            caption += tr("Other addresses for this peer: %1")
                    .arg(others.join(QStringLiteral(", ")));
        }
        if (!preferredIp.isEmpty() && preferredIp != ip) {
            caption += QLatin1Char('\n');
            caption += tr("This chat is on %1. The contact list prefers %2.")
                    .arg(ip, preferredIp);
        } else if (!others.isEmpty()) {
            caption += QLatin1Char('\n');
            caption += tr("Messages and files from the contact list use %1. "
                          "A fresh 192.168 or 172.16 address is chosen over "
                          "a fresh 10.x address.")
                    .arg(ip);
        }
    }
    return caption;
}

QString UserManager::signature(int row) const
{
    return m_model->data(m_model->index(row, USER_VIEW_IP_COLUMN),
                         Qt::UserRole + 2).toString();
}

QString UserManager::signatureForIp(const QString &ip) const
{
    const int row = ipToRow(ip);
    return row == -1 ? QString() : signature(row);
}

quint16 UserManager::peerPort(const QString &ip) const
{
    const int row = ipToRow(ip);
    if (row == -1) {
        return 0;
    }
    return static_cast<quint16>(m_model->data(
                m_model->index(row, USER_VIEW_IP_COLUMN),
                Qt::UserRole + 3).toUInt());
}

quint16 UserManager::sendPortForIp(const QString &ip) const
{
    const quint16 port = peerPort(ip);
    if (port != 0) {
        return port;
    }
    return IPMSG_DEFAULT_PORT;
}

QString UserManager::peersFilePath() const
{
    return Helper::appHomePath() + "/peers.json";
}

void UserManager::scheduleSavePeers()
{
    if (m_saveTimer) {
        m_saveTimer->start(800);
    }
}

void UserManager::suspendPeerPersist()
{
    m_suspendPeerPersist = true;
    if (m_saveTimer) {
        m_saveTimer->stop();
    }
}

void UserManager::adoptRestoredPeers()
{
    if (m_saveTimer) {
        m_saveTimer->stop();
    }
    m_memos.clear();
    loadPersistedPeers();
    // A restored file can still name this machine. Drop those rows
    // after the load, including any that were already on screen.
    pruneSelfRows();
    notifyUsersChanged();
    emit userCountUpdated(listedPeerCount());
}

bool UserManager::loadPersistedPeers()
{
    QFile file(peersFilePath());
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) {
        return false;
    }
    bool droppedSelf = false;

    const QJsonArray array = doc.array();
    for (int i = 0; i < array.size(); ++i) {
        const QJsonObject obj = array.at(i).toObject();
        const QString ip = obj.value("ip").toString();
        if (ip.isEmpty()) {
            continue;
        }
        const QString loadedLogin = obj.value("login").toString();
        const QString loadedHost = obj.value("host").toString();
        // Stale self rows (old WLAN, old LAN, 127.0.0.1) are not
        // reloaded. The next save drops them from peers.json. A FeiQ
        // row with a different host is kept, even when the login
        // matches ours ignoring case.
        if (isSelfPeer(loadedLogin, loadedHost, ip)) {
            droppedSelf = true;
            continue;
        }
        const QString memo = obj.value("memo").toString().trimmed();
        if (!memo.isEmpty()) {
            m_memos.insert(ip, memo);
        }
        if (contains(ip)) {
            continue;
        }
        Owner owner;
        owner.setIp(ip);
        owner.setName(obj.value("name").toString());
        owner.setGroup(obj.value("group").toString());
        owner.setHost(loadedHost);
        owner.setLoginName(loadedLogin);
        owner.setSignature(obj.value("sign").toString());
        owner.setPort(static_cast<quint16>(obj.value("port").toInt()));
        const int row = m_model->rowCount();
        m_model->insertRow(row);
        addUser(owner, row);
        const QDateTime lastSeen = QDateTime::fromString(
                    obj.value("lastSeen").toString(), Qt::ISODate);
        m_model->setData(m_model->index(row, USER_VIEW_IP_COLUMN),
                         lastSeen.isValid() ? lastSeen
                                            : QDateTime::currentDateTime(),
                         Qt::UserRole);
        setRowPresence(row, PRESENCE_STALE);
        applyDialupFlag(row, obj.value(QStringLiteral("dialup")).toBool());
    }

    if (m_model->rowCount() > 0) {
        emit userCountUpdated(listedPeerCount());
        notifyUsersChanged();
    }
    // During restore the timer already exists, so the rewrite can be
    // armed here. The constructor arms it after the timer is created.
    if (droppedSelf && m_saveTimer) {
        scheduleSavePeers();
    }
    return droppedSelf;
}

void UserManager::savePersistedPeers()
{
    if (m_suspendPeerPersist) {
        return;
    }
    QJsonArray array;
    QSet<QString> written;
    for (int row = 0; row < m_model->rowCount(); ++row) {
        QJsonObject obj;
        const QString peerIp = ip(row);
        obj.insert("ip", peerIp);
        obj.insert("name", name(row));
        obj.insert("group", group(row));
        obj.insert("host", host(row));
        obj.insert("login", loginName(row));
        obj.insert("sign", signature(row));
        obj.insert("port", peerPort(peerIp));
        const QString memo = memoForIp(peerIp);
        if (!memo.isEmpty()) {
            obj.insert("memo", memo);
        }
        if (peerIsDialup(peerIp)) {
            obj.insert(QStringLiteral("dialup"), true);
        }
        obj.insert("lastSeen",
                   m_model->data(m_model->index(row, USER_VIEW_IP_COLUMN),
                                 Qt::UserRole).toDateTime().toString(Qt::ISODate));
        array.append(obj);
        written.insert(peerIp);
    }
    for (auto it = m_memos.constBegin(); it != m_memos.constEnd(); ++it) {
        if (written.contains(it.key()) || it.value().isEmpty()) {
            continue;
        }
        QJsonObject obj;
        obj.insert("ip", it.key());
        obj.insert("memo", it.value());
        array.append(obj);
    }

    QFile file(peersFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return;
    }
    file.write(QJsonDocument(array).toJson(QJsonDocument::Compact));
}
