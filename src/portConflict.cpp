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

#include "portConflict.h"

#include "global.h"
#include "ipmsg.h"
#include "portBind.h"
#include "preferences.h"
#include "systray.h"

#include <QAbstractButton>
#include <QHostAddress>
#include <QMessageBox>
#include <QPushButton>
#include <QWidget>
#include <QtDebug>

namespace {

QWidget *windowForDialog(QWidget *parent)
{
    if (parent) {
        return parent;
    }
    if (!Global::systray) {
        return 0;
    }
    return Global::systray->openMainWindow();
}

void disarmQuit(QMessageBox *box)
{
    if (!box) {
        return;
    }
    // Closing the warning must not be treated as the last window closing.
    // That used to leave a tray-only process after WM_CLOSE.
    box->setAttribute(Qt::WA_QuitOnClose, false);
}

} // namespace

QString PortConflict::sideLine(const QString &name, const portSideResult &side)
{
    if (side.ok) {
        return tr("%1: listening").arg(name);
    }
    QString why;
    switch (side.kind) {
    case portFailKind::inUse:
        why = tr("already in use");
        break;
    case portFailKind::accessDenied:
        why = side.windowsExcluded
                ? tr("blocked by a Windows excluded port range")
                : tr("access denied");
        break;
    case portFailKind::addressUnavailable:
        why = tr("address is not available");
        break;
    case portFailKind::none:
    case portFailKind::other:
        why = tr("could not bind");
        break;
    }
    if (side.error.isEmpty()) {
        return tr("%1: %2").arg(name, why);
    }
    return tr("%1: %2 (%3)").arg(name, why, side.error);
}

QString PortConflict::explainFailure(const portListenResult &result)
{
    const bool excluded = result.udp.windowsExcluded || result.tcp.windowsExcluded;
    if (excluded) {
        return tr(
                "Windows refused this port because it is inside an excluded "
                "range. Hyper-V, NAT, and some VPN clients reserve blocks "
                "that include 2425. An administrator can list them with: "
                "netsh interface ipv4 show excludedportrange protocol=tcp");
    }
    const bool denied = result.udp.kind == portFailKind::accessDenied
            || result.tcp.kind == portFailKind::accessDenied;
    if (denied) {
        return tr(
                "The system refused this port (access denied). On Windows "
                "that often means an excluded port range rather than FeiQ "
                "itself. An administrator can check: "
                "netsh interface ipv4 show excludedportrange protocol=tcp");
    }
    const bool busy = result.udp.kind == portFailKind::inUse
            || result.tcp.kind == portFailKind::inUse;
    if (busy) {
        return tr(
                "Another program already has this port. That is often "
                "FeiQ, IP Messenger, or a second copy of qipmsg. Quit that "
                "program to stay on 2425, or accept a different port below.");
    }
    const bool missing = result.udp.kind == portFailKind::addressUnavailable
            || result.tcp.kind == portFailKind::addressUnavailable;
    if (missing) {
        return tr(
                "The selected network address is not available. "
                "Preferences → Detail → Network can switch back to Any IPv4.");
    }
    return tr(
            "The port could not be opened. Preferences → Detail → Network "
            "can change it. A restart is required after editing it there.");
}

void PortConflict::offerDefaultPort(QWidget *parent, const QHostAddress &address,
                                    quint16 port)
{
    if (!Global::preferences || !Global::preferences->portFallback) {
        return;
    }
    if (port == IPMSG_DEFAULT_PORT) {
        Global::preferences->portFallback = false;
        Global::preferences->save();
        return;
    }
    const portListenResult standard = PortBind::probe(address, IPMSG_DEFAULT_PORT);
    if (!standard.ok()) {
        return;
    }

    QMessageBox box(parent);
    disarmQuit(&box);
    box.setIcon(QMessageBox::Question);
    box.setWindowTitle(tr("Port 2425 is free"));
    box.setText(tr(
            "QIpMsg is using port %1 because an earlier start could not open "
            "2425. Port 2425 (IPMSG / FeiQ) is free now. Switch back so other "
            "clients can find you on the usual port?\n\n"
            "Closing this keeps port %1. You can also change it later in "
            "Preferences → Detail → Network.")
            .arg(port));
    QPushButton *backButton = box.addButton(
            tr("Switch back to 2425"), QMessageBox::AcceptRole);
    QPushButton *keepButton = box.addButton(
            tr("Keep port %1").arg(port), QMessageBox::RejectRole);
    box.setDefaultButton(backButton);
    box.setEscapeButton(keepButton);
    box.exec();

    if (box.clickedButton() == backButton) {
        Global::preferences->udpPort = IPMSG_DEFAULT_PORT;
    }
    Global::preferences->portFallback = false;
    Global::preferences->save();
}

bool PortConflict::askUseSuggested(QWidget *parent, const portListenResult &result,
                                   const QHostAddress &address, quint16 suggested)
{
    QMessageBox box(parent);
    disarmQuit(&box);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle(tr("Listen port unavailable"));

    const QString where = address.toString();
    QString suggestion;
    if (suggested != 0) {
        suggestion = PortConflict::tr(
                "A free port is available: %1. It will be saved as udpPort "
                "and used for both UDP discovery and TCP file transfer.")
                .arg(suggested);
    } else {
        suggestion = PortConflict::tr(
                "No free alternate port was found. The contact window stays "
                "open. Discovery or file transfer stays off until a port "
                "can be opened.");
    }
    box.setText(tr(
            "QIpMsg could not listen on port %1 (%2).\n\n"
            "%3\n%4\n\n"
            "%5\n\n"
            "%6\n\n"
            "Port 2425 is the IPMSG / FeiQ default. qipmsg keeps it whenever "
            "it is free, and changes it only if you accept. Peers that still "
            "use 2425 will not see a client that moved. If this choice moved "
            "you off 2425, a later start can offer to switch back once 2425 "
            "is free. Closing this dialog keeps the current port and leaves "
            "the contact window open.")
            .arg(result.port)
            .arg(where)
            .arg(sideLine(tr("UDP"), result.udp))
            .arg(sideLine(tr("TCP"), result.tcp))
            .arg(explainFailure(result))
            .arg(suggestion));

    QPushButton *useButton = 0;
    if (suggested != 0) {
        useButton = box.addButton(
                tr("Use port %1").arg(suggested),
                QMessageBox::AcceptRole);
        box.setDefaultButton(useButton);
    }
    QPushButton *keepButton = box.addButton(
            tr("Continue anyway"), QMessageBox::RejectRole);
    box.setEscapeButton(keepButton);
    if (!useButton) {
        box.setDefaultButton(keepButton);
    }
    box.exec();
    return useButton && box.clickedButton() == useButton;
}

bool PortConflict::resolveBeforeListen(QWidget *parent)
{
    if (!Global::preferences) {
        return false;
    }
    const QHostAddress address = Global::preferences->listenAddress();
    const quint16 port = Global::preferences->listenPort();
    const portListenResult now = PortBind::probe(address, port);
    if (now.ok()) {
        offerDefaultPort(parent, address, port);
        return false;
    }

    portListenResult failed = now;
    PortBind::annotateExcluded(&failed);
    QWidget *dialogParent = windowForDialog(parent);
    const quint16 suggested = PortBind::suggestFree(address, port);
    qWarning() << "PortConflict: port" << port
               << "udp" << failed.udp.error
               << "tcp" << failed.tcp.error
               << "suggest" << suggested;

    if (!askUseSuggested(dialogParent, failed, address, suggested)
            || suggested == 0) {
        return true;
    }

    Global::preferences->udpPort = suggested;
    Global::preferences->portFallback = suggested != IPMSG_DEFAULT_PORT;
    Global::preferences->save();

    const portListenResult again = PortBind::probe(
            address, static_cast<quint16>(suggested));
    if (again.ok()) {
        return false;
    }
    qWarning() << "PortConflict: suggested port still failed" << suggested
               << again.udp.error << again.tcp.error;
    QMessageBox box(dialogParent);
    disarmQuit(&box);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle(tr("Listen port unavailable"));
    box.setText(tr("Port %1 also failed to open. The contact window stays "
                   "open, but discovery or file transfer may not work.\n\n"
                   "UDP: %2\nTCP: %3")
                .arg(suggested)
                .arg(again.udp.error.isEmpty() ? tr("failed") : again.udp.error)
                .arg(again.tcp.error.isEmpty() ? tr("failed") : again.tcp.error));
    QPushButton *closeButton = box.addButton(tr("Continue anyway"),
                                             QMessageBox::RejectRole);
    box.setDefaultButton(closeButton);
    box.setEscapeButton(closeButton);
    box.exec();
    return true;
}

void PortConflict::warnStillDown(QWidget *parent, quint16 port, const QString &detail)
{
    QWidget *dialogParent = parent;
    if (!dialogParent && Global::systray) {
        dialogParent = Global::systray->openMainWindow();
    }
    QMessageBox box(dialogParent);
    disarmQuit(&box);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle(tr("Start QIpMsg"));
    box.setText(tr(
            "UDP discovery or the TCP file service is not listening on "
            "port %1. The contact window stays open. Change the port in "
            "Preferences → Detail → Network if this keeps happening.\n\n%2")
            .arg(port)
            .arg(detail));
    QPushButton *closeButton = box.addButton(tr("Continue anyway"),
                                             QMessageBox::RejectRole);
    box.setDefaultButton(closeButton);
    box.setEscapeButton(closeButton);
    box.exec();
}
