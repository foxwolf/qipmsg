#include "fileHashIndex.h"
#include "shareFolder.h"
#include "themeName.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

#include <cstdio>

namespace {

int fail(int code, const char *text)
{
    std::fprintf(stderr, "share check failed (%d): %s\n", code, text);
    return code;
}

bool writeFile(const QString &path, const QByteArray &body)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    return file.write(body) == body.size();
}

} // namespace

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    if (normalizeThemeName(QStringLiteral(" Dark ")) != QLatin1String("dark")) {
        return fail(1, "theme dark");
    }
    if (normalizeThemeName(QStringLiteral("system")) != QLatin1String("system")) {
        return fail(2, "theme system");
    }
    if (normalizeThemeName(QStringLiteral("nope")) != QLatin1String("light")) {
        return fail(3, "theme light");
    }
    if (!ShareFolder::tokenIsQipmsg(QStringLiteral("qipmsg"))
            || !ShareFolder::tokenHasShare(QStringLiteral("qipmsg,share,lock"))
            || !ShareFolder::tokenHasLock(QStringLiteral("qipmsg,share,lock"))
            || ShareFolder::tokenHasShare(QStringLiteral("qipmsg"))
            || ShareFolder::tokenIsQipmsg(QStringLiteral("feiq"))) {
        return fail(4, "token");
    }
    if (!ShareFolder::passwordMatches(QString(), QStringLiteral("x"))
            || !ShareFolder::passwordMatches(QStringLiteral("secret"),
                                             QStringLiteral("secret"))
            || ShareFolder::passwordMatches(QStringLiteral("secret"),
                                            QStringLiteral("Secret"))) {
        return fail(5, "password");
    }

    QTemporaryDir dir;
    if (!dir.isValid()) {
        return fail(6, "temp");
    }
    const QString root = dir.path() + QLatin1String("/share");
    const QString outside = dir.path() + QLatin1String("/outside");
    if (!writeFile(root + QLatin1String("/hello.txt"), "hello-share")
            || !writeFile(root + QLatin1String("/sub/nested.txt"), "nested")
            || !writeFile(outside + QLatin1String("/secret.txt"), "secret")) {
        return fail(7, "fixture");
    }
#ifndef Q_OS_WIN
    QFile::link(outside + QLatin1String("/secret.txt"),
                root + QLatin1String("/leak.txt"));
    QFile::link(outside, root + QLatin1String("/leakdir"));
#endif

    QString abs;
    QString error;
    if (ShareFolder::resolveInside(root, QStringLiteral("../outside/secret.txt"),
                                   &abs, &error)) {
        return fail(8, "traversal");
    }
    if (ShareFolder::resolveInside(root, outside + QLatin1String("/secret.txt"),
                                   &abs, &error)) {
        return fail(9, "absolute");
    }
    if (ShareFolder::resolveInside(root, QStringLiteral("sub/../../outside/secret.txt"),
                                   &abs, &error)) {
        return fail(10, "dotdot");
    }
    if (!ShareFolder::resolveInside(root, QStringLiteral("sub/nested.txt"),
                                    &abs, &error)
            || !abs.endsWith(QLatin1String("nested.txt"))) {
        return fail(11, "nested resolve");
    }
#ifndef Q_OS_WIN
    if (ShareFolder::resolveInside(root, QStringLiteral("leak.txt"), &abs, &error)) {
        return fail(12, "symlink file");
    }
    if (ShareFolder::resolveInside(root, QStringLiteral("leakdir/secret.txt"),
                                   &abs, &error)) {
        return fail(13, "symlink dir");
    }
#endif

    QList<ShareEntry> entries;
    bool truncated = false;
    if (!ShareFolder::listDirectory(root, QString(), &entries, &truncated, &error)) {
        return fail(14, "list");
    }
    bool sawHello = false;
    bool sawLeak = false;
    for (int i = 0; i < entries.size(); ++i) {
        if (entries.at(i).name == QLatin1String("hello.txt")) {
            sawHello = true;
        }
        if (entries.at(i).name.contains(QLatin1String("secret"))
                || entries.at(i).name.startsWith(QLatin1String("leak"))) {
            sawLeak = true;
        }
    }
    if (!sawHello || sawLeak) {
        return fail(15, "list contents");
    }

    const QByteArray encoded = ShareFolder::encodeList(entries, false);
    QList<ShareEntry> decoded;
    if (!ShareFolder::decodeList(encoded, &decoded, &truncated, &error)
            || decoded.size() != entries.size() || truncated) {
        return fail(16, "roundtrip");
    }

    const QByteArray request = ShareFolder::buildRequest(
                QStringLiteral("GET"), QStringLiteral("sub/nested.txt"),
                QStringLiteral("pw"), 0);
    QString verb;
    QString relative;
    QString password;
    qint64 offset = -1;
    if (!ShareFolder::parseRequest(request, &verb, &relative, &password, &offset)
            || verb != QLatin1String("GET")
            || relative != QLatin1String("sub/nested.txt")
            || password != QLatin1String("pw")
            || offset != 0) {
        return fail(17, "request");
    }

    const QByteArray bad = ShareFolder::buildRequest(
                QStringLiteral("GET"), QStringLiteral("../outside/secret.txt"),
                QStringLiteral("pw"), 0);
    const ShareAnswer denied = ShareFolder::answer(
                bad, true, root, QStringLiteral("pw"));
    if (denied.sendFile || !denied.payload.contains("ERR")) {
        return fail(18, "answer traversal");
    }
    const ShareAnswer auth = ShareFolder::answer(request, true, root, QStringLiteral("nope"));
    if (auth.sendFile || !auth.payload.contains("ERR auth")) {
        return fail(19, "answer auth");
    }
    const ShareAnswer ok = ShareFolder::answer(request, true, root, QStringLiteral("pw"));
    if (!ok.sendFile || !ok.filePath.endsWith(QLatin1String("nested.txt"))
            || !ok.payload.startsWith("QIP1\nOK ")) {
        return fail(20, "answer get");
    }
    const ShareAnswer off = ShareFolder::answer(request, false, root, QString());
    if (!off.payload.contains("ERR off")) {
        return fail(21, "answer off");
    }

    const QString indexPath = dir.path() + QLatin1String("/filehash.json");
    FileHashIndex::setStoreFile(indexPath);
    FileHashIndex::setEntryCap(2);
    const QString hello = root + QLatin1String("/hello.txt");
    qint64 size = -1;
    const QString sha = FileHashIndex::hashFile(hello, &size);
    if (sha.size() != 64 || size != 11) {
        return fail(22, "hash");
    }
    FileHashIndex::note(hello, sha, size, QStringLiteral("recv"), QStringLiteral("1.2.3.4"));
    if (FileHashIndex::findExisting(sha, size).isEmpty()) {
        return fail(23, "find");
    }
    if (FileHashIndex::findInDirectory(root, QStringLiteral("hello.txt"), sha, size)
            .isEmpty()) {
        return fail(24, "find dir");
    }
    const QString other = root + QLatin1String("/sub/nested.txt");
    qint64 otherSize = -1;
    const QString otherSha = FileHashIndex::hashFile(other, &otherSize);
    FileHashIndex::note(other, otherSha, otherSize, QStringLiteral("send"), QString());
    const QString third = outside + QLatin1String("/secret.txt");
    qint64 thirdSize = -1;
    const QString thirdSha = FileHashIndex::hashFile(third, &thirdSize);
    FileHashIndex::note(third, thirdSha, thirdSize, QStringLiteral("send"), QString());
    if (FileHashIndex::entryCount() != 2) {
        return fail(25, "cap");
    }
    if (!FileHashIndex::findExisting(sha, size).isEmpty()) {
        return fail(26, "oldest pruned");
    }
    FileHashIndex::setStoreFile(QString());
    FileHashIndex::setEntryCap(2000);

    std::fprintf(stdout, "share check ok\n");
    return 0;
}
