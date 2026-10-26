#include "offline_queue.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
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

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QTemporaryDir dir;
    CHECK(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("offline_queue.json"));
    OfflineQueue::setFilePathForTest(path);

    QFile legacy(path);
    CHECK(legacy.open(QIODevice::WriteOnly | QIODevice::Truncate));
    legacy.write("{\"not\":\"array\"}");
    legacy.close();

    OfflineQueue *queue = new OfflineQueue;
    CHECK(queue->count() == 0);
    const QString textId = queue->enqueue(QStringLiteral("10.0.0.8"),
                                           QStringLiteral("hello"),
                                           QStringLiteral("text"),
                                           QStringLiteral("alice"),
                                           QStringLiteral("host-a"));
    const QString shakeId = queue->enqueue(QStringLiteral("10.0.0.8"),
                                            QStringLiteral("#shake#"),
                                            QStringLiteral("shake"),
                                            QStringLiteral("alice"),
                                            QStringLiteral("host-a"));
    CHECK(!textId.isEmpty());
    CHECK(!shakeId.isEmpty());
    CHECK(queue->enqueue(QStringLiteral("10.0.0.8"), QStringLiteral("x"),
                         QStringLiteral("file")).isEmpty());
    CHECK(queue->count() == 2);
    delete queue;

    queue = new OfflineQueue;
    CHECK(queue->count() == 2);
    const QList<OfflineItem> loaded = queue->items();
    CHECK(loaded.size() == 2);
    CHECK(loaded.at(0).kind == QLatin1String("text"));
    CHECK(loaded.at(0).login == QLatin1String("alice"));
    CHECK(loaded.at(0).host == QLatin1String("host-a"));
    CHECK(loaded.at(1).kind == QLatin1String("shake"));
    CHECK(OfflineQueue::previewText(loaded.at(1))
          == QStringLiteral("[Window shake]"));

    QList<OfflinePeerHint> peers;
    OfflinePeerHint stale;
    stale.ip = QStringLiteral("10.0.0.8");
    stale.login = QStringLiteral("alice");
    stale.host = QStringLiteral("host-a");
    stale.reachable = false;
    OfflinePeerHint live;
    live.ip = QStringLiteral("10.0.0.9");
    live.login = QStringLiteral("alice");
    live.host = QStringLiteral("host-a");
    live.reachable = true;
    peers << stale << live;
    CHECK(OfflineQueue::matchReachableIp(QStringLiteral("alice"),
                                          QStringLiteral("host-a"), peers)
          == QLatin1String("10.0.0.9"));
    CHECK(OfflineQueue::matchReachableIp(QString(), QStringLiteral("host-a"), peers)
          .isEmpty());
    // Two fresh 10/8 addresses of one login+host. The lower IPv4 wins
    // so the retarget does not flap, and the item is not left stuck.
    OfflinePeerHint other = live;
    other.ip = QStringLiteral("10.0.0.10");
    peers << other;
    CHECK(OfflineQueue::matchReachableIp(QStringLiteral("alice"),
                                          QStringLiteral("host-a"), peers)
          == QLatin1String("10.0.0.9"));
    QList<OfflinePeerHint> feiq;
    OfflinePeerHint feiqZt;
    feiqZt.ip = QStringLiteral("10.147.19.236");
    feiqZt.login = QStringLiteral("foxwolf");
    feiqZt.host = QStringLiteral("work-laptop");
    feiqZt.reachable = true;
    OfflinePeerHint feiqLan;
    feiqLan.ip = QStringLiteral("192.168.14.164");
    feiqLan.login = QStringLiteral("FoxWolf");
    feiqLan.host = QStringLiteral("WORK-LAPTOP");
    feiqLan.reachable = true;
    feiq << feiqZt << feiqLan;
    CHECK(OfflineQueue::matchReachableIp(QStringLiteral("FoxWolf"),
                                          QStringLiteral("WORK-LAPTOP"), feiq)
          == QLatin1String("192.168.14.164"));

    CHECK(queue->assignIp(textId, QStringLiteral("10.0.0.9")));
    delete queue;
    queue = new OfflineQueue;
    bool moved = false;
    foreach (const OfflineItem &item, queue->items()) {
        if (item.id == textId) {
            moved = item.ip == QLatin1String("10.0.0.9");
        }
    }
    CHECK(moved);

    const QList<OfflineItem> taken = queue->takeForIp(QStringLiteral("10.0.0.9"));
    CHECK(taken.size() == 1);
    CHECK(queue->count() == 1);
    queue->restore(taken);
    CHECK(queue->count() == 2);
    delete queue;
    queue = new OfflineQueue;
    CHECK(queue->count() == 2);
    delete queue;

    QFile file(path);
    CHECK(file.open(QIODevice::ReadOnly));
    CHECK(QJsonDocument::fromJson(file.readAll()).isArray());

    if (g_fails) {
        std::fprintf(stderr, "%d queue checks failed\n", g_fails);
        return 1;
    }
    std::printf("queue checks ok\n");
    return 0;
}
