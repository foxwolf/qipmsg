#include "dataBackup.h"
#include "sendThrottle.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

#include <cstdio>

namespace {

void appendU16(QByteArray *out, quint16 value)
{
    out->append(static_cast<char>(value & 0xff));
    out->append(static_cast<char>((value >> 8) & 0xff));
}

void appendU32(QByteArray *out, quint32 value)
{
    appendU16(out, static_cast<quint16>(value & 0xffff));
    appendU16(out, static_cast<quint16>((value >> 16) & 0xffff));
}

quint32 crc32Of(const QByteArray &data)
{
    quint32 crc = 0xffffffffu;
    for (int i = 0; i < data.size(); ++i) {
        crc ^= static_cast<unsigned char>(data.at(i));
        for (int k = 0; k < 8; ++k) {
            if (crc & 1u) {
                crc = 0xedb88320u ^ (crc >> 1);
            } else {
                crc = crc >> 1;
            }
        }
    }
    return crc ^ 0xffffffffu;
}

bool writeStoredZip(const QString &path, const QByteArray &name, const QByteArray &payload)
{
    QByteArray local;
    appendU32(&local, 0x04034b50u);
    appendU16(&local, 20);
    appendU16(&local, 0x0800);
    appendU16(&local, 0);
    appendU16(&local, 0);
    appendU16(&local, 0);
    appendU32(&local, crc32Of(payload));
    appendU32(&local, static_cast<quint32>(payload.size()));
    appendU32(&local, static_cast<quint32>(payload.size()));
    appendU16(&local, static_cast<quint16>(name.size()));
    appendU16(&local, 0);
    local.append(name);

    QByteArray central;
    appendU32(&central, 0x02014b50u);
    appendU16(&central, 20);
    appendU16(&central, 20);
    appendU16(&central, 0x0800);
    appendU16(&central, 0);
    appendU16(&central, 0);
    appendU16(&central, 0);
    appendU32(&central, crc32Of(payload));
    appendU32(&central, static_cast<quint32>(payload.size()));
    appendU32(&central, static_cast<quint32>(payload.size()));
    appendU16(&central, static_cast<quint16>(name.size()));
    appendU16(&central, 0);
    appendU16(&central, 0);
    appendU16(&central, 0);
    appendU16(&central, 0);
    appendU32(&central, 0);
    appendU32(&central, 0);
    central.append(name);

    QByteArray eocd;
    appendU32(&eocd, 0x06054b50u);
    appendU16(&eocd, 0);
    appendU16(&eocd, 0);
    appendU16(&eocd, 1);
    appendU16(&eocd, 1);
    appendU32(&eocd, static_cast<quint32>(central.size()));
    appendU32(&eocd, static_cast<quint32>(local.size() + payload.size()));
    appendU16(&eocd, 0);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    return file.write(local) == local.size()
            && file.write(payload) == payload.size()
            && file.write(central) == central.size()
            && file.write(eocd) == eocd.size();
}

int fail(int code, const char *text)
{
    std::fprintf(stderr, "backup check failed (%d): %s\n", code, text);
    return code;
}

} // namespace

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QTemporaryDir dir;
    if (!dir.isValid()) {
        return fail(1, "temp dir");
    }
    const QString root = dir.path() + QLatin1String("/home");
    QDir().mkpath(root + QLatin1String("/history"));
    QDir().mkpath(root + QLatin1String("/avatars"));
    QDir().mkpath(root + QLatin1String("/recv"));

    QFile ini(root + QLatin1String("/qipmsg.ini"));
    if (!ini.open(QIODevice::WriteOnly)) {
        return fail(2, "ini");
    }
    ini.write("[NickName]\nuserName=demo\n");
    ini.close();

    QFile hist(root + QLatin1String("/history/10.0.0.1.jsonl"));
    if (!hist.open(QIODevice::WriteOnly)) {
        return fail(3, "history");
    }
    hist.write("{\"x\":\"hi\"}\n");
    hist.close();

    QFile peers(root + QLatin1String("/peers.json"));
    if (!peers.open(QIODevice::WriteOnly)) {
        return fail(4, "peers");
    }
    peers.write("[]");
    peers.close();

    QFile junk(root + QLatin1String("/recv/secret.bin"));
    if (!junk.open(QIODevice::WriteOnly)) {
        return fail(5, "recv");
    }
    junk.write("nope");
    junk.close();

    const QString zip = dir.path() + QLatin1String("/out.zip");
    QString error;
    int count = 0;
    if (!DataBackup::exportTree(root, zip, &error, &count)) {
        return fail(6, error.toLocal8Bit().constData());
    }
    if (count != 3) {
        return fail(7, "expected 3 archived files");
    }

    QFile::remove(root + QLatin1String("/qipmsg.ini"));
    QFile::remove(root + QLatin1String("/peers.json"));
    QFile::remove(root + QLatin1String("/history/10.0.0.1.jsonl"));
    QFile stale(root + QLatin1String("/qipmsg.ini"));
    if (!stale.open(QIODevice::WriteOnly)) {
        return fail(8, "stale");
    }
    stale.write("stale");
    stale.close();

    int restored = 0;
    if (!DataBackup::importTree(root, zip, &error, &restored)) {
        return fail(9, error.toLocal8Bit().constData());
    }
    if (restored != 3) {
        return fail(10, "restore count");
    }
    QFile ini2(root + QLatin1String("/qipmsg.ini"));
    if (!ini2.open(QIODevice::ReadOnly)) {
        return fail(11, "reopen ini");
    }
    const QByteArray body = ini2.readAll();
    if (!body.contains("demo")) {
        return fail(12, body.constData());
    }
    if (!QFile::exists(root + QLatin1String("/history/10.0.0.1.jsonl"))) {
        return fail(13, "history missing");
    }
    if (!QFile::exists(root + QLatin1String("/recv/secret.bin"))) {
        return fail(14, "recv file was removed");
    }

    const QString slipZip = dir.path() + QLatin1String("/slip.zip");
    if (!writeStoredZip(slipZip, QByteArray("../evil.txt"), QByteArray("pwn"))) {
        return fail(15, "slip zip");
    }
    const QString evilPath = dir.path() + QLatin1String("/evil.txt");
    QFile::remove(evilPath);
    if (DataBackup::importTree(root, slipZip, &error, 0)) {
        return fail(16, "slip import should fail");
    }
    if (QFile::exists(evilPath)) {
        return fail(17, "zip slip wrote outside the data folder");
    }

    SendThrottle::setLimitKBps(0);
    if (!SendThrottle::pace(100000, []() { return false; })) {
        return fail(18, "unlimited pace");
    }
    SendThrottle::setLimitKBps(102400);
    if (!SendThrottle::pace(4096, []() { return false; })) {
        return fail(19, "burst pace");
    }
    SendThrottle::setLimitKBps(1);
    int checks = 0;
    const bool aborted = !SendThrottle::pace(64 * 1024, [&checks]() {
        return ++checks > 2;
    });
    if (!aborted || checks < 3) {
        return fail(20, "throttle did not observe cancel");
    }
    SendThrottle::setLimitKBps(0);

    std::fprintf(stderr, "backup check ok\n");
    return 0;
}
