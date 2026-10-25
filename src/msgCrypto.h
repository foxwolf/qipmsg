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

#ifndef MSGCRYPTO_H
#define MSGCRYPTO_H

#include <QByteArray>
#include <QString>

// IPMSG public-key messages (GETPUBKEY / ANSPUBKEY) and optional SENDMSG
// encryption. Wire layout follows the IPMSG draft: capability hex, then
// exponent-modulus, then RSA-PKCS#1(session key) and PKCS#7 CBC ciphertext.
// The symmetric IV is zero unless a peer sets the packet-number IV bit
// inside the capability word (that bit is not UTF8OPT; UTF8OPT is only
// on the command word). Official IPMsg for Windows and FeiQ were not
// verified; see BUILD.md. Plaintext peers never see ENCRYPTOPT.

class MsgCrypto
{
public:
    static void setStorageDirForTest(const QString &dir);
    static void forgetMemoryForTest();

    static bool cipherSelfCheck();
    static bool ensureLocalKey();
    static bool generateKey(int bits);
    static bool hasLocalKey();
    static QString localAnswerBody();

    static bool rememberPeerAnswer(const QString &ip, const QString &body);
    static bool hasPeerKey(const QString &ip);
    static quint32 peerCaps(const QString &ip);

    static bool sealForPeer(const QString &ip, const QByteArray &plain,
                            QString *envelope);
    static bool sealPlain(const QByteArray &plain, const QString &modHex,
                          const QString &expHex, quint32 peerCaps,
                          QString *envelope);
    static bool openEnvelope(const QString &envelope, QByteArray *plain);

    static bool prepareOutgoing(const QString &ip, QString *additionalInfo,
                                quint32 *flags);
    static bool unwrapIncoming(quint32 flags, QString *additionalInfo,
                               QString *extendedInfo);
    static void requestPeerKey(const QString &ip, quint16 port);
    static void requestReachablePeers();

    static const int kLocalBits;
};

#endif // !MSGCRYPTO_H
