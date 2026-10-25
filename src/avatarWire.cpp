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

#include "avatarWire.h"

#include <QBuffer>
#include <QCryptographicHash>

const int AvatarWire::maxPngBytes = 96 * 1024;
const int AvatarWire::idHexLen = 12;

namespace {

bool commandIs126(const QByteArray &field)
{
    bool ok = false;
    const quint32 hex = field.toUInt(&ok, 16);
    if (ok && hex == 126u) {
        return true;
    }
    const quint32 decimal = field.toUInt(&ok, 10);
    return ok && decimal == 126u;
}

bool hexId(const QString &id)
{
    if (id.size() != AvatarWire::idHexLen) {
        return false;
    }
    for (int i = 0; i < id.size(); ++i) {
        const QChar c = id.at(i);
        const bool hex = (c >= QLatin1Char('0') && c <= QLatin1Char('9'))
                || (c >= QLatin1Char('a') && c <= QLatin1Char('f'));
        if (!hex) {
            return false;
        }
    }
    return true;
}

} // namespace

bool AvatarWire::idOk(const QString &id)
{
    return hexId(id.toLower());
}

QString AvatarWire::shortIdOf(const QByteArray &bytes)
{
    if (bytes.isEmpty()) {
        return QString();
    }
    const QByteArray hex = QCryptographicHash::hash(
                bytes, QCryptographicHash::Sha256).toHex();
    return QString::fromLatin1(hex.left(idHexLen));
}

QString AvatarWire::hashFromToken(const QString &token)
{
    if (token != QLatin1String("qipmsg")
            && !token.startsWith(QLatin1String("qipmsg,"))) {
        return QString();
    }
    QString found;
    const QStringList parts = token.split(QLatin1Char(','));
    for (int i = 0; i < parts.size(); ++i) {
        const QString part = parts.at(i);
        if (!part.startsWith(QLatin1String("av:"))) {
            continue;
        }
        const QString id = part.mid(3).toLower();
        if (hexId(id)) {
            found = id;
        }
    }
    return found;
}

QByteArray AvatarWire::buildRequest()
{
    QByteArray packet;
    packet.append("1:1:qip:qipmsg:");
    packet.append(QByteArray::number(126, 16));
    packet.append(":AVATAR:::");
    packet.append('\n');
    return packet;
}

bool AvatarWire::isRequest(const QByteArray &packet)
{
    QByteArray body = packet;
    const int nl = body.indexOf('\n');
    if (nl >= 0) {
        body = body.left(nl);
    }
    const QList<QByteArray> parts = body.split(':');
    if (parts.size() < 6) {
        return false;
    }
    if (QString::fromLatin1(parts.at(5)) != QLatin1String("AVATAR")) {
        return false;
    }
    return commandIs126(parts.at(4));
}

bool AvatarWire::pngLooksOk(const QByteArray &png)
{
    static const char kSig[] = "\x89PNG\r\n\x1a\n";
    if (png.size() < 8 || !png.startsWith(kSig)) {
        return false;
    }
    if (png.size() > maxPngBytes) {
        return false;
    }
    return !QImage::fromData(png, "PNG").isNull();
}

QByteArray AvatarWire::makeWirePng(const QImage &source, QString *error)
{
    if (source.isNull()) {
        if (error) {
            *error = QStringLiteral("read");
        }
        return QByteArray();
    }
    int edge = 128;
    for (int attempt = 0; attempt < 4; ++attempt) {
        QImage scaled = source;
        if (scaled.width() > edge || scaled.height() > edge) {
            scaled = scaled.scaled(edge, edge, Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation);
        }
        QByteArray bytes;
        QBuffer buffer(&bytes);
        if (!buffer.open(QIODevice::WriteOnly) || !scaled.save(&buffer, "PNG")) {
            if (error) {
                *error = QStringLiteral("read");
            }
            return QByteArray();
        }
        if (pngLooksOk(bytes)) {
            if (error) {
                error->clear();
            }
            return bytes;
        }
        edge /= 2;
        if (edge < 32) {
            break;
        }
    }
    if (error) {
        *error = QStringLiteral("big");
    }
    return QByteArray();
}

QByteArray AvatarWire::headerFor(const QByteArray &png, QString *errorCode)
{
    if (!pngLooksOk(png)) {
        if (errorCode) {
            *errorCode = png.isEmpty() ? QStringLiteral("none")
                                       : (png.size() > maxPngBytes
                                          ? QStringLiteral("big")
                                          : QStringLiteral("read"));
        }
        return QByteArray();
    }
    const QString id = shortIdOf(png);
    QByteArray header = "QIP1\nOK ";
    header.append(id.toLatin1());
    header.append(' ');
    header.append(QByteArray::number(png.size()));
    header.append('\n');
    if (errorCode) {
        errorCode->clear();
    }
    return header;
}

int AvatarWire::takeHeader(const QByteArray &buf, QString *id, int *pngBytes,
                           QString *errorCode)
{
    if (id) {
        id->clear();
    }
    if (pngBytes) {
        *pngBytes = -1;
    }
    if (errorCode) {
        errorCode->clear();
    }
    const int first = buf.indexOf('\n');
    if (first < 0) {
        return buf.size() > 64 ? -2 : -1;
    }
    if (buf.left(first) != "QIP1") {
        return -2;
    }
    const int second = buf.indexOf('\n', first + 1);
    if (second < 0) {
        return buf.size() > 256 ? -2 : -1;
    }
    const QByteArray line = buf.mid(first + 1, second - first - 1);
    if (line.startsWith("ERR")) {
        if (errorCode) {
            QByteArray token = line.mid(3).trimmed();
            *errorCode = QString::fromLatin1(token);
        }
        return second + 1;
    }
    if (!line.startsWith("OK ")) {
        return -2;
    }
    const QList<QByteArray> bits = line.mid(3).split(' ');
    if (bits.size() < 2) {
        return -2;
    }
    const QString photoId = QString::fromLatin1(bits.at(0)).toLower();
    if (!hexId(photoId)) {
        return -2;
    }
    bool ok = false;
    const int count = bits.at(1).toInt(&ok);
    if (!ok || count < 8 || count > maxPngBytes) {
        return -2;
    }
    if (id) {
        *id = photoId;
    }
    if (pngBytes) {
        *pngBytes = count;
    }
    return second + 1;
}
