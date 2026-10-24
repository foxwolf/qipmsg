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

#include "systray.h"
#include "main_window.h"
#include "setup_window.h"
#include "window_manager.h"
#include "global.h"
#include "preferences.h"
#include "user_manager.h"
#include "helper.h"
#include "about_dialog.h"
#include "history_search_window.h"
#include "transfer_file_window.h"
#include "offline_queue.h"
#include "constants.h"
#include "groupPolicy.h"

#include <QAction>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QApplication>
#include <QMessageBox>
#include <QInputDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QStandardItemModel>
#include <QtDebug>


Systray::Systray(QObject *parent)
    : QObject(parent)
{
    trayIcon = new QSystemTrayIcon;

    createActions();
    createMenus();

    createConnections();

    startTimer(500);

    m_iconKey = "normal";
    m_userCount = 0;
    trayIcon->setIcon(*Global::iconSet.value(m_iconKey));
    trayIcon->setContextMenu(trayIconMenu);
    refreshTooltip();
}

Systray::~Systray()
{
    delete trayIcon;
    trayIcon = 0;
}

void Systray::timerEvent(QTimerEvent *event)
{
    if (Global::windowManager->hiddenAlertCount() > 0) {
        if (m_iconKey == "normal") {
            m_iconKey = "receive";
        } else {
            m_iconKey = "normal";
        }
        trayIcon->setIcon(*Global::iconSet.value(m_iconKey));
    }
}

void Systray::updateToolTip(int i)
{
    m_userCount = i;
    refreshTooltip();
}

void Systray::refreshTooltip()
{
    const int unread = Global::windowManager
            ? Global::windowManager->unreadCount() : 0;
    const int queued = Global::offlineQueue
            ? Global::offlineQueue->count() : 0;
    QString tip;
    if (unread > 0) {
        tip = tr("IP Messenger (%1 users, %2 unread)")
                .arg(m_userCount).arg(unread);
    } else {
        tip = tr("IP Messenger (%1)").arg(m_userCount);
    }
    if (queued > 0) {
        tip += QString(" · ") + tr("%n queued", "", queued);
    }
    if (Global::preferences && Global::preferences->isStealth) {
        tip += QString(" · ") + tr("Stealth");
    } else if (Global::groupPolicy && Global::groupPolicy->hasHidden()) {
        tip += QString(" · ") + tr("Hidden from some groups");
    }
    if (Global::preferences && Global::preferences->isDoNotDisturb) {
        tip += QString(" · ") + tr("Do not disturb");
    }
    trayIcon->setToolTip(tip);
}

void Systray::onNewTextMessage(QString fromName, QString preview)
{
    notifyNewMessage(fromName, preview);
    refreshTooltip();
}

void Systray::notifyNewMessage(const QString &fromName, const QString &preview)
{
    if (Global::preferences->isDoNotDisturb) {
        return;
    }
    if (!Global::preferences->isTrayNotify) {
        return;
    }
    if (!trayIcon->supportsMessages()) {
        return;
    }
    trayIcon->showMessage(tr("New message from %1").arg(fromName),
                          preview,
                          QSystemTrayIcon::Information,
                          TRAY_NOTIFY_MS);
}

void Systray::setAway(bool away)
{
    Global::preferences->isAway = away;
    if (away && !Global::preferences->isAutoReply) {
        Global::preferences->isAutoReply = true;
    }
    Global::preferences->save();
    Global::userManager->announcePresence();
}

void Systray::setStealth(bool stealth)
{
    Global::userManager->applyStealth(stealth);
    Global::preferences->save();
    refreshLeaveActions();
}

void Systray::setDoNotDisturb(bool enabled)
{
    Global::preferences->isDoNotDisturb = enabled;
    Global::preferences->save();
    refreshLeaveActions();
}

void Systray::syncActions()
{
    refreshLeaveActions();
}

void Systray::refreshLeaveActions()
{
    if (awayAction) {
        awayAction->blockSignals(true);
        awayAction->setChecked(Global::preferences->isAway);
        awayAction->blockSignals(false);
    }
    if (stealthAction) {
        stealthAction->blockSignals(true);
        stealthAction->setChecked(Global::preferences->isStealth);
        stealthAction->blockSignals(false);
    }
    if (doNotDisturbAction) {
        doNotDisturbAction->blockSignals(true);
        doNotDisturbAction->setChecked(Global::preferences->isDoNotDisturb);
        doNotDisturbAction->blockSignals(false);
    }
    refreshTooltip();
}

void Systray::createActions()
{
    showTransferFileAction
        = new QAction(tr("Show file transfer monitor (%1)").arg(0), 0);
    delAckWindowAction = new QAction(tr("Delete all acknowledge windows"), 0);
    topWindowAction = new QAction(tr("Top all windows"), 0);
    setupAction = new QAction(tr("Setup preferences..."), 0);
    aboutAction = new QAction(tr("About..."), 0);
    aboutQtAction = new QAction(tr("About Qt"), 0);
    readMsgLogAction = new QAction(tr("Read message log file"), 0);
    findHistoryAction = new QAction(tr("Find in history..."), 0);
    offlineQueueAction = new QAction(tr("Offline message queue"), 0);
    broadcastAction = new QAction(tr("Broadcast to all online..."), 0);
    doNotDisturbAction = new QAction(tr("Do not disturb (don't pop message windows)"), this);
    doNotDisturbAction->setCheckable(true);
    doNotDisturbAction->setChecked(Global::preferences->isDoNotDisturb);
    doNotDisturbAction->setToolTip(tr(
            "Suppress tray balloons and auto-raised chat windows. "
            "Not the same as Away / auto-reply."));
    quitAction = new QAction(tr("Quit IP Messenger"), 0);
}

void Systray::createMenus()
{
    trayIconMenu = new QMenu;

    trayIconMenu->addAction(showTransferFileAction);

    trayIconMenu->addSeparator();
    trayIconMenu->addAction(delAckWindowAction);
    trayIconMenu->addAction(topWindowAction);

    trayIconMenu->addSeparator();
    trayIconMenu->addAction(setupAction);
    trayIconMenu->addAction(aboutAction);
    trayIconMenu->addAction(readMsgLogAction);
    trayIconMenu->addAction(findHistoryAction);
    trayIconMenu->addAction(offlineQueueAction);
    trayIconMenu->addAction(broadcastAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(aboutQtAction);

    trayIconMenu->addSeparator();
    trayIconMenu->addAction(doNotDisturbAction);
    trayIconMenu->addSeparator();
    leaveMenu = new QMenu(tr("Leave"));
    createLeaveMenu();
    trayIconMenu->addMenu(leaveMenu);

    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
}

void Systray::createLeaveMenu()
{
    awayAction = new QAction(tr("Away / Auto-reply"), this);
    awayAction->setCheckable(true);
    awayAction->setChecked(Global::preferences->isAway);
    connect(awayAction, SIGNAL(toggled(bool)),
            this, SLOT(setAway(bool)));
    leaveMenu->addAction(awayAction);

    stealthAction = new QAction(tr("Stealth / Hide mode"), this);
    stealthAction->setCheckable(true);
    stealthAction->setChecked(Global::preferences->isStealth);
    connect(stealthAction, SIGNAL(toggled(bool)),
            this, SLOT(setStealth(bool)));
    leaveMenu->addAction(stealthAction);
    leaveMenu->setEnabled(true);

    connect(doNotDisturbAction, SIGNAL(toggled(bool)),
            this, SLOT(setDoNotDisturb(bool)));
}

void Systray::show()
{
    trayIcon->show();
}

void Systray::createConnections()
{
    connect(showTransferFileAction, SIGNAL(triggered()),
            this, SLOT(showTransferFile()));

    connect(setupAction, SIGNAL(triggered()), this, SLOT(setup()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(quit()));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
    connect(trayIconMenu, SIGNAL(aboutToShow()),
            this, SLOT(refreshLeaveActions()));

    connect(delAckWindowAction, SIGNAL(triggered()),
            Global::windowManager, SLOT(destroyMsgReadedWindowList()));
    connect(topWindowAction, SIGNAL(triggered()),
            this, SLOT(setAllWindowVisible()));
    connect(readMsgLogAction, SIGNAL(triggered()),
            this, SLOT(readLog()));
    connect(findHistoryAction, SIGNAL(triggered()),
            this, SLOT(findInHistory()));
    connect(offlineQueueAction, SIGNAL(triggered()),
            this, SLOT(showOfflineQueue()));
    connect(broadcastAction, SIGNAL(triggered()),
            this, SLOT(broadcastToOnline()));

    connect(Global::userManager, SIGNAL(userCountUpdated(int)),
            this, SLOT(updateToolTip(int)));
    connect(Global::userManager, SIGNAL(usersChanged()),
            this, SLOT(refreshLeaveActions()));
    connect(Global::windowManager, SIGNAL(unreadCountChanged(int)),
            this, SLOT(refreshTooltip()));
    connect(Global::windowManager, SIGNAL(newTextMessage(QString,QString)),
            this, SLOT(onNewTextMessage(QString,QString)));
    if (Global::offlineQueue) {
        connect(Global::offlineQueue, SIGNAL(queueChanged()),
                this, SLOT(refreshOfflineQueueAction()));
    }
    refreshOfflineQueueAction();
}

void Systray::setup()
{
    SetupWindow *setupWindow = new SetupWindow;
    setupWindow->show();
}

void Systray::about()
{
    AboutDialog *about = new AboutDialog;
    about->setWindowFlags(Qt::Widget);
    about->show();
}

void Systray::quit()
{
    destroyMainWindowList();

    qApp->quit();
}

void Systray::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        Global::windowManager->visibleAllMsgReadedWindow();
        if (Global::preferences->isSingleClickSendWindow) {
            createMainWindow();
        }
        if (Global::windowManager->hiddenAlertCount() > 0) {
            Global::windowManager->visibleMsgWindow();
            m_iconKey = "normal";
            trayIcon->setIcon(*Global::iconSet.value(m_iconKey));
        }
        break;

    case QSystemTrayIcon::DoubleClick:
        Global::windowManager->visibleAllMsgReadedWindow();
        if (!Global::preferences->isSingleClickSendWindow) {
            createMainWindow();
        }
        break;

    default:
        break;
    }
}

void Systray::createMainWindow()
{
    MainWindow *mainWindow = new MainWindow;
    mainWindow->show();

    // prepend mainWindow to the list
    mainWindowList.insert(0, mainWindow);
}

void Systray::destroyMainWindowList()
{
    while (!mainWindowList.isEmpty()) {
        delete mainWindowList.takeFirst();
    }
}

// XXX NOTE: We do not use external editor.
void Systray::findInHistory()
{
    HistorySearchWindow *window = new HistorySearchWindow;
    window->show();
    window->raise();
    window->activateWindow();
}

void Systray::readLog()
{
    if (Global::preferences->logFilePath.isEmpty()) {
        QMessageBox::information(0, tr("Read Message Log"),
                tr("No message log file specified.")
                );
        return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(Global::preferences->logFilePath));
}

void Systray::broadcastToOnline()
{
    bool ok = false;
    const QString text = QInputDialog::getMultiLineText(
                0, tr("Broadcast"),
                tr("Message sent to every online peer:"),
                QString(), &ok);
    if (!ok || text.trimmed().isEmpty()) {
        return;
    }
    const int sent = Global::windowManager->broadcastTextToOnline(text);
    if (sent <= 0) {
        QMessageBox::information(0, tr("Broadcast"),
                tr("No online peers to broadcast to."));
        return;
    }
    QMessageBox::information(0, tr("Broadcast"),
            tr("Broadcast sent to %n online peer(s).", "", sent));
}

void Systray::showOfflineQueue()
{
    OfflineQueueDialog *dialog = new OfflineQueueDialog;
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void Systray::refreshOfflineQueueAction()
{
    const int n = Global::offlineQueue ? Global::offlineQueue->count() : 0;
    if (offlineQueueAction) {
        offlineQueueAction->setText(tr("Offline message queue (%1)").arg(n));
    }
    refreshTooltip();
}

void Systray::setAllWindowVisible()
{
    Global::windowManager->visibleAllMsgWindow();
    visibleAllMainWindow();
    Global::windowManager->visibleAllMsgReadedWindow();
}

void Systray::visibleAllMainWindow()
{
    foreach (MainWindow *w, mainWindowList) {
#if 0
        w->hide();
        w->show();
#else
        w->setWindowFlags(w->windowFlags() | Qt::WindowStaysOnTopHint);
        // w->hide();
        QPoint pos = w->pos();
        w->move(pos);
        w->show();
        w->setWindowFlags(w->windowFlags() & ~Qt::WindowStaysOnTopHint);
        w->move(pos);
        w->show();
#endif
    }
}

void Systray::showTransferFile()
{
    qDebug("Systray::showTransferFile");

    TransferFileWindow *transferFileWindow = new TransferFileWindow;

    transferFileWindow->show();
}

void Systray::updateTransferCount(int count)
{
    showTransferFileAction
        ->setText(tr("Show file transfer monitor (%1)").arg(count));
}

