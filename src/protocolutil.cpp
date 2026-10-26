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

#include "protocolutil.h"
#include "constants.h"

#include <QRegularExpression>

namespace {

bool parseHost(const QString &text, quint32 *host)
{
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty() || !host) {
        return false;
    }
    QHostAddress address;
    if (!address.setAddress(trimmed)) {
        return false;
    }
    if (address.protocol() != QAbstractSocket::IPv4Protocol) {
        return false;
    }
    *host = address.toIPv4Address();
    return true;
}

quint32 maskForPrefix(int prefix)
{
    if (prefix <= 0) {
        return 0;
    }
    if (prefix >= 32) {
        return 0xFFFFFFFFu;
    }
    return 0xFFFFFFFFu << (32 - prefix);
}

bool isProbeHost(quint32 host)
{
    if (host == 0 || host == 0xFFFFFFFFu) {
        return false;
    }
    if ((host >> 24) == 127u) {
        return false;
    }
    return true;
}

bool spanHasProbeHost(quint32 startHost, quint32 endHost)
{
    quint32 ip = startHost;
    while (true) {
        if (isProbeHost(ip)) {
            return true;
        }
        if (ip == endHost) {
            return false;
        }
        ++ip;
    }
}

} // namespace

int ProtocolUtil::maxIpv4RangeHosts()
{
    return 1024;
}

int ProtocolUtil::inspectIpv4Token(const QString &raw, QString *canonical,
                                   quint32 *startHost, quint32 *endHost)
{
    const QString text = raw.trimmed();
    if (text.isEmpty()) {
        return Ipv4TokenEmpty;
    }

    QString body = text;
    body.replace(QChar(0xFF0D), QLatin1Char('-'));
    body.replace(QLatin1Char('~'), QLatin1Char('-'));
    body.remove(QLatin1Char(' '));

    const int slash = body.indexOf(QLatin1Char('/'));
    const int dash = body.indexOf(QLatin1Char('-'));
    if (slash >= 0 && dash >= 0) {
        return Ipv4TokenInvalid;
    }

    quint32 start = 0;
    quint32 end = 0;
    bool cidr = false;
    int prefix = 32;

    if (slash >= 0) {
        cidr = true;
        bool ok = false;
        prefix = body.mid(slash + 1).toInt(&ok);
        if (!ok || prefix < 0 || prefix > 32 || body.mid(slash + 1).isEmpty()) {
            return Ipv4TokenBadPrefix;
        }
        if (!parseHost(body.left(slash), &start)) {
            return Ipv4TokenInvalid;
        }
        const quint32 mask = maskForPrefix(prefix);
        start = start & mask;
        end = start | ~mask;
    } else if (dash >= 0) {
        if (dash == 0 || dash == body.size() - 1) {
            return Ipv4TokenInvalid;
        }
        if (!parseHost(body.left(dash), &start)
                || !parseHost(body.mid(dash + 1), &end)) {
            return Ipv4TokenInvalid;
        }
        if (start > end) {
            const quint32 tmp = start;
            start = end;
            end = tmp;
        }
    } else {
        if (!parseHost(body, &start)) {
            return Ipv4TokenInvalid;
        }
        end = start;
    }

    if (startHost) {
        *startHost = start;
    }
    if (endHost) {
        *endHost = end;
    }

    const quint64 count = quint64(end) - quint64(start) + 1;
    if (count > quint64(maxIpv4RangeHosts())) {
        return Ipv4TokenTooLarge;
    }
    if (!spanHasProbeHost(start, end)) {
        return Ipv4TokenUnusable;
    }

    if (canonical) {
        if (cidr) {
            *canonical = QString("%1/%2")
                    .arg(QHostAddress(start).toString())
                    .arg(prefix);
        } else if (start != end) {
            *canonical = QString("%1-%2")
                    .arg(QHostAddress(start).toString(),
                         QHostAddress(end).toString());
        } else {
            *canonical = QHostAddress(start).toString();
        }
    }
    return Ipv4TokenOk;
}

QStringList ProtocolUtil::expandIpv4Hosts(quint32 startHost, quint32 endHost,
                                          bool *truncated)
{
    if (startHost > endHost) {
        const quint32 tmp = startHost;
        startHost = endHost;
        endHost = tmp;
    }

    bool cut = false;
    const quint64 count = quint64(endHost) - quint64(startHost) + 1;
    if (count > quint64(maxIpv4RangeHosts())) {
        const quint64 limitEnd = quint64(startHost)
                + quint64(maxIpv4RangeHosts() - 1);
        endHost = quint32(qMin(limitEnd, quint64(0xFFFFFFFFull)));
        cut = true;
    }
    if (truncated) {
        *truncated = cut;
    }

    QStringList hosts;
    quint32 ip = startHost;
    while (true) {
        if (isProbeHost(ip)) {
            hosts << QHostAddress(ip).toString();
        }
        if (ip == endHost) {
            break;
        }
        ++ip;
    }
    return hosts;
}

QList<QHostAddress> ProtocolUtil::targetsFromBroadcastTokens(
        const QStringList &tokens, int *invalidCount, int *truncatedCount)
{
    int invalid = 0;
    int truncated = 0;
    QList<QHostAddress> out;

    foreach (const QString &token, tokens) {
        quint32 start = 0;
        quint32 end = 0;
        const int status = inspectIpv4Token(token, 0, &start, &end);
        if (status == Ipv4TokenEmpty) {
            continue;
        }
        if (status != Ipv4TokenOk && status != Ipv4TokenTooLarge) {
            ++invalid;
            continue;
        }
        bool cut = false;
        const QStringList hosts = expandIpv4Hosts(start, end, &cut);
        if (cut) {
            ++truncated;
        }
        foreach (const QString &host, hosts) {
            out << QHostAddress(host);
        }
    }

    if (invalidCount) {
        *invalidCount = invalid;
    }
    if (truncatedCount) {
        *truncatedCount = truncated;
    }
    return out;
}

bool ProtocolUtil::commandFromDatagram(const QByteArray &datagram,
                                       quint32 *command)
{
    int field = 0;
    int start = 0;
    for (int i = 0; i <= datagram.size(); ++i) {
        const bool atEnd = (i == datagram.size());
        if (!atEnd) {
            const char ch = datagram.at(i);
            if (ch != ':' && ch != '\0') {
                continue;
            }
        }
        if (field == MSG_FLAGS_POS) {
            const QByteArray raw = datagram.mid(start, i - start).trimmed();
            bool ok = false;
            const quint32 value = raw.toULong(&ok, 10);
            if (!ok || raw.isEmpty()) {
                return false;
            }
            if (command) {
                *command = value;
            }
            return true;
        }
        if (atEnd || datagram.at(i) == '\0') {
            return false;
        }
        ++field;
        start = i + 1;
    }
    return false;
}

bool ProtocolUtil::isSecretEnvelope(quint32 flags)
{
    if (GET_MODE(flags) != IPMSG_SENDMSG) {
        return false;
    }
    return (flags & IPMSG_SECRETOPT) || (flags & IPMSG_READCHECKOPT);
}

namespace {

bool isDecimalPacket(const QString &text)
{
    if (text.isEmpty() || text.size() > 20) {
        return false;
    }
    for (int i = 0; i < text.size(); ++i) {
        if (!text.at(i).isDigit()) {
            return false;
        }
    }
    return true;
}

} // namespace

bool ProtocolUtil::isDialupOption(quint32 flags)
{
    return (flags & IPMSG_DIALUPOPT) != 0;
}

bool ProtocolUtil::isFeiqTypingCommand(quint32 command, bool *active)
{
    const quint32 mode = GET_MODE(command);
    if (mode != IPMSG_FEIQ_TYPING && mode != IPMSG_FEIQ_TYPINGSTOP) {
        return false;
    }
    if (command != mode) {
        return false;
    }
    if (active) {
        *active = (mode == IPMSG_FEIQ_TYPING);
    }
    return true;
}

int ProtocolUtil::etaSeconds(qint64 bytesSent, qint64 bytesTotal,
                             qint64 elapsedMs)
{
    if (bytesSent <= 0 || bytesTotal <= bytesSent || elapsedMs <= 0) {
        return -1;
    }
    const double perSec = (double)bytesSent * 1000.0 / (double)elapsedMs;
    if (perSec <= 0.0) {
        return -1;
    }
    const int secs = (int)((double)(bytesTotal - bytesSent) / perSec + 0.5);
    return secs < 1 ? 1 : secs;
}

namespace {

bool parseHexU32(const QString &text, quint32 *out)
{
    QString hex = text.trimmed();
    if (hex.startsWith(QLatin1String("0x"), Qt::CaseInsensitive)) {
        hex = hex.mid(2);
    }
    if (hex.isEmpty() || !out) {
        return false;
    }
    bool ok = false;
    const qulonglong value = hex.toULongLong(&ok, 16);
    if (!ok || value > 0xffffffffULL) {
        return false;
    }
    *out = static_cast<quint32>(value);
    return true;
}

bool parseHexI64(const QString &text, qint64 *out)
{
    const QString hex = text.trimmed();
    if (hex.isEmpty() || !out) {
        return false;
    }
    bool ok = false;
    const qint64 value = hex.toLongLong(&ok, 16);
    if (!ok) {
        return false;
    }
    *out = value;
    return true;
}

QString unescapeFileName(QString name)
{
    name.replace(QString(QChar('\a')) + QChar('\a'), QStringLiteral(":"));
    return name;
}

void takeExtendAttrs(const QStringList &fields, int begin, QMap<int, QString> *extend)
{
    if (!extend) {
        return;
    }
    for (int i = begin; i < fields.size(); ++i) {
        const QString item = fields.at(i);
        const int eq = item.indexOf(QLatin1Char('='));
        if (eq <= 0) {
            continue;
        }
        bool ok = false;
        const int key = item.left(eq).trimmed().toInt(&ok, 16);
        if (ok) {
            extend->insert(key, item.mid(eq + 1));
        }
    }
}

int kindFromWord(quint32 attrWord)
{
    const int kind = ProtocolUtil::fileAttrKind(attrWord, true);
    if (kind >= 0) {
        return kind;
    }
    return static_cast<int>(GET_MODE(attrWord));
}

FileAttachParsed blankAttachment(const QString &segment)
{
    FileAttachParsed parsed;
    parsed.parsed = false;
    parsed.fileId = 0;
    parsed.size = 0;
    parsed.attrWord = 0;
    parsed.type = -1;
    parsed.segment = segment;
    return parsed;
}

} // namespace

int ProtocolUtil::fileAttrKind(quint32 attrWord, bool zeroIsRegular)
{
    const quint32 mode = GET_MODE(attrWord);
    if (mode == IPMSG_FILE_DIR) {
        return IPMSG_FILE_DIR;
    }
    if (mode == IPMSG_FILE_RETPARENT) {
        return IPMSG_FILE_RETPARENT;
    }
    if (mode == IPMSG_FILE_REGULAR || mode == IPMSG_FILE_CLIPBOARD) {
        return IPMSG_FILE_REGULAR;
    }
    if (mode == 0 && zeroIsRegular) {
        return IPMSG_FILE_REGULAR;
    }
    return -1;
}

QStringList ProtocolUtil::fileAttachSegments(const QString &packet)
{
    QStringList out;
    const QStringList parts = packet.split(QChar(QLatin1Char('\0')));
    for (int i = 1; i < parts.size(); ++i) {
        const QStringList files = parts.at(i).split(QChar('\a'), Qt::SkipEmptyParts);
        foreach (const QString &file, files) {
            if (!file.isEmpty()) {
                out << file;
            }
        }
    }
    return out;
}

FileAttachParsed ProtocolUtil::parseFileAttachment(const QString &segment)
{
    FileAttachParsed parsed = blankAttachment(segment);
    QString info = segment;
    info.replace(QStringLiteral("::"), QString(QChar('\a')) + QChar('\a'));
    const QStringList fields = info.split(QLatin1Char(':'));
    if (fields.size() < 5) {
        return parsed;
    }

    bool idOk = false;
    const int fileId = fields.at(0).trimmed().toInt(&idOk, 10);
    qint64 classicSize = 0;
    quint32 classicWord = 0;
    const bool classicSizeOk = parseHexI64(fields.at(2), &classicSize);
    const bool classicAttrOk = parseHexU32(fields.at(4), &classicWord);
    if (classicSizeOk && classicAttrOk) {
        parsed.parsed = true;
        parsed.fileId = idOk ? fileId : 0;
        parsed.name = unescapeFileName(fields.at(1));
        parsed.size = classicSize;
        parsed.rawAttr = fields.at(4).trimmed();
        parsed.attrWord = classicWord;
        parsed.type = kindFromWord(classicWord);
        takeExtendAttrs(fields, 5, &parsed.extend);
        return parsed;
    }

    QStringList tail = fields;
    if (!tail.isEmpty() && tail.last().isEmpty()) {
        tail.removeLast();
    }
    QMap<int, QString> extend;
    while (tail.size() > 5) {
        const QString item = tail.last();
        const int eq = item.indexOf(QLatin1Char('='));
        if (eq <= 0) {
            break;
        }
        bool keyOk = false;
        const int key = item.left(eq).trimmed().toInt(&keyOk, 16);
        if (!keyOk) {
            break;
        }
        extend.insert(key, item.mid(eq + 1));
        tail.removeLast();
    }
    if (tail.size() >= 5) {
        quint32 attrWord = 0;
        qint64 size = 0;
        quint32 mtime = 0;
        const bool attrOk = parseHexU32(tail.last(), &attrWord);
        const bool sizeOk = parseHexI64(tail.at(tail.size() - 3), &size);
        const bool mtimeOk = parseHexU32(tail.at(tail.size() - 2), &mtime);
        Q_UNUSED(mtime);
        if (attrOk && sizeOk && mtimeOk) {
            bool rightIdOk = false;
            const int rightId = tail.at(0).trimmed().toInt(&rightIdOk, 10);
            QStringList nameParts;
            for (int i = 1; i < tail.size() - 3; ++i) {
                nameParts << tail.at(i);
            }
            parsed.parsed = true;
            parsed.fileId = rightIdOk ? rightId : 0;
            parsed.name = unescapeFileName(nameParts.join(QLatin1Char(':')));
            parsed.size = size;
            parsed.rawAttr = tail.last().trimmed();
            parsed.attrWord = attrWord;
            parsed.type = kindFromWord(attrWord);
            parsed.extend = extend;
            return parsed;
        }
    }

    // Size is present but fileattr is not hex. Old toInt stored 0 and the
    // transfer then refused the file. Keep the record and download it.
    if (classicSizeOk) {
        parsed.parsed = true;
        parsed.fileId = idOk ? fileId : 0;
        parsed.name = unescapeFileName(fields.at(1));
        parsed.size = classicSize;
        parsed.rawAttr = fields.at(4).trimmed();
        parsed.attrWord = 0;
        parsed.type = IPMSG_FILE_REGULAR;
        takeExtendAttrs(fields, 5, &parsed.extend);
    }
    return parsed;
}

QByteArray ProtocolUtil::makeFilePullRequest(const QByteArray &version,
                                            const QByteArray &requestPacketNo,
                                            const QByteArray &loginName,
                                            const QByteArray &hostName,
                                            quint32 command,
                                            qint64 offerPacketNo,
                                            int fileId,
                                            qint64 offset,
                                            bool withOffset)
{
    QByteArray out;
    out.reserve(version.size() + requestPacketNo.size() + loginName.size()
                + hostName.size() + 48);
    out.append(version);
    out.append(':');
    out.append(requestPacketNo);
    out.append(':');
    out.append(loginName);
    out.append(':');
    out.append(hostName);
    out.append(':');
    out.append(QByteArray::number(static_cast<qulonglong>(command)));
    out.append(':');
    out.append(QByteArray::number(static_cast<qlonglong>(offerPacketNo), 16));
    out.append(':');
    out.append(QByteArray::number(fileId, 16));
    out.append(':');
    if (withOffset) {
        out.append(QByteArray::number(static_cast<qlonglong>(offset), 16));
        out.append(':');
    }
    out.append('\0');
    return out;
}

QString ProtocolUtil::readReceiptPacketNo(const QString &additionalInfo,
                                          const QString &packetNoField)
{
    QString body = additionalInfo;
    const int nul = body.indexOf(QChar(QLatin1Char('\0')));
    if (nul >= 0) {
        body = body.left(nul);
    }
    body = body.trimmed();

    QString token;
    const int space = body.indexOf(QRegularExpression(QStringLiteral("\\s")));
    if (space < 0) {
        token = body;
    } else {
        token = body.left(space);
    }
    if (isDecimalPacket(token)) {
        return token;
    }

    const QString field = packetNoField.trimmed();
    if (body.isEmpty() && isDecimalPacket(field)) {
        return field;
    }
    return QString();
}
