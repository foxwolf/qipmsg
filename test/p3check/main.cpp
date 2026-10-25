#include "scheduleStore.h"
#include "favoriteStore.h"
#include "protocolutil.h"
#include "helper.h"
#include "ipmsg.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
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
    checkDialup();
    if (g_fails != 0) {
        std::fprintf(stderr, "%d checks failed\n", g_fails);
        return 1;
    }
    std::printf("p3check ok\n");
    return 0;
}
