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

#ifndef AVATARWIRE_H
#define AVATARWIRE_H

#include <QByteArray>
#include <QImage>
#include <QString>

// qipmsg peer photos. Classic IPMSG has no photo command. FeiQ keeps
// custom faces in local folders (UserHeadPhoto, CustomFace, RecvFace,
// SendFace). Public captures show a version token such as
// 1_lbt…#level#MAC#… and nickname\0group extra fields, not a stable
// avatar opcode. This client keeps version "1" and does not attach the
// photo as a file (FeiQ would pop a receive dialog).
//
// Among qipmsg peers the fourth presence field, which is already
// "qipmsg" or "qipmsg,share", gains ",av:<12 hex>". TCP command 126
// with verb AVATAR returns that small PNG. The request is sent only to
// peers that advertised "qipmsg". FeiQ still needs a human check: it
// should ignore the extra comma tag and the unknown verb.

class AvatarWire
{
public:
    static const int maxPngBytes;
    static const int idHexLen;

    static bool idOk(const QString &id);
    static QString shortIdOf(const QByteArray &bytes);
    static QString hashFromToken(const QString &token);

    static QByteArray buildRequest();
    static bool isRequest(const QByteArray &packet);

    static bool pngLooksOk(const QByteArray &png);
    static QByteArray makeWirePng(const QImage &source, QString *error);
    static QByteArray headerFor(const QByteArray &png, QString *errorCode);

    // -1: need more bytes. -2: not a photo reply. >0: header length.
    // On ERR, pngBytes is -1 and errorCode is the token after ERR.
    static int takeHeader(const QByteArray &buf, QString *id, int *pngBytes,
                          QString *errorCode);
};

#endif // !AVATARWIRE_H
