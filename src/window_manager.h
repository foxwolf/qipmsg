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
#include <QList>
#include <QMap>
#include <QObject>

class QWidget;

#include "owner.h"
#include "msg.h"

class ChatWindow;
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
    void recordOutgoing(const QString &ip, const QString &peerName,
                        const QString &text);
    void deliverText(const QString &ip, const QString &text);
    bool queueOrDeliverText(const QString &ip, const QString &text,
                            QWidget *askParent = 0);
    void recordIncomingText(const QString &ip, const QString &peerName,
                            const QString &text, bool notifyUser);

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

public slots:
    void clearUnread(QString ip);

private slots:
    void newMsg(Msg msg);
    void destroyMsgReadedWindowList();
    void flushOfflineQueue();

private:
    void createMsgWindow(Msg msg);
    void createMsgReadedWindow(Msg msg);
    void handleIncomingSendMsg(Msg msg);
    void maybeAutoReply(const Msg &msg);
    void playNoticeSound();
    bool shouldPopup() const;
    QString peerDisplayName(const Msg &msg) const;
    void applyIncomingShake(const QString &ip);
    ChatWindow *ensureChat(const QString &ip);
    void addUnread(const QString &ip);
    void emitUnread();

    void destroyMsgWindowList();
    void destroyChatWindows();

    QList<MsgWindow *> m_msgWindowList;
    QList<MsgReadedWindow *> m_msgReadedWindowList;
    QMap<QString, ChatWindow *> m_chatWindows;
    QMap<QString, int> m_unreadByIp;
    QMap<QString, QDateTime> m_lastAutoReply;
};

#endif // !WINDOW_MANAGER_H
