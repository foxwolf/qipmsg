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

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

#include <QRandomGenerator>
#include <QThread>

#include "qipmsg.h"
#include "helper.h"
#include "global.h"
#include "translator.h"
#include "lockfile.h"
#include "file_server.h"
#include "msg_thread.h"
#include "send_msg.h"
#include "recv_msg.h"
#include "user_manager.h"
#include "preferences.h"
#include "constants.h"
#include "localMsg.h"
#include "window_manager.h"
#include "systray.h"
#include "portConflict.h"

#include <QSystemTrayIcon>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#include <shlobj.h>
#endif

static void createHomeDirectory();
static int runLocalMessage(const LocalCli &cli);
static void myMessageOutput(QtMsgType type, const QMessageLogContext &context,
                            const QString &msg);
static void reportListenResult(QWidget *parent, bool userContinued);
static void setWindowsAppUserModelId();

int main(int argc, char *argv[])
{
    setWindowsAppUserModelId();
    QApplication app(argc, argv);
    const LocalCli cli = LocalMsg::parseCli(app.arguments());
    if (!cli.error.isEmpty()) {
        fprintf(stderr, "%s\n%s\n",
                cli.error.toLocal8Bit().constData(),
                LocalMsg::usageText().toLocal8Bit().constData());
        return 2;
    }

    qInstallMessageHandler(myMessageOutput);

    qRegisterMetaType<Msg>("Msg");
    qRegisterMetaType<QAbstractSocket::SocketError>("QAbstractSocket::SocketError");

    // Create application home directory
    createHomeDirectory();
    QString iniPath = Helper::appHomePath();
    Helper::setIniPath(iniPath);
    Helper::setAppPath(app.applicationDirPath());

    QString fileName = Helper::appHomePath() + "/qipmsg_internal.log";
    Helper::setInternalLogFileName(fileName);

    // Make sure there is only one instance running.
    LockFile::instance()->setLockFile(Helper::lockFile());
    if (!LockFile::instance()->lock(!cli.isMsg)) {
        if (cli.isMsg) {
            return runLocalMessage(cli);
        }
        qWarning("main: lock '%s' failed, exit.",
                 Helper::lockFile().toUtf8().data());
        return -1;
    }
    if (cli.isMsg) {
        fprintf(stderr, "qipmsg is not running\n%s\n",
                LocalMsg::usageText().toLocal8Bit().constData());
        LockFile::instance()->unlock();
        return 3;
    }

    // Set init package number as random.
    Helper::setPacketNo(QRandomGenerator::global()->bounded(1024));

    // Default UI is Simplified Chinese (zh_CN). Chinese system locales
    // still select zh_*; other locales fall back to the zh_CN catalog.
    Translator::instance()->load(QString());
    Helper::applyAppTheme(&app);

    QIpMsg *qipmsg = new QIpMsg;
    if (Global::iconSet.contains(QLatin1String("normal"))
            && Global::iconSet.value(QLatin1String("normal"))) {
        app.setWindowIcon(*Global::iconSet.value(QLatin1String("normal")));
    }

    // Decide this before any warning dialog. A parentless QMessageBox used
    // to be the only window; WM_CLOSE then left a tray-only process.
    const bool trayAvailable = QSystemTrayIcon::isSystemTrayAvailable();
    app.setQuitOnLastWindowClosed(!trayAvailable);

    QWidget *startupWindow = 0;
    if (Global::systray) {
        startupWindow = Global::systray->presentAtStartup();
    }
    const bool userContinued = PortConflict::resolveBeforeListen(startupWindow);
    if (Global::systray && Global::systray->visibleMainWindow()) {
        startupWindow = Global::systray->visibleMainWindow();
    }

    if (Global::fileServer) {
        Global::fileServer->startListening();
    }

    Global::msgThread->start();
    if (!Global::msgThread->waitUntilBound(5000)) {
        qWarning("main: UDP bind did not finish");
    }

    LocalMsgServer *localServer = new LocalMsgServer(qipmsg);
    localServer->setHandler([](const QString &target, const QString &text) {
        if (!Global::windowManager) {
            return QStringLiteral("ERR not-ready");
        }
        return Global::windowManager->deliverLocalCommand(target, text);
    });
    QString localError;
    if (!localServer->start(QString(), &localError)) {
        qWarning("main: local message channel failed: %s",
                 localError.toUtf8().constData());
    }

    reportListenResult(startupWindow, userContinued);

    Global::userManager->announcePresence();

    int rc = app.exec();

    if (!Global::preferences->isStealth) {
        Global::userManager->broadcastExit();
    }


    delete qipmsg;

    return rc;
}

static int runLocalMessage(const LocalCli &cli)
{
    const LocalSendResult result = LocalMsg::sendToServer(
                LocalMsg::serverName(), cli.target, cli.text, 15);
    if (result.code == 0) {
        fprintf(stdout, "%s\n", result.response.toLocal8Bit().constData());
        return 0;
    }
    const QString line = result.error.isEmpty() ? result.response : result.error;
    fprintf(stderr, "%s\n", line.toLocal8Bit().constData());
    return result.code;
}

static void createHomeDirectory()
{
    // Create qipmsg home directories
    if (!QFile::exists(Helper::appHomePath())) {
        QDir d;
        if (!d.mkdir(Helper::appHomePath())) {
            qWarning("createHomeDirectory: can't create %s",
                     Helper::appHomePath().toUtf8().data());
        }
    }
}

static void myMessageOutput(QtMsgType type, const QMessageLogContext &context,
                            const QString &msg) {
    Q_UNUSED(context)

    QString line = "[" + QTime::currentTime().toString() + "] " + msg;

    switch (type) {
        case QtDebugMsg:
#ifndef NO_DEBUG_ON_CONSOLE
            fprintf(stderr, "Debug: %s\n", line.toLocal8Bit().data());
#endif
            Helper::writeInternalLog(line);
            break;
        case QtWarningMsg:
#ifndef NO_DEBUG_ON_CONSOLE
            fprintf(stderr, "Warning: %s\n", line.toLocal8Bit().data());
#endif
            Helper::writeInternalLog("WARNING: " + line);
            break;
        case QtCriticalMsg:
#ifndef NO_DEBUG_ON_CONSOLE
            fprintf(stderr, "Critical: %s\n", line.toLocal8Bit().data());
#endif
            Helper::writeInternalLog("CRITICAL: " + line);
            break;
        case QtFatalMsg:
#ifndef NO_DEBUG_ON_CONSOLE
            fprintf(stderr, "Fatal: %s\n", line.toLocal8Bit().data());
#endif
            Helper::writeInternalLog("FATAL: " + line);
            abort();                    // deliberately core dump
        case QtInfoMsg:
#ifndef NO_DEBUG_ON_CONSOLE
            fprintf(stderr, "Info: %s\n", line.toLocal8Bit().data());
#endif
            Helper::writeInternalLog("INFO: " + line);
            break;
    }
}

static void setWindowsAppUserModelId()
{
#ifdef Q_OS_WIN
    // Keep the taskbar button and the notification icon on this program
    // instead of a temporary host path such as the installer directory.
    SetCurrentProcessExplicitAppUserModelID(L"QIpMsg.QIpMsg");
#else
    return;
#endif
}

static void reportListenResult(QWidget *parent, bool userContinued)
{
    const bool tcpOk = Global::fileServer && Global::fileServer->isListening();
    const bool udpOk = Global::msgThread && Global::msgThread->udpListening();
    const quint16 port = Global::preferences
            ? Global::preferences->listenPort() : 0;
    if (Global::systray) {
        Global::systray->noteListenProblem(!(tcpOk && udpOk), port);
    }
    if (tcpOk && udpOk) {
        return;
    }
    qWarning() << "main::reportListenResult: udp" << udpOk << "tcp" << tcpOk
               << "port" << port;
    if (userContinued) {
        return;
    }
    const QString detail = Global::fileServer
            ? Global::fileServer->listenError() : QString();
    PortConflict::warnStillDown(parent, port, detail);
}

