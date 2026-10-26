#include "portBind.h"
#include "ipmsg.h"

#include <QCoreApplication>
#include <QHostAddress>
#include <QTcpServer>
#include <QThread>
#include <QUdpSocket>

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

static bool listenSoon(QTcpServer *server, const QHostAddress &address, quint16 port)
{
    if (!server) {
        return false;
    }
    for (int attempt = 0; attempt < 10; ++attempt) {
        if (server->listen(address, port)) {
            return true;
        }
        server->close();
        QThread::msleep(50);
    }
    return false;
}

// Windows keeps an exclusive bind busy for a short time after closesocket.
// An immediate second probe can fail even though the port is free a moment
// later (qipmsgPortCheck fail on released.ok / again.ok).
static portListenResult probeWhenReleased(const QHostAddress &address,
                                          quint16 port)
{
    portListenResult result;
    for (int attempt = 0; attempt < 40; ++attempt) {
        result = PortBind::probe(address, port);
        if (result.ok()) {
            return result;
        }
        QThread::msleep(50);
    }
    const QByteArray udpError = result.udp.error.toLocal8Bit();
    const QByteArray tcpError = result.tcp.error.toLocal8Bit();
    std::fprintf(stderr,
                 "probe %u still down: udp ok=%d kind=%d [%s] tcp ok=%d kind=%d [%s]\n",
                 static_cast<unsigned>(port),
                 result.udp.ok ? 1 : 0,
                 static_cast<int>(result.udp.kind),
                 udpError.constData(),
                 result.tcp.ok ? 1 : 0,
                 static_cast<int>(result.tcp.kind),
                 tcpError.constData());
    return result;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    const QHostAddress any(QHostAddress::AnyIPv4);

    CHECK(PortBind::classify(QAbstractSocket::AddressInUseError, QString())
          == portFailKind::inUse);
    CHECK(PortBind::classify(QAbstractSocket::SocketAccessError, QString())
          == portFailKind::accessDenied);
    CHECK(PortBind::classify(QAbstractSocket::SocketAddressNotAvailableError,
                             QString())
          == portFailKind::addressUnavailable);
    CHECK(PortBind::classify(QAbstractSocket::UnknownSocketError,
                             QStringLiteral("An attempt was made to access a socket in a way forbidden by its access permissions."))
          == portFailKind::accessDenied);
    CHECK(PortBind::classify(QAbstractSocket::UnknownSocketError,
                             QStringLiteral("通常每个套接字地址(协议/网络地址/端口)只允许使用一次。"))
          == portFailKind::inUse);
    CHECK(PortBind::classify(QAbstractSocket::UnknownSocketError,
                             QStringLiteral("socket error 10013"))
          == portFailKind::accessDenied);

    const QString sample =
            QStringLiteral("Protocol tcp Port Exclusion Ranges\n\n"
                           "Start Port    End Port\n"
                           "----------    --------\n"
                           "      2425        2524     *\n"
                           "     50000       50059     *\n");
    CHECK(PortBind::textContainsPort(sample, 2425));
    CHECK(PortBind::textContainsPort(sample, 2500));
    CHECK(PortBind::textContainsPort(sample, 2524));
    CHECK(!PortBind::textContainsPort(sample, 2424));
    CHECK(!PortBind::textContainsPort(sample, 2525));
    CHECK(PortBind::textContainsPort(sample, 50000));
    CHECK(!PortBind::textContainsPort(QString(), 2425));
    CHECK(!PortBind::textContainsPort(sample, 0));

    const QList<quint16> backToDefault = PortBind::candidatePorts(12425);
    CHECK(!backToDefault.isEmpty());
    CHECK(backToDefault.first() == IPMSG_DEFAULT_PORT);
    CHECK(!backToDefault.contains(12425));

    const QList<quint16> leaveDefault = PortBind::candidatePorts(IPMSG_DEFAULT_PORT);
    CHECK(!leaveDefault.contains(IPMSG_DEFAULT_PORT));
    CHECK(!leaveDefault.isEmpty());
    CHECK(leaveDefault.first() == 12425);

    portListenResult idle;
    idle.port = IPMSG_DEFAULT_PORT;
    idle.udp.ok = true;
    idle.udp.kind = portFailKind::none;
    idle.tcp.ok = true;
    idle.tcp.kind = portFailKind::none;
    PortBind::annotateExcluded(&idle);
    CHECK(!idle.udp.windowsExcluded);
    CHECK(!idle.tcp.windowsExcluded);

    quint16 taken = 0;
    {
        QTcpServer held;
        CHECK(held.listen(any, 0));
        taken = held.serverPort();
        CHECK(taken != 0);
        const portListenResult blocked = PortBind::probe(any, taken);
        CHECK(!blocked.ok());
        CHECK(!blocked.tcp.ok);
        CHECK(blocked.tcp.kind == portFailKind::inUse);
        held.close();
    }

    const portListenResult released = probeWhenReleased(any, taken);
    CHECK(released.ok());
    const portListenResult again = probeWhenReleased(any, taken);
    CHECK(again.ok());

    QTcpServer rebound;
    CHECK(listenSoon(&rebound, any, taken));
    rebound.close();

    QUdpSocket udpHold;
    CHECK(udpHold.bind(any, 0, QAbstractSocket::DefaultForPlatform));
    const quint16 udpTaken = udpHold.localPort();
    const portListenResult udpBlocked = PortBind::probe(any, udpTaken);
    CHECK(!udpBlocked.udp.ok);
    CHECK(udpBlocked.udp.kind == portFailKind::inUse);
    udpHold.close();

    const quint16 suggested = PortBind::suggestFree(any, taken);
    CHECK(suggested != 0);
    CHECK(suggested != taken);
    const portListenResult suggestedProbe = PortBind::probe(any, suggested);
    CHECK(suggestedProbe.ok());
    QTcpServer suggestedListen;
    CHECK(listenSoon(&suggestedListen, any, suggested));
    suggestedListen.close();

    if (g_fails != 0) {
        std::fprintf(stderr, "%d port check(s) failed\n", g_fails);
        return 1;
    }
    std::printf("port check ok\n");
    return 0;
}
