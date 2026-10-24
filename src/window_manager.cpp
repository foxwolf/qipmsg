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

#include <QHostAddress>
#include <QMessageBox>
#include <QPoint>
#include <QSet>
#include <QSize>

WindowManager::WindowManager(QObject *parent)
    : QObject(parent)
{
    connect(Global::userManager, SIGNAL(usersChanged()),
            this, SLOT(flushOfflineQueue()));
    connect(Global::userManager, SIGNAL(usersChanged()),
            this, SLOT(refreshChatCaptions()));
}

WindowManager::~WindowManager()
{
    destroyMsgWindowList();
    destroyMsgReadedWindowList();
    destroyChatWindows();
    destroyDiscussionWindows();
}

void WindowManager::newMsg(Msg msg)
{
    switch (GET_MODE(msg->flags())) {
    case IPMSG_SENDMSG:
        handleIncomingSendMsg(msg);
        break;
    case IPMSG_READMSG:
        createMsgReadedWindow(msg);
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

bool WindowManager::shouldPopup() const
{
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
            recordIncomingText(letter.ip, peerName, shown, false);
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
    const bool isShake = !sealed && !attach && Helper::isShakeText(text);

    if (isShake) {
        recordIncomingText(msg->ip(), name, tr("[Window shake]"), !autoReply);
        applyIncomingShake(msg->ip());
        if (!autoReply) {
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
            recordIncomingText(msg->ip(), name, shown, !autoReply);
        } else if (attach) {
            recordIncomingText(msg->ip(), name,
                               tr("[File attachment]"), !autoReply);
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
            emit newTextMessage(name, tr("(Sealed message)"));
            if (shouldPopup() && !attach) {
                openChat(msg->ip(), true);
            } else if (!chat->isVisible()) {
                addUnread(msg->ip());
            }
        }
    }

    if (!autoReply) {
        maybeAutoReply(msg);
        playNoticeSound();
    }

    if (sealed) {
        if (attach) {
            createMsgWindow(msg);
        }
    } else if (attach) {
        createMsgWindow(msg);
    } else if (shouldPopup()) {
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
    if (Global::systray) {
        foreach (MainWindow *mw, Global::systray->mainWindowList) {
            if (mw && mw->isVisible()) {
                Helper::shakeWidget(mw);
                return;
            }
        }
    }
}

void WindowManager::recordIncomingText(const QString &ip,
                                       const QString &peerName,
                                       const QString &text,
                                       bool notifyUser,
                                       const QStringList &imagePaths)
{
    ChatRecord record;
    record.timestamp = QDateTime::currentDateTime();
    record.peerIp = ip;
    record.peerName = peerName;
    record.direction = "in";
    record.text = text;
    record.imagePaths = imagePaths;
    Global::messageStore->append(record);

    ChatWindow *w = m_chatWindows.value(ip, 0);
    if (w) {
        w->appendMessage(record.timestamp, peerName, text, true,
                         record.imagePaths);
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
        const quint32 flags = Global::userManager->sendFlagsForIp(
                    ip, IPMSG_SENDMSG | IPMSG_SENDCHECKOPT | IPMSG_BROADCASTOPT);
        QString additionalInfo = text;
        additionalInfo.append(QChar('\0'));
        SendMsg sendMsg(QHostAddress(ip),
                        Global::userManager->sendPortForIp(ip),
                        additionalInfo, ""/* extendedInfo */, flags);
        Global::msgThread->addSendMsg(Msg(sendMsg));
        recordOutgoing(ip, Global::userManager->displayNameForIp(ip),
                       tr("[Broadcast] %1").arg(text));
        ++sent;
    }
    return sent;
}

void WindowManager::deliverText(const QString &ip, const QString &text)
{
    QString additionalInfo = text;
    additionalInfo.append(QChar('\0'));
    const quint32 flags = Global::userManager->sendFlagsForIp(
                ip, IPMSG_SENDMSG | IPMSG_SENDCHECKOPT);
    SendMsg sendMsg(QHostAddress(ip),
                    Global::userManager->sendPortForIp(ip),
                    additionalInfo, ""/* extendedInfo */, flags);
    Global::msgThread->addSendMsg(Msg(sendMsg));
    recordOutgoing(ip, Global::userManager->displayNameForIp(ip), text);
}

bool WindowManager::queueOrDeliverText(const QString &ip, const QString &text,
                                       QWidget *askParent)
{
    if (text.isEmpty()) {
        return false;
    }
    if (Global::userManager->isReachable(ip)) {
        deliverText(ip, text);
        return true;
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
    Global::offlineQueue->enqueue(ip, text);
    return true;
}

void WindowManager::flushOfflineQueue()
{
    if (!Global::offlineQueue || !Global::userManager) {
        return;
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
        foreach (const OfflineItem &item, pending) {
            deliverText(ip, item.text);
        }
    }
}

void WindowManager::recordOutgoing(const QString &ip, const QString &peerName,
                                   const QString &text,
                                   const QStringList &imagePaths)
{
    ChatRecord record;
    record.timestamp = QDateTime::currentDateTime();
    record.peerIp = ip;
    record.peerName = peerName;
    record.direction = "out";
    record.text = text;
    record.imagePaths = imagePaths;
    Global::messageStore->append(record);

    ChatWindow *w = m_chatWindows.value(ip, 0);
    if (w) {
        w->appendMessage(record.timestamp, Global::userManager->ourself().name(),
                         text, false, record.imagePaths);
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

    const quint32 flags = Global::userManager->sendFlagsForIp(
                ip, IPMSG_SENDMSG | IPMSG_AUTORETOPT | IPMSG_SENDCHECKOPT);
    SendMsg sendMsg(msg->ipAddress(), msg->port(),
                    additionalInfo, ""/* extendedInfo */, flags);
    Global::msgThread->addSendMsg(Msg(sendMsg));

    m_lastAutoReply.insert(ip, now);
    recordOutgoing(ip, peerDisplayName(msg), text);
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

        if (shouldPopup()) {
            msgWindow->show();
        }
}

void WindowManager::createMsgReadedWindow(Msg msg)
{
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
