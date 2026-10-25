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

#include "portBind.h"
#include "ipmsg.h"

#include <QRegularExpression>
#include <QTcpServer>
#include <QThread>
#include <QUdpSocket>

#ifdef Q_OS_WIN
#include <QProcess>
#endif

namespace {

void appendCandidate(QList<quint16> *ports, int port, quint16 avoid)
{
    if (!ports || port < 1 || port > 65535) {
        return;
    }
    const quint16 value = static_cast<quint16>(port);
    if (value == avoid || ports->contains(value)) {
        return;
    }
    ports->append(value);
}

#ifdef Q_OS_WIN
QString excludedRangeText(bool tcp)
{
    QProcess process;
    QStringList args;
    args << QStringLiteral("interface")
         << QStringLiteral("ipv4")
         << QStringLiteral("show")
         << QStringLiteral("excludedportrange")
         << (tcp ? QStringLiteral("protocol=tcp")
                 : QStringLiteral("protocol=udp"));
    process.start(QStringLiteral("netsh"), args);
    if (!process.waitForStarted(1500)) {
        return QString();
    }
    if (!process.waitForFinished(2500)) {
        process.kill();
        process.waitForFinished(500);
        return QString();
    }
    return QString::fromLocal8Bit(process.readAllStandardOutput()
                                  + process.readAllStandardError());
}
#endif

} // namespace

portFailKind PortBind::classify(QAbstractSocket::SocketError error,
                                const QString &errorString)
{
    if (error == QAbstractSocket::AddressInUseError) {
        return portFailKind::inUse;
    }
    if (error == QAbstractSocket::SocketAccessError) {
        return portFailKind::accessDenied;
    }
    if (error == QAbstractSocket::SocketAddressNotAvailableError) {
        return portFailKind::addressUnavailable;
    }

    const QString folded = errorString.toLower();
    if (folded.contains(QLatin1String("access"))
            || folded.contains(QLatin1String("forbidden"))
            || folded.contains(QLatin1String("10013"))
            || errorString.contains(QStringLiteral("权限"))
            || errorString.contains(QStringLiteral("拒绝访问"))
            || errorString.contains(QStringLiteral("访问权限不允许"))) {
        return portFailKind::accessDenied;
    }
    if (folded.contains(QLatin1String("already"))
            || folded.contains(QLatin1String("in use"))
            || folded.contains(QLatin1String("only one usage"))
            || folded.contains(QLatin1String("10048"))
            || errorString.contains(QStringLiteral("占用"))
            || errorString.contains(QStringLiteral("已在使用"))
            || errorString.contains(QStringLiteral("只允许使用一次"))) {
        return portFailKind::inUse;
    }
    if (folded.contains(QLatin1String("not available"))
            || folded.contains(QLatin1String("not valid"))
            || errorString.contains(QStringLiteral("不可用"))) {
        return portFailKind::addressUnavailable;
    }
    return portFailKind::other;
}

bool PortBind::textContainsPort(const QString &text, quint16 port)
{
    if (text.isEmpty() || port == 0) {
        return false;
    }
    const QRegularExpression re(QStringLiteral("(\\d{1,5})\\s+(\\d{1,5})"));
    QRegularExpressionMatchIterator it = re.globalMatch(text);
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        bool okStart = false;
        bool okEnd = false;
        const int start = match.captured(1).toInt(&okStart);
        const int end = match.captured(2).toInt(&okEnd);
        if (!okStart || !okEnd || start < 0 || end > 65535 || end < start) {
            continue;
        }
        if (port >= start && port <= end) {
            return true;
        }
    }
    return false;
}

QList<quint16> PortBind::candidatePorts(quint16 avoid)
{
    QList<quint16> ports;
    // 2425 stays first whenever it is not the port that just failed,
    // so a temporary move does not hide the IPMSG / FeiQ default.
    appendCandidate(&ports, IPMSG_DEFAULT_PORT, avoid);
    appendCandidate(&ports, 12425, avoid);
    for (int port = 2426; port <= 2440; ++port) {
        appendCandidate(&ports, port, avoid);
    }
    for (int port = 12426; port <= 12440; ++port) {
        appendCandidate(&ports, port, avoid);
    }
    appendCandidate(&ports, 42425, avoid);
    return ports;
}

portSideResult PortBind::tryUdp(QUdpSocket *socket,
                                const QHostAddress &address,
                                quint16 port,
                                QAbstractSocket::BindMode mode,
                                bool retryInUse)
{
    portSideResult side;
    if (!socket) {
        side.error = QStringLiteral("no udp socket");
        return side;
    }
    const int attempts = retryInUse ? 5 : 1;
    for (int attempt = 0; attempt < attempts; ++attempt) {
        if (socket->bind(address, port, mode)) {
            side.ok = true;
            side.kind = portFailKind::none;
            side.error.clear();
            return side;
        }
        side.kind = classify(socket->error(), socket->errorString());
        side.error = socket->errorString();
        socket->close();
        if (!retryInUse || side.kind != portFailKind::inUse
                || attempt + 1 >= attempts) {
            return side;
        }
        QThread::msleep(100);
    }
    return side;
}

portSideResult PortBind::tryTcp(QTcpServer *server,
                                const QHostAddress &address,
                                quint16 port,
                                bool retryInUse)
{
    portSideResult side;
    if (!server) {
        side.error = QStringLiteral("no tcp server");
        return side;
    }
    const int attempts = retryInUse ? 5 : 1;
    for (int attempt = 0; attempt < attempts; ++attempt) {
        if (server->listen(address, port)) {
            side.ok = true;
            side.kind = portFailKind::none;
            side.error.clear();
            return side;
        }
        side.kind = classify(server->serverError(), server->errorString());
        side.error = server->errorString();
        server->close();
        if (!retryInUse || side.kind != portFailKind::inUse
                || attempt + 1 >= attempts) {
            return side;
        }
        QThread::msleep(100);
    }
    return side;
}

portListenResult PortBind::probe(const QHostAddress &address, quint16 port)
{
    portListenResult result;
    result.port = port;

    QUdpSocket udp;
    result.udp = tryUdp(&udp, address, port,
                        QAbstractSocket::DefaultForPlatform, false);
    udp.close();

    QTcpServer tcp;
    result.tcp = tryTcp(&tcp, address, port, false);
    tcp.close();
    return result;
}

void PortBind::annotateExcluded(portListenResult *result)
{
    if (!result) {
        return;
    }
#ifndef Q_OS_WIN
    Q_UNUSED(result);
    return;
#else
    const bool udpNeed = !result->udp.ok
            && result->udp.kind == portFailKind::accessDenied;
    const bool tcpNeed = !result->tcp.ok
            && result->tcp.kind == portFailKind::accessDenied;
    if (udpNeed) {
        const QString text = excludedRangeText(false);
        result->udp.windowsExcluded = textContainsPort(text, result->port);
    }
    if (tcpNeed) {
        const QString text = excludedRangeText(true);
        result->tcp.windowsExcluded = textContainsPort(text, result->port);
    }
#endif
}

quint16 PortBind::suggestFree(const QHostAddress &address, quint16 avoid)
{
    const QList<quint16> ports = candidatePorts(avoid);
    for (int i = 0; i < ports.size(); ++i) {
        const portListenResult result = probe(address, ports.at(i));
        if (result.ok()) {
            return ports.at(i);
        }
    }
    return 0;
}
