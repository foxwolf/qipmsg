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

#include "avatarFetch.h"
#include "avatarStore.h"
#include "avatarWire.h"
#include "global.h"
#include "ipmsg.h"
#include "shareFolder.h"
#include "user_manager.h"

#include <QHostAddress>
#include <QTcpSocket>
#include <QTimer>

namespace {

const int kFetchTimeoutMs = 8000;
const int kMaxQueue = 32;

} // namespace

AvatarFetch::AvatarFetch(QObject *parent)
    : QObject(parent), m_socket(0), m_timer(0), m_pngSize(-1), m_headerAt(-1),
      m_busy(false), m_closing(false)
{
    m_socket = new QTcpSocket(this);
    connect(m_socket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(m_socket, &QTcpSocket::errorOccurred,
            this, &AvatarFetch::onError);
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

void AvatarFetch::republish()
{
    if (!Global::userManager) {
        return;
    }
    Global::userManager->announcePresence();
}

QString AvatarFetch::explain(const QString &code) const
{
    if (code == QLatin1String("none")) {
        return tr("The peer has no photo to send.");
    }
    if (code == QLatin1String("big")) {
        return tr("The photo is larger than qipmsg will transfer.");
    }
    if (code == QLatin1String("read")) {
        return tr("The photo data was not a readable PNG.");
    }
    if (code.isEmpty()) {
        return tr("The peer closed the photo connection.");
    }
    return tr("The peer refused the photo (%1).").arg(code);
}

void AvatarFetch::consider(const QString &ip)
{
    if (ip.isEmpty() || !Global::avatarStore || !Global::userManager) {
        return;
    }
    AvatarStore *store = Global::avatarStore;
    if (store->hasManual(ip)) {
        store->setStatus(ip, tr("Using a photo set on this computer. "
                                "It overrides the peer photo."));
        return;
    }

    const QString token = Global::userManager->peerQipToken(ip);
    const bool qip = ShareFolder::tokenIsQipmsg(token);
    const QString id = AvatarWire::hashFromToken(token);
    const QString skip = store->skipId(ip);
    if (!id.isEmpty() && (skip == id || skip == QLatin1String("*"))) {
        store->setWireHidden(ip, true);
        store->setStatus(ip, tr("Photo hidden on this computer until the peer "
                                "publishes a different one."));
        return;
    }
    if (!qip) {
        store->setStatus(ip, tr("FeiQ and classic IPMSG have no photo packet "
                                "this client can use. The color block stays. "
                                "You can still set a local photo for this IP."));
        return;
    }
    if (id.isEmpty()) {
        store->setWireHidden(ip, true);
        store->setStatus(ip, tr("This qipmsg peer has not published a photo."));
        return;
    }
    if (store->wireId(ip) == id && store->hasWire(ip)) {
        store->setWireHidden(ip, false);
        store->setStatus(ip, tr("Photo from this qipmsg peer."));
        return;
    }
    if (!Global::userManager->isReachable(ip)) {
        store->setStatus(ip, tr("Photo %1 is published, but the peer is not reachable.")
                         .arg(id));
        return;
    }
    if (m_current == ip || m_queue.contains(ip)) {
        store->setStatus(ip, tr("Fetching photo…"));
        return;
    }
    if (m_queue.size() >= kMaxQueue) {
        store->setStatus(ip, tr("Photo fetch is waiting. Too many peers are queued."));
        return;
    }
    m_queue.append(ip);
    m_expect.insert(ip, id);
    store->setStatus(ip, tr("Fetching photo…"));
    startNext();
}

void AvatarFetch::clearLocal(const QString &ip)
{
    if (!Global::avatarStore || ip.isEmpty()) {
        return;
    }
    AvatarStore *store = Global::avatarStore;
    if (store->hasManual(ip)) {
        store->clearManual(ip);
        consider(ip);
        return;
    }
    if (store->hasWire(ip) || !store->skipId(ip).isEmpty()) {
        QString id = store->wireId(ip);
        if (!AvatarWire::idOk(id) && Global::userManager) {
            id = AvatarWire::hashFromToken(Global::userManager->peerQipToken(ip));
        }
        store->clearWire(ip);
        store->writeSkip(ip, AvatarWire::idOk(id) ? id : QStringLiteral("*"));
        store->setWireHidden(ip, true);
        store->setStatus(ip, tr("Photo hidden on this computer until the peer "
                                "publishes a different one."));
        return;
    }
    store->setStatus(ip, tr("No saved photo for this IP."));
}

void AvatarFetch::startNext()
{
    if (m_busy || m_queue.isEmpty() || !m_socket) {
        return;
    }
    m_current = m_queue.takeFirst();
    m_wantId = m_expect.take(m_current);
    if (!Global::userManager || !Global::userManager->isReachable(m_current)) {
        if (Global::avatarStore) {
            Global::avatarStore->setStatus(
                        m_current,
                        tr("Photo %1 is published, but the peer is not reachable.")
                        .arg(m_wantId));
        }
        m_current.clear();
        QTimer::singleShot(0, this, SLOT(startNext()));
        return;
    }
    m_busy = true;
    m_buf.clear();
    m_pngSize = -1;
    m_headerAt = -1;
    m_closing = true;
    m_socket->abort();
    m_closing = false;
    if (m_timer) {
        m_timer->start(kFetchTimeoutMs);
    }
    quint16 port = Global::userManager->sendPortForIp(m_current);
    if (port == 0) {
        port = IPMSG_DEFAULT_PORT;
    }
    m_socket->connectToHost(QHostAddress(m_current), port);
}

void AvatarFetch::onConnected()
{
    if (!m_busy || !m_socket) {
        return;
    }
    m_socket->write(AvatarWire::buildRequest());
}

void AvatarFetch::onReadyRead()
{
    if (!m_busy || !m_socket || !Global::avatarStore) {
        return;
    }
    m_buf.append(m_socket->readAll());
    if (m_buf.size() > AvatarWire::maxPngBytes + 256) {
        fail(tr("The photo is larger than qipmsg will transfer."));
        return;
    }
    if (m_headerAt < 0) {
        QString id;
        QString errorCode;
        int pngBytes = -1;
        const int taken = AvatarWire::takeHeader(m_buf, &id, &pngBytes, &errorCode);
        if (taken == -1) {
            return;
        }
        if (taken == -2) {
            fail(tr("The peer sent a photo reply this client could not read."));
            return;
        }
        m_headerAt = taken;
        if (!errorCode.isEmpty()) {
            fail(explain(errorCode));
            return;
        }
        if (id != m_wantId) {
            fail(tr("The photo did not match the advertised id."));
            return;
        }
        m_pngSize = pngBytes;
    }
    if (m_headerAt < 0 || m_pngSize < 0) {
        return;
    }
    if (m_buf.size() < m_headerAt + m_pngSize) {
        return;
    }
    const QByteArray png = m_buf.mid(m_headerAt, m_pngSize);
    if (!AvatarWire::pngLooksOk(png) || AvatarWire::shortIdOf(png) != m_wantId) {
        fail(tr("The photo data was not a readable PNG."));
        return;
    }
    const QString ip = m_current;
    const QString id = m_wantId;
    if (Global::avatarStore->skipId(ip) == id) {
        stopSocket();
        return;
    }
    QString error;
    if (!Global::avatarStore->saveWire(ip, png, id, &error)) {
        fail(error.isEmpty() ? tr("Could not store that photo.") : error);
        return;
    }
    Global::avatarStore->setStatus(ip, tr("Photo from this qipmsg peer."));
    stopSocket();
}

void AvatarFetch::onDisconnected()
{
    if (m_closing || !m_busy) {
        return;
    }
    onReadyRead();
    if (!m_busy) {
        return;
    }
    fail(tr("The peer closed the photo connection."));
}

void AvatarFetch::onError(QAbstractSocket::SocketError)
{
    if (m_closing || !m_busy) {
        return;
    }
    const QString why = m_socket ? m_socket->errorString() : QString();
    fail(tr("Could not fetch the photo (%1).")
         .arg(why.isEmpty() ? tr("connection failed") : why));
}

void AvatarFetch::onTimeout()
{
    if (!m_busy) {
        return;
    }
    fail(tr("Could not fetch the photo (timed out)."));
}

void AvatarFetch::fail(const QString &reason)
{
    const QString ip = m_current;
    if (Global::avatarStore && !ip.isEmpty()) {
        Global::avatarStore->setStatus(ip, reason);
    }
    stopSocket();
}

void AvatarFetch::stopSocket()
{
    m_closing = true;
    if (m_timer) {
        m_timer->stop();
    }
    if (m_socket) {
        m_socket->abort();
    }
    m_closing = false;
    m_busy = false;
    m_current.clear();
    m_buf.clear();
    m_pngSize = -1;
    m_headerAt = -1;
    QTimer::singleShot(0, this, SLOT(startNext()));
}
