#include "scheduleStore.h"
#include "favoriteStore.h"
#include "protocolutil.h"
#include "helper.h"
#include "ipmsg.h"
#include "winShare.h"
#include "localMsg.h"
#include "zoneStore.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFile>
#include <QLocalSocket>
#include <QTemporaryDir>

#include <cstdio>

static QString g_home;

QString Helper::appHomePath()
{
    return g_home;
}

static int g_fails = 0;

static void check(bool cond, const char *text, int line)
{
    if (!cond) {
        std::fprintf(stderr, "fail %d: %s\n", line, text);
        ++g_fails;
    }
}

#define CHECK(cond) check((cond), #cond, __LINE__)

static void checkSchedule(const QString &root)
{
    QDir().mkpath(root);
    {
        ScheduleStore store;
        const QDateTime past = QDateTime::currentDateTime().addSecs(-120);
        const QDateTime future = QDateTime::currentDateTime().addSecs(3600);
        CHECK(store.add(QString(), past, false, QString(), QString()).isEmpty());
        const QString pastId = store.add(QStringLiteral("stand up"), past,
                                         false, QString(), QString());
        CHECK(!pastId.isEmpty());
        const QString futureId = store.add(QStringLiteral("later"), future,
                                           true, QStringLiteral("10.0.0.8"),
                                           QStringLiteral("lee"));
        CHECK(!futureId.isEmpty());
        const QList<ScheduleReminder> due = store.takeDue(QDateTime::currentDateTime());
        CHECK(due.size() == 1);
        CHECK(due.first().id == pastId);
        CHECK(due.first().text == QStringLiteral("stand up"));
        CHECK(!due.first().notifyPeer);
        CHECK(store.takeDue(QDateTime::currentDateTime()).isEmpty());
        CHECK(store.items().size() == 2);
        bool pastDone = false;
        bool futureWaiting = false;
        const QList<ScheduleReminder> rows = store.items();
        for (int i = 0; i < rows.size(); ++i) {
            if (rows.at(i).id == pastId) {
                pastDone = rows.at(i).done;
            }
            if (rows.at(i).id == futureId) {
                futureWaiting = !rows.at(i).done && rows.at(i).notifyPeer
                        && rows.at(i).peerIp == QStringLiteral("10.0.0.8");
            }
        }
        CHECK(pastDone);
        CHECK(futureWaiting);
        CHECK(store.update(futureId, QStringLiteral("later edited"), past,
                           false, QString(), QString()));
        const QList<ScheduleReminder> again = store.takeDue(QDateTime::currentDateTime());
        CHECK(again.size() == 1);
        CHECK(again.first().text == QStringLiteral("later edited"));
        CHECK(!again.first().notifyPeer);
        CHECK(store.remove(pastId));
        CHECK(store.items().size() == 1);
    }
    {
        ScheduleStore reloaded;
        CHECK(reloaded.items().size() == 1);
        CHECK(reloaded.items().first().done);
        CHECK(reloaded.items().first().text == QStringLiteral("later edited"));
    }
    QFile junk(root + QStringLiteral("/schedule.json"));
    CHECK(junk.open(QIODevice::WriteOnly | QIODevice::Truncate));
    junk.write("{not-json");
    junk.close();
    ScheduleStore broken;
    CHECK(broken.items().isEmpty());
    CHECK(!broken.add(QStringLiteral("after junk"),
                      QDateTime::currentDateTime().addSecs(60),
                      false, QString(), QString()).isEmpty());
    CHECK(broken.items().size() == 1);
}

static void checkFavorites(const QString &root)
{
    QDir().mkpath(root);
    QFile::remove(root + QStringLiteral("/favorites.json"));
    FavoriteStore store;
    CHECK(store.add(QStringLiteral("nope"), QStringLiteral("t"),
                    QStringLiteral("body"), QString(), QString()).isEmpty());
    const QString textId = store.add(QStringLiteral("text"), QString(),
                                     QStringLiteral("hello from chat"),
                                     QStringLiteral("10.1.1.1"),
                                     QStringLiteral("ann"));
    CHECK(!textId.isEmpty());
    const QString fileId = store.add(QStringLiteral("file"),
                                     QStringLiteral("note.txt"),
                                     QStringLiteral("/tmp/note.txt"),
                                     QStringLiteral("10.1.1.1"),
                                     QStringLiteral("ann"));
    CHECK(!fileId.isEmpty());
    const QString noteId = store.add(QStringLiteral("note"),
                                     QStringLiteral("Note about ann"),
                                     QStringLiteral("brings cake"),
                                     QStringLiteral("10.1.1.1"),
                                     QStringLiteral("ann"));
    CHECK(!noteId.isEmpty());
    CHECK(store.search(QStringLiteral("cake")).size() == 1);
    CHECK(store.search(QStringLiteral("ANN")).size() == 3);
    CHECK(store.search(QStringLiteral("missing")).isEmpty());
    CHECK(store.search(QString()).size() == 3);
    CHECK(store.remove(fileId));
    CHECK(store.items().size() == 2);
    FavoriteStore reloaded;
    CHECK(reloaded.items().size() == 2);
    bool sawNote = false;
    const QList<FavoriteEntry> rows = reloaded.items();
    for (int i = 0; i < rows.size(); ++i) {
        if (rows.at(i).id == noteId) {
            sawNote = rows.at(i).kind == QStringLiteral("note")
                    && rows.at(i).body == QStringLiteral("brings cake");
        }
        CHECK(rows.at(i).id != fileId);
    }
    CHECK(sawNote);
}

static LocalPeerRef makePeer(const char *ip, const char *login, const char *host,
                              bool reachable)
{
    LocalPeerRef peer;
    peer.ip = QString::fromLatin1(ip);
    peer.login = QString::fromLatin1(login);
    peer.host = QString::fromLatin1(host);
    peer.reachable = reachable;
    return peer;
}

static void checkShareAddress()
{
    QString normalized;
    CHECK(WinShare::isSafeIpv4(QStringLiteral("10.1.2.3"), &normalized));
    CHECK(normalized == QStringLiteral("10.1.2.3"));
    CHECK(WinShare::isSafeIpv4(QStringLiteral("192.168.0.1"), 0));
    CHECK(WinShare::isSafeIpv4(QStringLiteral("8.8.8.8"), 0));
    CHECK(!WinShare::isSafeIpv4(QStringLiteral("0.0.0.0"), 0));
    CHECK(!WinShare::isSafeIpv4(QStringLiteral("224.0.0.1"), 0));
    CHECK(!WinShare::isSafeIpv4(QStringLiteral("255.255.255.255"), 0));
    CHECK(!WinShare::isSafeIpv4(QStringLiteral("10.01.2.3"), 0));
    CHECK(!WinShare::isSafeIpv4(QStringLiteral("10.1.2.3\\share"), 0));
    CHECK(!WinShare::isSafeIpv4(QStringLiteral("10.1.2.3/share"), 0));
    CHECK(!WinShare::isSafeIpv4(QStringLiteral("office"), 0));
    CHECK(!WinShare::isSafeIpv4(QStringLiteral("10.1.2"), 0));
    CHECK(!WinShare::isSafeIpv4(QStringLiteral("::1"), 0));
    CHECK(!WinShare::isSafeIpv4(QStringLiteral(" 10.1.2.3"), 0));
    CHECK(WinShare::openNeighborhood(QStringLiteral("not-an-ip")).result
          == winShareBadAddress);
    CHECK(WinShare::openNeighborhood(QStringLiteral("10.1.2.3\\x")).result
          == winShareBadAddress);
#ifndef Q_OS_WIN
    CHECK(!WinShare::platformSupports());
    CHECK(WinShare::openNeighborhood(QStringLiteral("10.1.2.3")).result
          == winShareNotWindows);
#else
    CHECK(WinShare::platformSupports());
#endif
}

static void checkLocalPlan()
{
    QList<LocalPeerRef> peers;
    peers.append(makePeer("10.9.8.7", "lee", "Office", true));
    peers.append(makePeer("10.9.8.8", "ann", "Office", false));
    peers.append(makePeer("10.0.0.1", "bob", "Host", true));
    peers.append(makePeer("10.0.0.2", "bob", "Host", false));

    LocalPlan plan = LocalMsg::planDelivery(QStringLiteral("10.9.8.7"),
                                            QStringLiteral("hello"), peers, false);
    CHECK(plan.kind == localPlanSend);
    CHECK(plan.ip == QStringLiteral("10.9.8.7"));
    plan = LocalMsg::planDelivery(QStringLiteral("10.9.8.8"),
                                  QStringLiteral("hello"), peers, false);
    CHECK(plan.kind == localPlanOffline);
    CHECK(LocalMsg::planError(plan) == QStringLiteral("ERR offline"));
    plan = LocalMsg::planDelivery(QStringLiteral("10.9.8.8"),
                                  QStringLiteral("hello"), peers, true);
    CHECK(plan.kind == localPlanQueue);
    CHECK(LocalMsg::planError(plan) == QStringLiteral("OK queued 10.9.8.8"));
    plan = LocalMsg::planDelivery(QStringLiteral("10.1.1.1"),
                                  QStringLiteral("hello"), peers, false);
    CHECK(plan.kind == localPlanSend);
    CHECK(plan.ip == QStringLiteral("10.1.1.1"));
    plan = LocalMsg::planDelivery(QStringLiteral("lee@office"),
                                  QStringLiteral("hello"), peers, false);
    CHECK(plan.kind == localPlanSend);
    CHECK(plan.ip == QStringLiteral("10.9.8.7"));
    plan = LocalMsg::planDelivery(QStringLiteral("Ann@OFFICE"),
                                  QStringLiteral("hello"), peers, false);
    CHECK(plan.kind == localPlanOffline);
    plan = LocalMsg::planDelivery(QStringLiteral("bob@host"),
                                  QStringLiteral("hello"), peers, true);
    CHECK(plan.kind == localPlanAmbiguous);
    plan = LocalMsg::planDelivery(QStringLiteral("nope@host"),
                                  QStringLiteral("hello"), peers, true);
    CHECK(plan.kind == localPlanMissing);
    plan = LocalMsg::planDelivery(QStringLiteral("hello"),
                                  QStringLiteral("x"), peers, true);
    CHECK(plan.kind == localPlanBad);
    plan = LocalMsg::planDelivery(QStringLiteral("10.9.8.7"),
                                  QStringLiteral("  "), peers, true);
    CHECK(plan.kind == localPlanEmpty);
    plan = LocalMsg::planDelivery(QStringLiteral("10.9.8.7"),
                                  QString(QChar(0)), peers, true);
    CHECK(plan.kind == localPlanNul);
    plan = LocalMsg::planDelivery(QStringLiteral("10.9.8.7"),
                                  QString(LocalMsg::maxTextChars + 1, QLatin1Char('a')),
                                  peers, true);
    CHECK(plan.kind == localPlanTooLong);

    QString error;
    const QByteArray encoded = LocalMsg::encodeRequest(
                QStringLiteral("10.0.0.8"), QStringLiteral("line\n2"), &error);
    CHECK(error.isEmpty());
    QByteArray partial = encoded.left(3);
    QString target;
    QString text;
    bool needMore = false;
    CHECK(!LocalMsg::takeRequest(&partial, &target, &text, &error, &needMore));
    CHECK(needMore);
    partial = encoded;
    CHECK(LocalMsg::takeRequest(&partial, &target, &text, &error, &needMore));
    CHECK(target == QStringLiteral("10.0.0.8"));
    CHECK(text == QStringLiteral("line\n2"));
    CHECK(partial.isEmpty());
    CHECK(LocalMsg::encodeRequest(QStringLiteral("bad\n"), QStringLiteral("x"),
                                  &error).isEmpty());

    const QStringList okArgs = QStringList()
            << QStringLiteral("qipmsg")
            << QStringLiteral("--msg")
            << QStringLiteral("lee@office")
            << QStringLiteral("--text")
            << QStringLiteral("hi");
    const LocalCli okCli = LocalMsg::parseCli(okArgs);
    CHECK(okCli.isMsg);
    CHECK(okCli.error.isEmpty());
    CHECK(okCli.target == QStringLiteral("lee@office"));
    CHECK(okCli.text == QStringLiteral("hi"));
    const QStringList badArgs = QStringList()
            << QStringLiteral("qipmsg")
            << QStringLiteral("--msg")
            << QStringLiteral("10.0.0.1");
    CHECK(!LocalMsg::parseCli(badArgs).error.isEmpty());

    const QString channel = LocalMsg::serverName();
    CHECK(channel == LocalMsg::serverName());
    CHECK(channel.startsWith(QStringLiteral("qipmsg-msg-")));
    CHECK(channel.size() == 12 + QStringLiteral("qipmsg-msg-").size());
}

static bool readLocalLine(QLocalSocket *socket, QByteArray *got)
{
    QElapsedTimer clock;
    clock.start();
    while (!got->contains('\n') && clock.elapsed() < 2000) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
        *got += socket->readAll();
    }
    return got->contains('\n');
}

static void checkLocalSocket()
{
    LocalMsgServer server;
    QList<LocalPeerRef> peers;
    peers.append(makePeer("10.9.8.7", "lee", "Office", true));
    server.setHandler([&peers](const QString &target, const QString &text) {
        Q_UNUSED(text);
        const LocalPlan plan = LocalMsg::planDelivery(
                    target, QStringLiteral("ping"), peers, false);
        if (plan.kind == localPlanSend) {
            return QStringLiteral("OK sent ") + plan.ip;
        }
        return LocalMsg::planError(plan);
    });
    QString error;
    const QString name = QStringLiteral("qipmsg-p3-")
            + QString::number(QCoreApplication::applicationPid());
    CHECK(server.start(name, &error));
    if (!error.isEmpty()) {
        std::fprintf(stderr, "listen: %s\n", error.toLocal8Bit().constData());
    }

    QLocalSocket socket;
    socket.connectToServer(name);
    QElapsedTimer clock;
    clock.start();
    while (socket.state() != QLocalSocket::ConnectedState && clock.elapsed() < 2000) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    }
    CHECK(socket.state() == QLocalSocket::ConnectedState);
    const QByteArray payload = LocalMsg::encodeRequest(
                QStringLiteral("lee@office"), QStringLiteral("hello"), &error);
    CHECK(error.isEmpty());
    CHECK(socket.write(payload) == payload.size());
    socket.flush();
    QByteArray got;
    CHECK(readLocalLine(&socket, &got));
    CHECK(QString::fromUtf8(got).startsWith(QStringLiteral("OK sent 10.9.8.7")));
}

static void checkZone(const QString &root)
{
    QDir().mkpath(root);
    ZoneStore store(root);
    QString error;
    CHECK(store.ensureRoot(&error));
    CHECK(store.createNote(QString(), &error).isEmpty());
    const QString first = store.createNote(QStringLiteral("../secret"), &error);
    CHECK(error.isEmpty());
    CHECK(!first.isEmpty());
    CHECK(!first.contains(QLatin1Char('/')));
    CHECK(!first.contains(QLatin1String("..")));
    CHECK(store.noteNames().contains(first));
    const QString second = store.createNote(QStringLiteral("会议"), &error);
    CHECK(!second.isEmpty());
    CHECK(second.endsWith(QLatin1String(".md")));
    CHECK(store.writeNote(first, QStringLiteral("hello\nworld"), &error));
    CHECK(store.readNote(first, &error) == QStringLiteral("hello\nworld"));
    CHECK(!store.writeNote(QStringLiteral("../x.md"), QStringLiteral("no"), &error));
    CHECK(!store.removeNote(QStringLiteral("..\\x.md"), &error));
    CHECK(!store.writeNote(QStringLiteral("note.exe"), QStringLiteral("no"), &error));
    const QString outside = QFileInfo(root).absolutePath() + QStringLiteral("/outside.txt");
    QFile outsideFile(outside);
    CHECK(outsideFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    outsideFile.write("x");
    outsideFile.close();
    const QString linkName = QStringLiteral("escape.md");
    const QString linkPath = QDir(root).filePath(linkName);
    QFile::remove(linkPath);
    if (QFile::link(outside, linkPath) && QFileInfo(linkPath).isSymLink()) {
        CHECK(store.readNote(linkName, &error).isEmpty());
        CHECK(!error.isEmpty());
        CHECK(!store.removeNote(linkName, &error));
        QFile::remove(linkPath);
    }
    CHECK(store.removeNote(first, &error));
    CHECK(!store.noteNames().contains(first));
    CHECK(store.noteNames().contains(second));
    const QString huge(ZoneStore::maxNoteBytes + 1, QLatin1Char('a'));
    CHECK(!store.writeNote(second, huge, &error));
}

static void checkDialup()
{
    CHECK(ProtocolUtil::isDialupOption(IPMSG_DIALUPOPT));
    CHECK(ProtocolUtil::isDialupOption(IPMSG_BR_ENTRY | IPMSG_DIALUPOPT));
    CHECK(ProtocolUtil::isDialupOption(IPMSG_DIALUPOPT | IPMSG_ABSENCEOPT));
    CHECK(!ProtocolUtil::isDialupOption(IPMSG_BR_ENTRY));
    CHECK(!ProtocolUtil::isDialupOption(IPMSG_ABSENCEOPT));
    CHECK(!ProtocolUtil::isDialupOption(IPMSG_FILEATTACHOPT));
    CHECK(IPMSG_DIALUPOPT == 0x00010000UL);
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QTemporaryDir dir;
    CHECK(dir.isValid());
    g_home = dir.path();
    checkSchedule(dir.path());
    checkFavorites(dir.path());
    checkShareAddress();
    checkLocalPlan();
    checkLocalSocket();
    checkZone(dir.path() + QStringLiteral("/zone"));
    checkDialup();
    if (g_fails != 0) {
        std::fprintf(stderr, "%d checks failed\n", g_fails);
        return 1;
    }
    std::printf("p3check ok\n");
    return 0;
}
