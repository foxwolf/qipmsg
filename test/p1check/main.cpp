// Self-check for cross-subnet range tokens and IPMSG option helpers.
// Not a GUI test: it links protocolutil.cpp and Qt Core/Network only.

#include "autostart.h"
#include "protocolutil.h"

#include <QCoreApplication>
#include <QHostAddress>

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

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QString canonical;
    quint32 start = 0;
    quint32 end = 0;

    CHECK(ProtocolUtil::inspectIpv4Token(
              "192.168.2.10", &canonical, &start, &end) == ProtocolUtil::Ipv4TokenOk);
    CHECK(canonical == QLatin1String("192.168.2.10"));
    CHECK(start == end);
    CHECK(ProtocolUtil::expandIpv4Hosts(start, end, 0).size() == 1);

    canonical.clear();
    CHECK(ProtocolUtil::inspectIpv4Token(
              "192.168.2.12 - 192.168.2.10", &canonical, &start, &end)
          == ProtocolUtil::Ipv4TokenOk);
    CHECK(canonical == QLatin1String("192.168.2.10-192.168.2.12"));
    CHECK(ProtocolUtil::expandIpv4Hosts(start, end, 0)
          == (QStringList() << "192.168.2.10" << "192.168.2.11" << "192.168.2.12"));

    canonical.clear();
    CHECK(ProtocolUtil::inspectIpv4Token(
              "192.168.2.15/24", &canonical, &start, &end) == ProtocolUtil::Ipv4TokenOk);
    CHECK(canonical == QLatin1String("192.168.2.0/24"));
    CHECK(QHostAddress(start).toString() == QLatin1String("192.168.2.0"));
    CHECK(QHostAddress(end).toString() == QLatin1String("192.168.2.255"));
    CHECK(ProtocolUtil::expandIpv4Hosts(start, end, 0).size() == 256);

    CHECK(ProtocolUtil::inspectIpv4Token("10.0.0.0/22", &canonical, &start, &end)
          == ProtocolUtil::Ipv4TokenOk);
    CHECK(quint64(end) - quint64(start) + 1 == 1024u);

    CHECK(ProtocolUtil::inspectIpv4Token("10.0.0.0/21", 0, &start, &end)
          == ProtocolUtil::Ipv4TokenTooLarge);
    bool truncated = false;
    const QStringList capped = ProtocolUtil::expandIpv4Hosts(start, end, &truncated);
    CHECK(truncated);
    CHECK(capped.size() == ProtocolUtil::maxIpv4RangeHosts());

    CHECK(ProtocolUtil::inspectIpv4Token("10.1.2.3/33", 0, 0, 0)
          == ProtocolUtil::Ipv4TokenBadPrefix);
    CHECK(ProtocolUtil::inspectIpv4Token("not-an-ip", 0, 0, 0)
          == ProtocolUtil::Ipv4TokenInvalid);
    CHECK(ProtocolUtil::inspectIpv4Token("127.0.0.1", 0, 0, 0)
          == ProtocolUtil::Ipv4TokenUnusable);
    CHECK(ProtocolUtil::inspectIpv4Token("", 0, 0, 0)
          == ProtocolUtil::Ipv4TokenEmpty);

    int invalid = 0;
    int truncatedCount = 0;
    const QList<QHostAddress> targets = ProtocolUtil::targetsFromBroadcastTokens(
                QStringList() << "192.168.2.10-192.168.2.12" << "bogus" << "10.9.8.7",
                &invalid, &truncatedCount);
    CHECK(invalid == 1);
    CHECK(truncatedCount == 0);
    CHECK(targets.size() == 4);

    const quint32 utf8Send = IPMSG_SENDMSG | IPMSG_UTF8OPT;
    QByteArray datagram = QByteArray("1:100:user:host:")
            + QByteArray::number(utf8Send)
            + QByteArray(":hello");
    datagram.append('\0');
    quint32 command = 0;
    CHECK(ProtocolUtil::commandFromDatagram(datagram, &command));
    CHECK((command & IPMSG_UTF8OPT) != 0);
    CHECK(GET_MODE(command) == IPMSG_SENDMSG);

    CHECK(ProtocolUtil::isSecretEnvelope(IPMSG_SENDMSG | IPMSG_SECRETOPT));
    CHECK(ProtocolUtil::isSecretEnvelope(IPMSG_SENDMSG | IPMSG_READCHECKOPT));
    CHECK(!ProtocolUtil::isSecretEnvelope(IPMSG_BR_ENTRY | IPMSG_SERVEROPT));
    CHECK(!ProtocolUtil::isSecretEnvelope(IPMSG_SENDMSG));

    CHECK(ProtocolUtil::readReceiptPacketNo("4321", "99") == QLatin1String("4321"));
    CHECK(ProtocolUtil::readReceiptPacketNo(QString("4321") + QChar('\0') + "extra", "99")
          == QLatin1String("4321"));
    CHECK(ProtocolUtil::readReceiptPacketNo("4321 extra", "99") == QLatin1String("4321"));
    CHECK(ProtocolUtil::readReceiptPacketNo(QString(), "77") == QLatin1String("77"));
    CHECK(ProtocolUtil::readReceiptPacketNo("opened", "77").isEmpty());

    bool typing = false;
    CHECK(ProtocolUtil::isFeiqTypingCommand(IPMSG_FEIQ_TYPING, &typing));
    CHECK(typing);
    CHECK(IPMSG_FEIQ_TYPING == 121u);
    CHECK(ProtocolUtil::isFeiqTypingCommand(IPMSG_FEIQ_TYPINGSTOP, &typing));
    CHECK(!typing);
    CHECK(IPMSG_FEIQ_TYPINGSTOP == 122u);
    CHECK(!ProtocolUtil::isFeiqTypingCommand(IPMSG_SENDMSG, &typing));
    CHECK(!ProtocolUtil::isFeiqTypingCommand(IPMSG_FEIQ_TYPING | IPMSG_UTF8OPT, &typing));
    command = 0;
    CHECK(ProtocolUtil::commandFromDatagram(
            QByteArray("1_lbt6_0#128#AABBCCDDEEFF#0#0#0#4001#9:9:user:host:121:"),
            &command));
    CHECK(command == IPMSG_FEIQ_TYPING);
    CHECK(ProtocolUtil::commandFromDatagram(
            QByteArray("1:9:user:host:122:"), &command));
    CHECK(command == IPMSG_FEIQ_TYPINGSTOP);
    CHECK(ProtocolUtil::etaSeconds(1000, 5000, 1000) == 4);
    CHECK(ProtocolUtil::etaSeconds(0, 5000, 1000) < 0);
    CHECK(ProtocolUtil::etaSeconds(5000, 5000, 1000) < 0);

    const QString desktop = AutoStart::xdgDesktopText("/tmp/my app/qipmsg");
    CHECK(desktop.contains("X-QIpMsg-Autostart=true"));
    CHECK(desktop.contains("X-GNOME-Autostart-enabled=true"));
    CHECK(desktop.contains("Exec=\"/tmp/my app/qipmsg\""));
    CHECK(AutoStart::xdgFileName() == QLatin1String("qipmsg-autostart.desktop"));
    const QString agent = AutoStart::macAgentText("/Applications/QIpMsg.app/Contents/MacOS/qipmsg");
    CHECK(agent.contains("org.qipmsg.login"));
    CHECK(agent.contains("<key>RunAtLoad</key>"));
    CHECK(agent.contains("/Applications/QIpMsg.app/Contents/MacOS/qipmsg"));
    CHECK(AutoStart::windowsValueName() == QLatin1String("QIpMsg"));

    if (g_fails != 0) {
        std::fprintf(stderr, "%d checks failed\n", g_fails);
        return 1;
    }
    std::printf("qipmsgP1Check ok\n");
    return 0;
}
