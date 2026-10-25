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

#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include <QDateTime>
#include <QHash>
#include <QHostAddress>
#include <QList>
#include <QMap>
#include <QObject>
#include <QStringList>

class QTimer;

class QWidget;

#include "owner.h"
#include "msg.h"

class ChatWindow;
class DiscussionWindow;
class MsgWindow;
class MsgReadedWindow;

class WindowManager : public QObject
{
    Q_OBJECT

public:
    WindowManager(QObject *parent = 0);
    ~WindowManager();

    void visibleMsgWindow();
    void visibleAllMsgWindow();
    void visibleAllMsgReadedWindow();
    void raiseUnreadChats();

    ChatWindow *openChat(const QString &ip, bool raiseWindow = true,
                         const QString &draft = QString());
    DiscussionWindow *openDiscussion(const QString &id, bool raiseWindow = true);
    void recordOutgoing(const QString &ip, const QString &peerName,
                        const QString &text,
                        const QStringList &imagePaths = QStringList(),
                        const QString &packetNo = QString(),
                        bool expectRead = false);
    void deliverText(const QString &ip, const QString &text);
    int broadcastTextToOnline(const QString &text);
    bool openSealedLetter(const QString &ip, const QString &packetNo);
    bool queueOrDeliverText(const QString &ip, const QString &text,
                            QWidget *askParent = 0);
    void recordIncomingText(const QString &ip, const QString &peerName,
                            const QString &text, bool notifyUser,
                            const QStringList &imagePaths = QStringList());

    void removeMsgWindow(MsgWindow *w) {
        m_msgWindowList.removeAll(w);
    }

    void removeMsgReadedWindow(MsgReadedWindow *w) {
        m_msgReadedWindowList.removeAll(w);
    }

    int hidedMsgWindowCount() const;
    int unreadCount() const;
    int hiddenAlertCount() const;

signals:
    void unreadCountChanged(int count);
    void newTextMessage(QString fromName, QString preview);
    void peerTypingChanged(QString ip, bool typing);

public slots:
    void clearUnread(QString ip);

private slots:
    void newMsg(Msg msg);
    void destroyMsgReadedWindowList();
    void flushOfflineQueue();
    void refreshChatCaptions();
    void sweepTyping();
    void notePeerTyping(QString ip, bool typing);

private:
    void createMsgWindow(Msg msg);
    void createMsgReadedWindow(Msg msg);
    void handleIncomingSendMsg(Msg msg);
    void handleReadMsg(Msg msg);
    void maybeAutoReply(const Msg &msg);
    void playNoticeSound();
    bool shouldPopup() const;
    bool isPeerGroupMuted(const QString &ip) const;
    QString peerDisplayName(const Msg &msg) const;
    void applyIncomingShake(const QString &ip);
    ChatWindow *ensureChat(const QString &ip);
    void addUnread(const QString &ip);
    void emitUnread();

    void destroyMsgWindowList();
    void destroyChatWindows();
    void destroyDiscussionWindows();
    QString sealedStorageKey(const QString &ip, const QString &packetNo) const;

    struct SealedLetter {
        QString packetNo;
        QString ip;
        QString text;
        QHostAddress address;
        quint16 port;
        bool opened;
        bool broadcast;
    };

    QList<MsgWindow *> m_msgWindowList;
    QList<MsgReadedWindow *> m_msgReadedWindowList;
    QMap<QString, ChatWindow *> m_chatWindows;
    QMap<QString, DiscussionWindow *> m_discussionWindows;
    QMap<QString, int> m_unreadByIp;
    QMap<QString, QDateTime> m_lastAutoReply;
    QHash<QString, SealedLetter> m_sealedLetters;
    QHash<QString, QDateTime> m_typingUntil;
    QTimer *m_typingSweep;
};

#endif // !WINDOW_MANAGER_H
