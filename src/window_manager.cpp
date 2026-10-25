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

#include "window_manager.h"
#include "chat_window.h"
#include "constants.h"
#include "discussion_window.h"
#include "global.h"
#include "helper.h"
#include "main_window.h"
#include "message_store.h"
#include "msg_readed_window.h"
#include "msg_thread.h"
#include "msg_window.h"
#include "preferences.h"
#include "send_msg.h"
#include "sound_thread.h"
#include "systray.h"
#include "user_manager.h"
#include "offline_queue.h"
#include "protocolutil.h"
#include "groupPolicy.h"
#include "msgCrypto.h"
#include "richText.h"
#include "localMsg.h"

#include <QHostAddress>
#include <QMessageBox>
#include <QPoint>
#include <QSet>
#include <QSize>
#include <QTimer>

WindowManager::WindowManager(QObject *parent)
    : QObject(parent), m_typingSweep(0), m_queueRetry(0), m_flushing(false)
{
    connect(Global::userManager, SIGNAL(usersChanged()),
            this, SLOT(flushOfflineQueue()));
    connect(Global::userManager, SIGNAL(usersChanged()),
            this, SLOT(refreshChatCaptions()));
    m_typingSweep = new QTimer(this);
    m_typingSweep->setInterval(1000);
    connect(m_typingSweep, SIGNAL(timeout()), this, SLOT(sweepTyping()));
    m_typingSweep->start();
    m_queueRetry = new QTimer(this);
    m_queueRetry->setInterval(5000);
    connect(m_queueRetry, SIGNAL(timeout()), this, SLOT(flushOfflineQueue()));
    m_queueRetry->start();
    QTimer::singleShot(0, this, SLOT(flushOfflineQueue()));
}

WindowManager::~WindowManager()
{
    destroyMsgWindowList();
    destroyMsgReadedWindowList();
    destroyChatWindows();
    destroyDiscussionWindows();
}

void WindowManager::notePeerTyping(QString ip, bool typing)
{
    if (ip.isEmpty()) {
        return;
    }
    if (typing) {
        m_typingUntil.insert(ip, QDateTime::currentDateTime().addMSecs(5000));
    } else {
        m_typingUntil.remove(ip);
    }
    ChatWindow *chat = m_chatWindows.value(ip, 0);
    if (chat) {
        chat->setPeerTyping(typing);
    }
    emit peerTypingChanged(ip, typing);
}

void WindowManager::sweepTyping()
{
    if (m_typingUntil.isEmpty()) {
        return;
    }
    const QDateTime now = QDateTime::currentDateTime();
    const QList<QString> keys = m_typingUntil.keys();
    foreach (const QString &ip, keys) {
        if (m_typingUntil.value(ip) <= now) {
            m_typingUntil.remove(ip);
            ChatWindow *chat = m_chatWindows.value(ip, 0);
            if (chat) {
                chat->setPeerTyping(false);
            }
            emit peerTypingChanged(ip, false);
        }
    }
}

void WindowManager::newMsg(Msg msg)
{
    switch (GET_MODE(msg->flags())) {
    case IPMSG_SENDMSG:
        handleIncomingSendMsg(msg);
        break;
    case IPMSG_READMSG:
        handleReadMsg(msg);
        break;

    default:
        break;
    }
}

QString WindowManager::peerDisplayName(const Msg &msg) const
{
    const QString fromList = Global::userManager->displayNameForIp(msg->ip());
    if (!fromList.isEmpty() && fromList != msg->ip()) {
        return fromList;
    }
    const QString ownerName = msg->owner().name();
    return ownerName.isEmpty() ? msg->ip() : ownerName;
}

bool WindowManager::isPeerGroupMuted(const QString &ip) const
{
    if (ip.isEmpty() || !Global::groupPolicy || !Global::userManager) {
        return false;
    }
    const int row = Global::userManager->ipToRow(ip);
    if (row < 0) {
        return false;
    }
    return Global::groupPolicy->isMuted(Global::userManager->group(row));
}

bool WindowManager::shouldPopup() const
{
    // Do-not-disturb is independent of BR_ABSENCE / auto-reply. It only
    // stops auto-raising windows. History and the unread counter still run.
    if (Global::preferences->isDoNotDisturb) {
        return false;
    }
    if (Global::preferences->isNoAutoPopupMsg) {
        return false;
    }
    if (Global::preferences->isAway && Global::preferences->isNoPopupWhenAway) {
        return false;
    }
    return true;
}

void WindowManager::playNoticeSound()
{
    if (!Global::preferences->isNoSoundAlarm) {
        Global::soundThread->play(Global::preferences->noticeSound);
    }
}

QString WindowManager::sealedStorageKey(const QString &ip,
                                         const QString &packetNo) const
{
    return ip + QLatin1Char('\n') + packetNo;
}

bool WindowManager::openSealedLetter(const QString &ip, const QString &packetNo)
{
    const QString key = sealedStorageKey(ip, packetNo);
    if (!m_sealedLetters.contains(key)) {
        return false;
    }
    SealedLetter letter = m_sealedLetters.value(key);
    if (!letter.opened) {
        letter.opened = true;
        m_sealedLetters.insert(key, letter);

        const int flags = IPMSG_READMSG | IPMSG_READCHECKOPT;
        SendMsg sendMsg(letter.address, letter.port,
                        letter.packetNo, ""/* extendedInfo */, flags);
        Global::msgThread->addSendMsg(Msg(sendMsg));

        const QString peerName = Global::userManager->displayNameForIp(letter.ip);
        if (Helper::isShakeText(letter.text)) {
            recordIncomingText(letter.ip, peerName, tr("[Window shake]"), false);
            applyIncomingShake(letter.ip);
        } else if (!letter.text.isEmpty()) {
            QString shown = letter.text;
            if (letter.broadcast) {
                shown = tr("[Broadcast] %1").arg(letter.text);
            }
            recordIncomingText(letter.ip, peerName, shown, false, QStringList(),
                               false, letter.packetNo);
        }
    }

    ChatWindow *w = m_chatWindows.value(letter.ip, 0);
    if (w) {
        w->markSealedOpened(letter.packetNo);
    }
    return true;
}

void WindowManager::handleIncomingSendMsg(Msg msg)
{
    const bool sealed = ProtocolUtil::isSecretEnvelope(msg->flags());
    const bool attach = msg->flags() & IPMSG_FILEATTACHOPT;
    const QString text = msg->additionalInfo();
    const QString name = peerDisplayName(msg);
    const bool autoReply = msg->flags() & IPMSG_AUTORETOPT;
    const bool encrypted = msg->transportEncrypted();
    const bool isShake = !sealed && !attach && Helper::isShakeText(text);
    const bool muted = isPeerGroupMuted(msg->ip());
    const bool notify = !autoReply && !muted;

    if (isShake) {
        recordIncomingText(msg->ip(), name, tr("[Window shake]"), notify,
                           QStringList(), encrypted);
        if (notify) {
            applyIncomingShake(msg->ip());
            playNoticeSound();
        }
        return;
    }

    if (!sealed) {
        QString shown = text;
        if ((msg->flags() & IPMSG_BROADCASTOPT) && !shown.isEmpty()) {
            shown = tr("[Broadcast] %1").arg(shown);
        }
        if (!shown.isEmpty()) {
            recordIncomingText(msg->ip(), name, shown, notify, QStringList(),
                               encrypted, msg->packetNoString());
        } else if (attach) {
            recordIncomingText(msg->ip(), name,
                               tr("[File attachment]"), notify, QStringList(),
                               encrypted);
        }
    } else {
        SealedLetter letter;
        letter.packetNo = msg->packetNoString();
        letter.ip = msg->ip();
        letter.text = text;
        letter.address = msg->ipAddress();
        letter.port = msg->port();
        letter.opened = false;
        letter.broadcast = (msg->flags() & IPMSG_BROADCASTOPT) != 0;
        m_sealedLetters.insert(sealedStorageKey(letter.ip, letter.packetNo), letter);

        ChatWindow *chat = ensureChat(msg->ip());
        chat->appendSealedLetter(letter.packetNo, name);
        if (!autoReply) {
            if (!muted) {
                emit newTextMessage(name, tr("(Sealed message)"));
                if (shouldPopup() && !attach) {
                    openChat(msg->ip(), true);
                } else if (!chat->isVisible()) {
                    addUnread(msg->ip());
                }
            } else if (!chat->isVisible()) {
                addUnread(msg->ip());
            }
        }
    }

    if (!autoReply) {
        maybeAutoReply(msg);
        if (!muted) {
            playNoticeSound();
        }
    }

    if (sealed) {
        if (attach) {
            createMsgWindow(msg);
        }
    } else if (attach) {
        createMsgWindow(msg);
    } else if (shouldPopup() && !muted) {
        openChat(msg->ip(), true);
    }
}

void WindowManager::applyIncomingShake(const QString &ip)
{
    ChatWindow *w = ensureChat(ip);
    if (shouldPopup()) {
        openChat(ip, true);
        w->shakeWindow();
        return;
    }
    if (w->isVisible()) {
        w->shakeWindow();
        return;
    }
    if (Global::preferences->isDoNotDisturb) {
        return;
    }
    if (Global::systray) {
        foreach (MainWindow *mw, Global::systray->mainWindowList) {
            if (mw && mw->isVisible()) {
                Helper::shakeWidget(mw);
                return;
            }
        }
    }
}

void WindowManager::noteInstantSkip(QString ip, QString fileName, bool incoming)
{
    if (ip.isEmpty() || fileName.isEmpty() || !Global::userManager) {
        return;
    }
    const QString peerName = Global::userManager->displayNameForIp(ip);
    if (incoming) {
        recordIncomingText(ip, peerName,
                           tr("[Instant] %1 already present, skipped transfer")
                           .arg(fileName),
                           false);
        return;
    }
    recordOutgoing(ip, peerName,
                   tr("[Instant] %1 already on the peer, skipped transfer")
                   .arg(fileName));
}

void WindowManager::recordIncomingText(const QString &ip,
                                       const QString &peerName,
                                       const QString &text,
                                       bool notifyUser,
                                       const QStringList &imagePaths,
                                       bool encrypted,
                                       const QString &packetNo,
                                       const QString &richHtml)
{
    QString html = richHtml;
    if (html.isEmpty() && !packetNo.isEmpty()) {
        const QString key = sealedStorageKey(ip, packetNo);
        const QString pending = m_pendingRich.value(key);
        if (!pending.isEmpty() && RichText::coversPlain(pending, text)) {
            html = pending;
            m_pendingRich.remove(key);
        }
    }

    ChatRecord record;
    record.timestamp = QDateTime::currentDateTime();
    record.peerIp = ip;
    record.peerName = peerName;
    record.direction = "in";
    record.text = text;
    record.richHtml = html;
    record.imagePaths = imagePaths;
    record.packetNo = packetNo;
    record.encrypted = encrypted;
    Global::messageStore->append(record);

    ChatWindow *w = m_chatWindows.value(ip, 0);
    if (w) {
        w->appendMessage(record.timestamp, peerName, text, true,
                         record.imagePaths, packetNo, false, false,
                         encrypted, html);
        if (!w->isVisible()) {
            addUnread(ip);
        }
    } else {
        addUnread(ip);
    }

    if (notifyUser) {
        QString preview = text;
        preview.replace('\n', ' ');
        if (preview.size() > 80) {
            preview = preview.left(80) + "...";
        }
        emit newTextMessage(peerName, preview);
    }
}

int WindowManager::broadcastTextToOnline(const QString &text)
{
    if (text.trimmed().isEmpty() || !Global::userManager || !Global::msgThread) {
        return 0;
    }

    // Verified before this action: chat, discussion, main-window multi-send,
    // offline flush, and auto-reply never set IPMSG_BROADCASTOPT. Multi-send
    // uses IPMSG_MULTICASTOPT and still unicasts. This action unicasts
    // SENDMSG|SENDCHECKOPT|BROADCASTOPT to each reachable peer (online or
    // away). Stale peers are skipped and not queued. The subnet broadcast
    // address is not used here, so a peer who is already online does not
    // get a second copy from 255.255.255.255.
    int sent = 0;
    const int rows = Global::userManager->userCount();
    for (int row = 0; row < rows; ++row) {
        const QString ip = Global::userManager->ip(row);
        if (!Global::userManager->isReachable(ip)) {
            continue;
        }
        quint32 flags = Global::userManager->sendFlagsForIp(
                    ip, IPMSG_SENDMSG | IPMSG_SENDCHECKOPT | IPMSG_BROADCASTOPT);
        QString additionalInfo = text;
        additionalInfo.append(QChar('\0'));
        const bool encrypted = MsgCrypto::prepareOutgoing(ip, &additionalInfo, &flags);
        SendMsg sendMsg(QHostAddress(ip),
                        Global::userManager->sendPortForIp(ip),
                        additionalInfo, ""/* extendedInfo */, flags);
        Global::msgThread->addSendMsg(Msg(sendMsg));
        recordOutgoing(ip, Global::userManager->displayNameForIp(ip),
                       tr("[Broadcast] %1").arg(text), QStringList(),
                       sendMsg.packetNoString(), false, encrypted);
        ++sent;
    }
    return sent;
}

bool WindowManager::deliverText(const QString &ip, const QString &text)
{
    if (!Global::msgThread || !Global::userManager) {
        return false;
    }
    QString additionalInfo = text;
    additionalInfo.append(QChar('\0'));
    quint32 flags = Global::userManager->sendFlagsForIp(
                ip, IPMSG_SENDMSG | IPMSG_SENDCHECKOPT);
    const bool encrypted = MsgCrypto::prepareOutgoing(ip, &additionalInfo, &flags);
    SendMsg sendMsg(QHostAddress(ip),
                    Global::userManager->sendPortForIp(ip),
                    additionalInfo, ""/* extendedInfo */, flags);
    Global::msgThread->addSendMsg(Msg(sendMsg));
    recordOutgoing(ip, Global::userManager->displayNameForIp(ip), text,
                   QStringList(), sendMsg.packetNoString(), false, encrypted);
    return true;
}

QString WindowManager::deliverLocalCommand(const QString &target, const QString &text)
{
    QList<LocalPeerRef> peers;
    if (Global::userManager) {
        const int rows = Global::userManager->userCount();
        for (int row = 0; row < rows; ++row) {
            LocalPeerRef peer;
            peer.ip = Global::userManager->ip(row);
            peer.login = Global::userManager->loginName(row);
            peer.host = Global::userManager->host(row);
            peer.reachable = Global::userManager->isReachable(peer.ip);
            peers.append(peer);
        }
    }
    const bool queueOn = Global::preferences && Global::preferences->isOfflineQueue;
    const LocalPlan plan = LocalMsg::planDelivery(target, text, peers, queueOn);
    if (plan.kind == localPlanSend) {
        if (!deliverText(plan.ip, text)) {
            return QStringLiteral("ERR send-failed");
        }
        return QStringLiteral("OK sent ") + plan.ip;
    }
    if (plan.kind == localPlanQueue) {
        if (!Global::offlineQueue) {
            return QStringLiteral("ERR send-failed");
        }
        QString login;
        QString host;
        peerIdentity(plan.ip, &login, &host);
        const QString id = Global::offlineQueue->enqueue(
                    plan.ip, text, QStringLiteral("text"), login, host);
        if (id.isEmpty()) {
            return QStringLiteral("ERR send-failed");
        }
        return QStringLiteral("OK queued ") + plan.ip;
    }
    return LocalMsg::planError(plan);
}

bool WindowManager::deliverShake(const QString &ip)
{
    if (!Global::msgThread || !Global::userManager) {
        return false;
    }
    quint32 flags = Global::userManager->sendFlagsForIp(
                ip, IPMSG_SENDMSG | IPMSG_SENDCHECKOPT);
    QString additionalInfo = Helper::shakeToken();
    additionalInfo.append(QChar('\0'));
    const bool encrypted = MsgCrypto::prepareOutgoing(ip, &additionalInfo, &flags);
    SendMsg sendMsg(QHostAddress(ip),
                    Global::userManager->sendPortForIp(ip),
                    additionalInfo, ""/* extendedInfo */, flags);
    Global::msgThread->addSendMsg(Msg(sendMsg));
    recordOutgoing(ip, Global::userManager->displayNameForIp(ip),
                   tr("[Window shake]"), QStringList(),
                   sendMsg.packetNoString(), false, encrypted);
    return true;
}

void WindowManager::peerIdentity(const QString &ip, QString *login, QString *host) const
{
    if (login) {
        login->clear();
    }
    if (host) {
        host->clear();
    }
    if (!Global::userManager) {
        return;
    }
    const int row = Global::userManager->ipToRow(ip);
    if (row < 0) {
        return;
    }
    if (login) {
        *login = Global::userManager->loginName(row);
    }
    if (host) {
        *host = Global::userManager->host(row);
    }
}

QList<OfflinePeerHint> WindowManager::reachableHints() const
{
    QList<OfflinePeerHint> hints;
    if (!Global::userManager) {
        return hints;
    }
    const int rows = Global::userManager->userCount();
    for (int row = 0; row < rows; ++row) {
        OfflinePeerHint hint;
        hint.ip = Global::userManager->ip(row);
        hint.login = Global::userManager->loginName(row);
        hint.host = Global::userManager->host(row);
        hint.reachable = Global::userManager->isReachable(hint.ip);
        hints.append(hint);
    }
    return hints;
}

bool WindowManager::queueOrDeliverText(const QString &ip, const QString &text,
                                       QWidget *askParent)
{
    if (text.isEmpty() || !Global::offlineQueue || !Global::userManager) {
        return false;
    }
    if (Global::userManager->isReachable(ip)) {
        return deliverText(ip, text);
    }
    if (!Global::preferences->isOfflineQueue) {
        const int ret = QMessageBox::question(
                    askParent,
                    tr("Peer offline"),
                    tr("%1 is offline. Queue this message and send when they come online?")
                    .arg(Global::userManager->displayNameForIp(ip)),
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::Yes);
        if (ret != QMessageBox::Yes) {
            return false;
        }
    }
    QString login;
    QString host;
    peerIdentity(ip, &login, &host);
    Global::offlineQueue->enqueue(ip, text, QStringLiteral("text"), login, host);
    return true;
}

bool WindowManager::queueOrDeliverShake(const QString &ip, QWidget *askParent)
{
    if (ip.isEmpty() || !Global::offlineQueue || !Global::userManager) {
        return false;
    }
    if (Global::userManager->isReachable(ip)) {
        return deliverShake(ip);
    }
    if (!Global::preferences || !Global::preferences->isOfflineQueue) {
        const int ret = QMessageBox::question(
                    askParent,
                    tr("Peer offline"),
                    tr("%1 is offline. Queue this window shake and send when they come online?")
                    .arg(Global::userManager->displayNameForIp(ip)),
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::Yes);
        if (ret != QMessageBox::Yes) {
            return false;
        }
    }
    QString login;
    QString host;
    peerIdentity(ip, &login, &host);
    const QString id = Global::offlineQueue->enqueue(
                ip, Helper::shakeToken(), QStringLiteral("shake"), login, host);
    return !id.isEmpty();
}

void WindowManager::flushOfflineQueue()
{
    if (m_flushing || !Global::offlineQueue || !Global::userManager) {
        return;
    }
    m_flushing = true;

    const QList<OfflinePeerHint> hints = reachableHints();
    foreach (const OfflineItem &item, Global::offlineQueue->items()) {
        if (Global::userManager->isReachable(item.ip)) {
            continue;
        }
        const QString neu = OfflineQueue::matchReachableIp(item.login, item.host, hints);
        if (!neu.isEmpty() && neu != item.ip) {
            Global::offlineQueue->assignIp(item.id, neu);
        }
    }

    QSet<QString> ips;
    foreach (const OfflineItem &item, Global::offlineQueue->items()) {
        ips.insert(item.ip);
    }

    foreach (const QString &ip, ips) {
        if (!Global::userManager->isReachable(ip)) {
            continue;
        }
        const QList<OfflineItem> pending = Global::offlineQueue->takeForIp(ip);
        QList<OfflineItem> failed;
        foreach (const OfflineItem &item, pending) {
            const bool ok = (item.kind == QLatin1String("shake"))
                    ? deliverShake(ip)
                    : deliverText(ip, item.text);
            if (!ok) {
                failed.append(item);
            }
        }
        if (!failed.isEmpty()) {
            Global::offlineQueue->restore(failed);
        }
    }
    m_flushing = false;
}

void WindowManager::sendRichSidecar(const QString &ip, const QString &packetNo,
                                    const QString &html)
{
    if (ip.isEmpty() || packetNo.isEmpty() || html.isEmpty()) {
        return;
    }
    if (!Global::msgThread || !Global::userManager
            || !Global::userManager->peerIsQipmsg(ip)) {
        return;
    }
    const QString body = RichText::sidecarBody(packetNo, html);
    if (body.isEmpty()) {
        return;
    }
    const quint32 flags = Global::userManager->sendFlagsForIp(ip, IPMSG_QIPMSG_EXT);
    SendMsg sendMsg(QHostAddress(ip), Global::userManager->sendPortForIp(ip),
                    body, QString(), flags);
    Global::msgThread->addSendMsg(Msg(sendMsg));
}

void WindowManager::noteRichHtml(QString ip, QString packetNo, QString html)
{
    if (ip.isEmpty() || packetNo.isEmpty() || html.isEmpty() || !Global::messageStore) {
        return;
    }
    if (!Global::userManager || !Global::userManager->peerIsQipmsg(ip)) {
        return;
    }
    const int attached = Global::messageStore->attachRich(ip, packetNo, html);
    if (attached == 0) {
        if (m_pendingRich.size() >= 64) {
            m_pendingRich.erase(m_pendingRich.begin());
        }
        m_pendingRich.insert(sealedStorageKey(ip, packetNo), html);
        return;
    }
    if (attached > 0) {
        ChatWindow *chat = m_chatWindows.value(ip, 0);
        if (chat) {
            chat->applyRichHtml(packetNo, html);
        }
    }
}

void WindowManager::recordOutgoing(const QString &ip, const QString &peerName,
                                   const QString &text,
                                   const QStringList &imagePaths,
                                   const QString &packetNo,
                                   bool expectRead,
                                   bool encrypted,
                                   const QString &richHtml)
{
    QString html = richHtml;
    if (!html.isEmpty() && !RichText::coversPlain(html, text)) {
        html.clear();
    }
    ChatRecord record;
    record.timestamp = QDateTime::currentDateTime();
    record.peerIp = ip;
    record.peerName = peerName;
    record.direction = "out";
    record.text = text;
    record.richHtml = html;
    record.imagePaths = imagePaths;
    record.packetNo = packetNo;
    record.expectRead = expectRead;
    record.readReceipt = false;
    record.encrypted = encrypted;
    Global::messageStore->append(record);

    ChatWindow *w = m_chatWindows.value(ip, 0);
    if (w) {
        w->appendMessage(record.timestamp, Global::userManager->ourself().name(),
                         text, false, record.imagePaths, packetNo, expectRead,
                         false, encrypted, html);
    }
}

void WindowManager::handleReadMsg(Msg msg)
{
    const QString packetNo = ProtocolUtil::readReceiptPacketNo(
                msg->additionalInfo(), msg->packetNoString());
    const QString ip = msg->ip();
    bool bubbleUpdated = false;
    if (!packetNo.isEmpty() && Global::messageStore) {
        Global::messageStore->markRead(ip, packetNo);
    }
    ChatWindow *chat = m_chatWindows.value(ip, 0);
    if (chat && !packetNo.isEmpty()) {
        bubbleUpdated = chat->markPacketRead(packetNo);
    }

    // The classic "envelope opened" dialog stays for the case where the
    // sender is not looking at that chat. Do-not-disturb suppresses it
    // inside createMsgReadedWindow. A visible bubble is enough feedback.
    if (chat && chat->isVisible() && bubbleUpdated) {
        return;
    }
    if (Global::preferences->isReadCheck) {
        createMsgReadedWindow(msg);
    }
}

void WindowManager::maybeAutoReply(const Msg &msg)
{
    if (!(Global::preferences->isAway || Global::preferences->isAutoReply)) {
        return;
    }
    if (msg->flags() & IPMSG_AUTORETOPT) {
        return;
    }

    const QString ip = msg->ip();
    const QDateTime now = QDateTime::currentDateTime();
    const int cooldown = qMax(5, Global::preferences->autoReplyCooldownSecs);
    if (m_lastAutoReply.contains(ip)
            && m_lastAutoReply.value(ip).secsTo(now) < cooldown) {
        return;
    }

    QString text = Global::preferences->autoReplyText.trimmed();
    if (text.isEmpty()) {
        text = tr("I am away from my computer.");
    }

    QString additionalInfo = text;
    additionalInfo.append(QChar('\0'));

    quint32 flags = Global::userManager->sendFlagsForIp(
                ip, IPMSG_SENDMSG | IPMSG_AUTORETOPT | IPMSG_SENDCHECKOPT);
    const bool encrypted = MsgCrypto::prepareOutgoing(ip, &additionalInfo, &flags);
    SendMsg sendMsg(msg->ipAddress(), msg->port(),
                    additionalInfo, ""/* extendedInfo */, flags);
    Global::msgThread->addSendMsg(Msg(sendMsg));

    m_lastAutoReply.insert(ip, now);
    recordOutgoing(ip, peerDisplayName(msg), text, QStringList(),
                   sendMsg.packetNoString(), false, encrypted);
}

ChatWindow *WindowManager::ensureChat(const QString &ip)
{
    ChatWindow *w = m_chatWindows.value(ip, 0);
    if (!w) {
        w = new ChatWindow(ip);
        w->loadHistory();
        m_chatWindows.insert(ip, w);
        connect(w, SIGNAL(chatShown(QString)),
                this, SLOT(clearUnread(QString)));
    } else {
        w->refreshPeerCaption();
    }
    return w;
}

DiscussionWindow *WindowManager::openDiscussion(const QString &id,
                                                bool raiseWindow)
{
    if (id.isEmpty()) {
        return 0;
    }
    DiscussionWindow *w = m_discussionWindows.value(id, 0);
    if (!w) {
        w = new DiscussionWindow(id);
        m_discussionWindows.insert(id, w);
    }
    if (raiseWindow) {
        w->show();
        w->raise();
        w->activateWindow();
    }
    return w;
}

void WindowManager::refreshChatCaptions()
{
    QMapIterator<QString, ChatWindow *> it(m_chatWindows);
    while (it.hasNext()) {
        it.next();
        if (it.value()) {
            it.value()->refreshPeerCaption();
        }
    }
}

ChatWindow *WindowManager::openChat(const QString &ip, bool raiseWindow,
                                    const QString &draft)
{
    ChatWindow *w = ensureChat(ip);
    if (m_typingUntil.value(ip) > QDateTime::currentDateTime()) {
        w->setPeerTyping(true);
    }
    if (!draft.isEmpty()) {
        w->setDraft(draft);
    }
    if (raiseWindow) {
        w->show();
        w->raise();
        w->activateWindow();
    }
    return w;
}

void WindowManager::addUnread(const QString &ip)
{
    m_unreadByIp[ip] = m_unreadByIp.value(ip, 0) + 1;
    emitUnread();
}

void WindowManager::clearUnread(QString ip)
{
    if (m_unreadByIp.value(ip, 0) == 0) {
        return;
    }
    m_unreadByIp.remove(ip);
    emitUnread();
}

int WindowManager::unreadCount() const
{
    int count = 0;
    QMapIterator<QString, int> it(m_unreadByIp);
    while (it.hasNext()) {
        it.next();
        count += it.value();
    }
    return count;
}

int WindowManager::hiddenAlertCount() const
{
    return hidedMsgWindowCount() + unreadCount();
}

void WindowManager::emitUnread()
{
    emit unreadCountChanged(unreadCount());
}

void WindowManager::raiseUnreadChats()
{
    const QList<QString> ips = m_unreadByIp.keys();
    foreach (const QString &ip, ips) {
        if (m_unreadByIp.value(ip, 0) > 0) {
            openChat(ip, true);
        }
    }
}

void WindowManager::createMsgWindow(Msg msg)
{
        MsgWindow *msgWindow = new MsgWindow(msg);

        m_msgWindowList.insert(0, msgWindow);

        if (shouldPopup() && !isPeerGroupMuted(msg->ip())) {
            msgWindow->show();
        }
}

void WindowManager::createMsgReadedWindow(Msg msg)
{
        if (Global::preferences->isDoNotDisturb) {
            return;
        }
        MsgReadedWindow *window = new MsgReadedWindow(msg->owner().name());

        m_msgReadedWindowList.insert(0, window);

        window->show();
}

void WindowManager::destroyMsgReadedWindowList()
{
    while (!m_msgReadedWindowList.isEmpty()) {
        delete m_msgReadedWindowList.takeFirst();
    }
}

void WindowManager::destroyMsgWindowList()
{
    while (!m_msgWindowList.isEmpty()) {
        delete m_msgWindowList.takeFirst();
    }
}

void WindowManager::destroyChatWindows()
{
    QMapIterator<QString, ChatWindow *> it(m_chatWindows);
    while (it.hasNext()) {
        it.next();
        delete it.value();
    }
    m_chatWindows.clear();
}

void WindowManager::destroyDiscussionWindows()
{
    QMapIterator<QString, DiscussionWindow *> it(m_discussionWindows);
    while (it.hasNext()) {
        it.next();
        delete it.value();
    }
    m_discussionWindows.clear();
}

void WindowManager::visibleAllMsgWindow()
{
    foreach (MsgWindow *w, m_msgWindowList) {
        w->setWindowFlags(w->windowFlags() | Qt::WindowStaysOnTopHint);
        QPoint pos = w->pos();
        w->move(pos);
        w->show();
        w->setWindowFlags(w->windowFlags() & ~Qt::WindowStaysOnTopHint);
        w->move(pos);
        w->show();
    }
    raiseUnreadChats();
}

void WindowManager::visibleMsgWindow()
{
    foreach (MsgWindow *w, m_msgWindowList) {
        if (!w->isVisible()) {
            w->show();
        }
    }
    raiseUnreadChats();
}

void WindowManager::visibleAllMsgReadedWindow()
{
    foreach (MsgReadedWindow *w, m_msgReadedWindowList) {
        w->setWindowFlags(w->windowFlags() | Qt::WindowStaysOnTopHint);
        QPoint pos = w->pos();
        w->move(pos);
        w->show();
        w->setWindowFlags(w->windowFlags() & ~Qt::WindowStaysOnTopHint);
        w->move(pos);
        w->show();
    }
}

int WindowManager::hidedMsgWindowCount() const
{
    int count = 0;
    foreach (MsgWindow *w, m_msgWindowList) {
        if (!w->isVisible()) {
            ++count;
        }
    }

    return count;
}
