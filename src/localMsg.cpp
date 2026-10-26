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

#include "localMsg.h"
#include "helper.h"
#include "peerFold.h"
#include "winShare.h"

#include <QCryptographicHash>
#include <QLocalServer>
#include <QLocalSocket>
#include <QThread>

namespace {

const int kMaxTargetChars = 160;
const int kMaxTextBytes = 12000;
const int kMaxBufferBytes = 20000;

bool sameToken(const QString &left, const QString &right)
{
    return left.compare(right, Qt::CaseInsensitive) == 0;
}

bool peerKnown(const QList<LocalPeerRef> &peers, const QString &ip, bool *reachable)
{
    bool found = false;
    bool up = false;
    for (int i = 0; i < peers.size(); ++i) {
        if (peers.at(i).ip != ip) {
            continue;
        }
        found = true;
        if (peers.at(i).reachable) {
            up = true;
        }
    }
    if (reachable) {
        *reachable = up;
    }
    return found;
}

} // namespace

const int LocalMsg::maxTextChars = 4000;

QString LocalMsg::serverName()
{
    const QByteArray digest = QCryptographicHash::hash(
                Helper::appHomePath().toUtf8(), QCryptographicHash::Sha256).toHex();
    return QStringLiteral("qipmsg-msg-") + QString::fromLatin1(digest.left(12));
}

QString LocalMsg::usageText()
{
    return QStringLiteral(
            "usage: qipmsg --msg <ipv4-or-login@host> --text <message>\n"
            "Sends one plain text message through the qipmsg that is already running.\n"
            "stdout: OK sent <ip>  or  OK queued <ip>\n"
            "exit 1: the running copy refused (stderr ERR ...)\n"
            "exit 2: bad command line\n"
            "exit 3: qipmsg is not running");
}

LocalCli LocalMsg::parseCli(const QStringList &args)
{
    LocalCli cli;
    cli.isMsg = false;
    bool sawText = false;
    for (int i = 1; i < args.size(); ++i) {
        const QString arg = args.at(i);
        if (arg == QLatin1String("--msg")) {
            cli.isMsg = true;
            if (i + 1 >= args.size() || args.at(i + 1).startsWith(QLatin1String("--"))) {
                cli.error = QStringLiteral("missing target after --msg");
                return cli;
            }
            cli.target = args.at(++i);
        } else if (arg == QLatin1String("--text")) {
            if (i + 1 >= args.size()) {
                cli.error = QStringLiteral("missing message after --text");
                return cli;
            }
            cli.text = args.at(++i);
            sawText = true;
        }
    }
    if (cli.isMsg && !sawText) {
        cli.error = QStringLiteral("pass --text with the message");
    } else if (cli.isMsg && cli.text.isEmpty()) {
        cli.error = QStringLiteral("message is empty");
    }
    return cli;
}

LocalPlan LocalMsg::planDelivery(const QString &target, const QString &text,
                                 const QList<LocalPeerRef> &peers,
                                 bool queueEnabled)
{
    LocalPlan plan;
    plan.kind = localPlanBad;
    if (text.trimmed().isEmpty()) {
        plan.kind = localPlanEmpty;
        return plan;
    }
    if (text.contains(QChar(0))) {
        plan.kind = localPlanNul;
        return plan;
    }
    if (text.size() > maxTextChars || text.toUtf8().size() > kMaxTextBytes) {
        plan.kind = localPlanTooLong;
        return plan;
    }
    const QString trimmed = target.trimmed();
    if (trimmed.isEmpty() || trimmed.size() > kMaxTargetChars
            || trimmed.contains(QLatin1Char('\n'))
            || trimmed.contains(QLatin1Char('\r'))
            || trimmed.contains(QChar(0))) {
        return plan;
    }
    QString normalized;
    if (WinShare::isSafeIpv4(trimmed, &normalized)) {
        plan.ip = normalized;
        bool reachable = false;
        if (!peerKnown(peers, normalized, &reachable)) {
            plan.kind = localPlanSend;
            return plan;
        }
        if (reachable) {
            plan.kind = localPlanSend;
            return plan;
        }
        plan.kind = queueEnabled ? localPlanQueue : localPlanOffline;
        return plan;
    }
    const int at = trimmed.indexOf(QLatin1Char('@'));
    if (at <= 0 || at != trimmed.lastIndexOf(QLatin1Char('@'))
            || at >= trimmed.size() - 1) {
        return plan;
    }
    const QString login = trimmed.left(at);
    const QString host = trimmed.mid(at + 1);
    if (login.contains(QLatin1Char(' ')) || host.contains(QLatin1Char(' '))
            || host.contains(QLatin1Char('\\')) || host.contains(QLatin1Char('/'))) {
        return plan;
    }
    QList<int> matches;
    for (int i = 0; i < peers.size(); ++i) {
        if (!sameToken(peers.at(i).login, login) || !sameToken(peers.at(i).host, host)) {
            continue;
        }
        matches.append(i);
    }
    if (matches.isEmpty()) {
        plan.kind = localPlanMissing;
        return plan;
    }
    // login@host names one person even when that host has two NICs.
    // The old path returned ERR ambiguous for bob@host on two addresses.
    // That is the FeiQ dual-home case, so pick the same preferred
    // address the contact list uses (fresh LAN over fresh 10/8).
    int chosen = matches.first();
    if (matches.size() > 1) {
        QList<PeerFoldEnd> ends;
        for (int n = 0; n < matches.size(); ++n) {
            const LocalPeerRef &peer = peers.at(matches.at(n));
            PeerFoldEnd end;
            end.ip = peer.ip;
            end.fresh = peer.reachable;
            ends.append(end);
        }
        const int best = preferredPeerIndex(ends);
        if (best >= 0 && best < matches.size()) {
            chosen = matches.at(best);
        }
    }
    plan.ip = peers.at(chosen).ip;
    if (peers.at(chosen).reachable) {
        plan.kind = localPlanSend;
    } else {
        plan.kind = queueEnabled ? localPlanQueue : localPlanOffline;
    }
    return plan;
}

QString LocalMsg::planError(const LocalPlan &plan)
{
    switch (plan.kind) {
    case localPlanBad:
        return QStringLiteral("ERR bad-target");
    case localPlanMissing:
        return QStringLiteral("ERR not-found");
    case localPlanAmbiguous:
        return QStringLiteral("ERR ambiguous");
    case localPlanEmpty:
        return QStringLiteral("ERR empty");
    case localPlanTooLong:
        return QStringLiteral("ERR too-long");
    case localPlanNul:
        return QStringLiteral("ERR nul");
    case localPlanOffline:
        return QStringLiteral("ERR offline");
    case localPlanSend:
        return QStringLiteral("OK sent ") + plan.ip;
    case localPlanQueue:
        return QStringLiteral("OK queued ") + plan.ip;
    }
    return QStringLiteral("ERR send-failed");
}

QByteArray LocalMsg::encodeRequest(const QString &target, const QString &text,
                                   QString *error)
{
    if (error) {
        error->clear();
    }
    if (target.trimmed().isEmpty() || target.contains(QLatin1Char('\n'))
            || target.contains(QLatin1Char('\r')) || target.contains(QChar(0))
            || target.size() > kMaxTargetChars) {
        if (error) {
            *error = QStringLiteral("bad-target");
        }
        return QByteArray();
    }
    if (text.isEmpty()) {
        if (error) {
            *error = QStringLiteral("empty");
        }
        return QByteArray();
    }
    if (text.contains(QChar(0))) {
        if (error) {
            *error = QStringLiteral("nul");
        }
        return QByteArray();
    }
    const QByteArray body = text.toUtf8();
    if (text.size() > maxTextChars || body.size() > kMaxTextBytes) {
        if (error) {
            *error = QStringLiteral("too-long");
        }
        return QByteArray();
    }
    QByteArray out;
    out.append("QIP1\n");
    out.append(target.toUtf8());
    out.append('\n');
    out.append(QByteArray::number(body.size()));
    out.append('\n');
    out.append(body);
    return out;
}

bool LocalMsg::takeRequest(QByteArray *buffer, QString *target, QString *text,
                           QString *error, bool *needMore)
{
    if (needMore) {
        *needMore = false;
    }
    if (error) {
        error->clear();
    }
    if (!buffer) {
        if (error) {
            *error = QStringLiteral("bad-header");
        }
        return false;
    }
    const int first = buffer->indexOf('\n');
    if (first < 0) {
        if (needMore) {
            *needMore = buffer->size() < 32;
        }
        if (buffer->size() >= 32 && error) {
            *error = QStringLiteral("bad-header");
        }
        return false;
    }
    if (buffer->left(first) != QByteArray("QIP1")) {
        if (error) {
            *error = QStringLiteral("bad-header");
        }
        return false;
    }
    const int second = buffer->indexOf('\n', first + 1);
    if (second < 0) {
        if (needMore) {
            *needMore = buffer->size() < 400;
        }
        if (buffer->size() >= 400 && error) {
            *error = QStringLiteral("bad-target");
        }
        return false;
    }
    const QByteArray targetBytes = buffer->mid(first + 1, second - first - 1);
    if (targetBytes.isEmpty() || targetBytes.size() > kMaxTargetChars
            || targetBytes.contains('\r')) {
        if (error) {
            *error = QStringLiteral("bad-target");
        }
        return false;
    }
    const int third = buffer->indexOf('\n', second + 1);
    if (third < 0) {
        if (needMore) {
            *needMore = buffer->size() < 420;
        }
        if (buffer->size() >= 420 && error) {
            *error = QStringLiteral("bad-length");
        }
        return false;
    }
    bool ok = false;
    const int length = buffer->mid(second + 1, third - second - 1).toInt(&ok);
    if (!ok || length < 0 || length > kMaxTextBytes) {
        if (error) {
            *error = QStringLiteral("bad-length");
        }
        return false;
    }
    const int bodyAt = third + 1;
    if (buffer->size() < bodyAt + length) {
        if (needMore) {
            *needMore = true;
        }
        return false;
    }
    const QByteArray body = buffer->mid(bodyAt, length);
    if (target) {
        *target = QString::fromUtf8(targetBytes);
    }
    if (text) {
        *text = QString::fromUtf8(body);
    }
    buffer->remove(0, bodyAt + length);
    return true;
}

LocalSendResult LocalMsg::sendToServer(const QString &name, const QString &target,
                                       const QString &text, int attempts)
{
    LocalSendResult result;
    result.code = 2;
    QString encodeError;
    const QByteArray payload = encodeRequest(target, text, &encodeError);
    if (payload.isEmpty()) {
        result.error = encodeError.isEmpty()
                ? QStringLiteral("bad-target") : encodeError;
        return result;
    }
    if (attempts < 1) {
        attempts = 1;
    }
    for (int attempt = 0; attempt < attempts; ++attempt) {
        QLocalSocket socket;
        socket.connectToServer(name);
        if (!socket.waitForConnected(200)) {
            QThread::msleep(150);
            continue;
        }
        if (socket.write(payload) != payload.size() || !socket.waitForBytesWritten(2000)) {
            result.code = 1;
            result.error = QStringLiteral("write failed");
            return result;
        }
        QByteArray got;
        while (!got.contains('\n') && got.size() < 400) {
            if (!socket.waitForReadyRead(2000)) {
                break;
            }
            got += socket.readAll();
        }
        const int nl = got.indexOf('\n');
        if (nl < 0) {
            result.code = 1;
            result.error = QStringLiteral("bad response");
            return result;
        }
        result.response = QString::fromUtf8(got.left(nl));
        if (result.response.startsWith(QLatin1String("OK "))) {
            result.code = 0;
            return result;
        }
        result.code = 1;
        result.error = result.response;
        return result;
    }
    result.code = 3;
    result.error = QStringLiteral("qipmsg is not running");
    return result;
}

LocalMsgServer::LocalMsgServer(QObject *parent)
    : QObject(parent), m_server(0)
{
}

void LocalMsgServer::setHandler(const Handler &handler)
{
    m_handler = handler;
}

bool LocalMsgServer::start(const QString &name, QString *error)
{
    if (error) {
        error->clear();
    }
    if (m_server) {
        if (error) {
            *error = QStringLiteral("already listening");
        }
        return false;
    }
    const QString chosen = name.isEmpty() ? LocalMsg::serverName() : name;
    QLocalServer::removeServer(chosen);
    m_server = new QLocalServer(this);
    m_server->setSocketOptions(QLocalServer::UserAccessOption);
    if (!m_server->listen(chosen)) {
        if (error) {
            *error = m_server->errorString();
        }
        return false;
    }
    connect(m_server, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
    return true;
}

QString LocalMsgServer::name() const
{
    if (!m_server) {
        return QString();
    }
    return m_server->serverName();
}

void LocalMsgServer::acceptConnection()
{
    if (!m_server) {
        return;
    }
    while (m_server->hasPendingConnections()) {
        QLocalSocket *socket = m_server->nextPendingConnection();
        if (!socket) {
            break;
        }
        m_buffers.insert(socket, QByteArray());
        connect(socket, SIGNAL(readyRead()), this, SLOT(readSocket()));
        connect(socket, SIGNAL(disconnected()), this, SLOT(discardSocket()));
        if (socket->bytesAvailable() > 0) {
            consume(socket);
        }
    }
}

void LocalMsgServer::readSocket()
{
    consume(qobject_cast<QLocalSocket *>(sender()));
}

void LocalMsgServer::consume(QLocalSocket *socket)
{
    if (!socket || !m_buffers.contains(socket)) {
        return;
    }
    QByteArray buffer = m_buffers.value(socket);
    buffer += socket->readAll();
    if (buffer.size() > kMaxBufferBytes) {
        socket->write("ERR too-long\n");
        socket->disconnectFromServer();
        m_buffers.remove(socket);
        return;
    }
    QString target;
    QString text;
    QString error;
    bool needMore = false;
    if (!LocalMsg::takeRequest(&buffer, &target, &text, &error, &needMore)) {
        m_buffers.insert(socket, buffer);
        if (needMore || error.isEmpty()) {
            return;
        }
        socket->write("ERR ");
        socket->write(error.toUtf8());
        socket->write("\n");
        socket->disconnectFromServer();
        return;
    }
    m_buffers.insert(socket, buffer);
    QString reply = QStringLiteral("ERR not-ready");
    if (m_handler) {
        reply = m_handler(target, text);
    }
    reply.replace(QLatin1Char('\n'), QLatin1Char(' '));
    reply.replace(QLatin1Char('\r'), QLatin1Char(' '));
    if (reply.size() > 300) {
        reply.truncate(300);
    }
    socket->write(reply.toUtf8());
    socket->write("\n");
    socket->flush();
    socket->disconnectFromServer();
}

void LocalMsgServer::discardSocket()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());
    if (!socket) {
        return;
    }
    m_buffers.remove(socket);
    socket->deleteLater();
}
