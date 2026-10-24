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

#ifndef SYSTRAY_H
#define SYSTRAY_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QSystemTrayIcon>

class QAction;
class QMenu;
class QActionGroup;
class QIcon;

class MainWindow;

class Systray : public QObject
{
    Q_OBJECT

public:
    Systray(QObject *parent = 0);
    ~Systray();

    void show();
    void syncActions();

    QList<MainWindow *> mainWindowList;
    void notifyNewMessage(const QString &fromName, const QString &preview);

protected:
    void timerEvent(QTimerEvent *event);

private slots:
    void updateToolTip(int i);
    void refreshTooltip();
    void updateTransferCount(int);
    void onNewTextMessage(QString fromName, QString preview);
    void setAway(bool away);
    void setStealth(bool stealth);
    void setDoNotDisturb(bool enabled);
    void refreshLeaveActions();

private slots:
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void setup();
    void about();
    void quit();
    void readLog();
    void findInHistory();
    void setAllWindowVisible();
    void showTransferFile();
    void showOfflineQueue();
    void refreshOfflineQueueAction();
    void broadcastToOnline();

private:
    void createActions();
    void createLeaveMenu();
    void createMenus();
    void createIconSet();
    void createConnections();

    void createMainWindow();
    void destroyMainWindowList();
    void visibleAllMainWindow();

    QAction *showTransferFileAction;
    QAction *delAckWindowAction;
    QAction *topWindowAction;
    QAction *setupAction;
    QAction *aboutAction;
    QAction *aboutQtAction;
    QAction *readMsgLogAction;
    QAction *findHistoryAction;
    QAction *offlineQueueAction;
    QAction *broadcastAction;
    QMenu *leaveMenu;
    QAction *quitAction;

    QActionGroup *leaveActionGroup;
    QAction *awayAction;
    QAction *stealthAction;
    QAction *doNotDisturbAction;
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

    QString m_iconKey;
    int m_userCount;
};

#endif // !SYSTRAY_H

