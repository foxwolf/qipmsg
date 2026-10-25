#include "ipmsg.h"
#include "msgCrypto.h"

#include <QCoreApplication>
#include <QFile>
#include <QRegularExpression>
#include <QTemporaryDir>

#include <cstdio>

static int g_fails = 0;

static void check(bool cond, const char *text, int line)
{
    if (!cond) {
        std::fprintf(stderr, "fail %d: %s\n", line, text);
        ++g_fails;
    }
}

#define CHECK(cond) check((cond), #cond, __LINE__)

static QString capsOnly(quint32 caps, const QString &answer)
{
    const int colon = answer.indexOf(QLatin1Char(':'));
    return QString::number(caps, 16) + answer.mid(colon);
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    CHECK(MsgCrypto::cipherSelfCheck());

    QTemporaryDir dir;
    CHECK(dir.isValid());
    MsgCrypto::setStorageDirForTest(dir.path());
    CHECK(MsgCrypto::generateKey(512));
    CHECK(MsgCrypto::hasLocalKey());

    const QString answer = MsgCrypto::localAnswerBody();
    const QRegularExpression shape(
            QStringLiteral("^[0-9a-f]+:[0-9a-f]+-[0-9a-f]+$"));
    CHECK(shape.match(answer).hasMatch());
    CHECK(answer.contains(QLatin1Char('-')));

    MsgCrypto::forgetMemoryForTest();
    CHECK(QFile::exists(dir.filePath(QStringLiteral("rsa_private.json"))));
    CHECK(MsgCrypto::ensureLocalKey());
    CHECK(MsgCrypto::localAnswerBody() == answer);

    const QByteArray plain("hello\0queue", 11);
    CHECK(MsgCrypto::rememberPeerAnswer(QStringLiteral("10.1.1.1"), answer));
    CHECK(MsgCrypto::hasPeerKey(QStringLiteral("10.1.1.1")));
    QString envelope;
    CHECK(MsgCrypto::sealForPeer(QStringLiteral("10.1.1.1"), plain, &envelope));
    QByteArray opened;
    CHECK(MsgCrypto::openEnvelope(envelope, &opened));
    CHECK(opened == plain);

    QString extra = envelope + QStringLiteral(":ab");
    QByteArray openedExtra;
    CHECK(MsgCrypto::openEnvelope(extra, &openedExtra));
    CHECK(openedExtra == plain);

    QString tampered = envelope;
    tampered.chop(1);
    tampered.append(tampered.endsWith(QLatin1Char('0'))
                    ? QLatin1Char('1') : QLatin1Char('0'));
    QByteArray junk;
    CHECK(!MsgCrypto::openEnvelope(tampered, &junk));

    CHECK(!MsgCrypto::rememberPeerAnswer(QStringLiteral("10.1.1.2"), QStringLiteral("zz")));
    CHECK(!MsgCrypto::rememberPeerAnswer(QStringLiteral("10.1.1.2"),
                                          QStringLiteral("2:11-abc")));
    CHECK(!MsgCrypto::rememberPeerAnswer(
            QStringLiteral("10.1.1.2"),
            QStringLiteral("2:010001-00112233445566778899aabbccddeeff")));

    const QString bfAnswer = capsOnly(IPMSG_RSA_1024 | IPMSG_BLOWFISH_128, answer);
    CHECK(MsgCrypto::rememberPeerAnswer(QStringLiteral("10.2.2.2"), bfAnswer));
    CHECK((MsgCrypto::peerCaps(QStringLiteral("10.2.2.2")) & IPMSG_AES_256) == 0);
    CHECK((MsgCrypto::peerCaps(QStringLiteral("10.2.2.2")) & IPMSG_BLOWFISH_128) != 0);
    QString bfEnv;
    CHECK(MsgCrypto::sealForPeer(QStringLiteral("10.2.2.2"), plain, &bfEnv));
    CHECK(bfEnv.startsWith(QStringLiteral("20002:"))
          || bfEnv.startsWith(QStringLiteral("00020002:")));
    QByteArray bfOpened;
    CHECK(MsgCrypto::openEnvelope(bfEnv, &bfOpened));
    CHECK(bfOpened == plain);

    const QString bare = capsOnly(IPMSG_RSA_1024, answer);
    CHECK(MsgCrypto::rememberPeerAnswer(QStringLiteral("10.3.3.3"), bare));
    QString none;
    CHECK(!MsgCrypto::sealForPeer(QStringLiteral("10.3.3.3"), plain, &none));

    MsgCrypto::forgetMemoryForTest();
    CHECK(QFile::remove(dir.filePath(QStringLiteral("rsa_private.json"))));
    QByteArray locked;
    CHECK(!MsgCrypto::openEnvelope(envelope, &locked));

    CHECK(MsgCrypto::generateKey(512));
    QByteArray wrong;
    CHECK(!MsgCrypto::openEnvelope(envelope, &wrong));

    MsgCrypto::forgetMemoryForTest();
    CHECK(MsgCrypto::hasPeerKey(QStringLiteral("10.1.1.1")));

    if (g_fails) {
        std::fprintf(stderr, "%d crypto checks failed\n", g_fails);
        return 1;
    }
    std::printf("crypto checks ok\n");
    return 0;
}
