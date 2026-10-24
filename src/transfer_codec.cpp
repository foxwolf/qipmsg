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

#include "transfer_codec.h"
#include "global.h"
#include "preferences.h"
#include "protocolutil.h"

#include <QSet>

TransferCodec::TransferCodec()
    : m_codec(0)
{
    QString name = "GBK";
    if (Global::preferences && !Global::preferences->transferCodecName.isEmpty()) {
        name = Global::preferences->transferCodecName;
    }
    setTransCodec(name);
}

void TransferCodec::setTransCodec(const QString &codecName)
{
    QByteArray name = codecName.toLatin1();
    if (name.isEmpty()) {
        name = "GBK";
    }

    m_codec = codecForNameSafe(name);
    if (!m_codec) {
        m_codec = codecForNameSafe("GBK");
    }
    if (!m_codec) {
        m_codec = codecForNameSafe("GB2312");
    }
    if (!m_codec) {
        m_codec = codecForNameSafe("UTF-8");
    }
    if (!m_codec) {
        m_codec = QTextCodec::codecForLocale();
    }

    m_codecName = m_codec ? QString::fromLatin1(m_codec->name()) : QString("UTF-8");
}

QTextCodec * TransferCodec::codec() const
{
    if (m_codec) {
        return m_codec;
    }
    QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");
    return utf8 ? utf8 : QTextCodec::codecForLocale();
}

QString TransferCodec::codecName() const
{
    return m_codecName;
}

QTextCodec *TransferCodec::codecForNameSafe(const QByteArray &name) const
{
    if (name.isEmpty()) {
        return 0;
    }
    if (codecMap.contains(name)) {
        return codecMap.value(name);
    }
    return QTextCodec::codecForName(name);
}

QString TransferCodec::decodeWith(QTextCodec *textCodec, const QByteArray &bytes,
                                  int *invalidChars) const
{
    if (!textCodec) {
        if (invalidChars) {
            *invalidChars = bytes.size();
        }
        return QString::fromUtf8(bytes);
    }

    QTextCodec::ConverterState state;
    const QString text = textCodec->toUnicode(bytes.constData(), bytes.size(),
                                              &state);
    int invalid = static_cast<int>(state.invalidChars);
    if (text.contains(QChar::ReplacementCharacter)) {
        invalid += text.count(QChar::ReplacementCharacter);
    }
    if (invalidChars) {
        *invalidChars = invalid;
    }
    return text;
}

QString TransferCodec::decode(const QByteArray &bytes) const
{
    QList<QByteArray> tryNames;
    if (!m_codecName.isEmpty()) {
        tryNames << m_codecName.toLatin1();
    }
    tryNames << "GBK" << "GB2312" << "UTF-8";

    QSet<QByteArray> seen;
    QString best;
    int bestInvalid = 0x7fffffff;

    foreach (const QByteArray &name, tryNames) {
        if (seen.contains(name)) {
            continue;
        }
        seen.insert(name);

        QTextCodec *textCodec = codecForNameSafe(name);
        int invalid = 0;
        const QString text = decodeWith(textCodec, bytes, &invalid);
        if (invalid == 0) {
            return text;
        }
        if (invalid < bestInvalid) {
            bestInvalid = invalid;
            best = text;
        }
    }

    if (!best.isEmpty()) {
        return best;
    }
    return QString::fromUtf8(bytes);
}

QByteArray TransferCodec::encode(const QString &text) const
{
    return codec()->fromUnicode(text);
}

QString TransferCodec::decodeDatagram(const QByteArray &bytes) const
{
    quint32 command = 0;
    if (ProtocolUtil::commandFromDatagram(bytes, &command)
            && (command & IPMSG_UTF8OPT)) {
        return QString::fromUtf8(bytes.constData(), bytes.size());
    }
    return decode(bytes);
}

QByteArray TransferCodec::encodeForFlags(const QString &text, quint32 flags) const
{
    if (flags & IPMSG_UTF8OPT) {
        return text.toUtf8();
    }
    return encode(text);
}
