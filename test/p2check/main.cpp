#include "richText.h"
#include "windowSnap.h"
#include "discussion_store.h"
#include "helper.h"
#include "userSkin.h"
#include "nicBind.h"
#include "themeName.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>

QString Helper::appHomePath()
{
    return QStringLiteral("/tmp/qipmsg-p2-unused");
}

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

static void appendU16(QByteArray *out, quint16 value)
{
    out->append(static_cast<char>(value & 0xff));
    out->append(static_cast<char>((value >> 8) & 0xff));
}

static void appendU32(QByteArray *out, quint32 value)
{
    appendU16(out, static_cast<quint16>(value & 0xffff));
    appendU16(out, static_cast<quint16>((value >> 16) & 0xffff));
}

static QByteArray makeZip(const QList<QPair<QString, QByteArray> > &files,
                          const QList<quint16> &methods,
                          const QList<QByteArray> &stored)
{
    QByteArray local;
    QByteArray central;
    for (int i = 0; i < files.size(); ++i) {
        const QByteArray name = files.at(i).first.toUtf8();
        const QByteArray payload = stored.at(i);
        const quint32 rawSize = static_cast<quint32>(files.at(i).second.size());
        const quint32 compSize = static_cast<quint32>(payload.size());
        const quint32 offset = static_cast<quint32>(local.size());
        appendU32(&local, 0x04034b50u);
        appendU16(&local, 20);
        appendU16(&local, 0);
        appendU16(&local, methods.at(i));
        appendU16(&local, 0);
        appendU16(&local, 0);
        appendU32(&local, 0);
        appendU32(&local, compSize);
        appendU32(&local, rawSize);
        appendU16(&local, static_cast<quint16>(name.size()));
        appendU16(&local, 0);
        local.append(name);
        local.append(payload);

        appendU32(&central, 0x02014b50u);
        appendU16(&central, 20);
        appendU16(&central, 20);
        appendU16(&central, 0);
        appendU16(&central, methods.at(i));
        appendU16(&central, 0);
        appendU16(&central, 0);
        appendU32(&central, 0);
        appendU32(&central, compSize);
        appendU32(&central, rawSize);
        appendU16(&central, static_cast<quint16>(name.size()));
        appendU16(&central, 0);
        appendU16(&central, 0);
        appendU16(&central, 0);
        appendU16(&central, 0);
        appendU32(&central, 0);
        appendU32(&central, offset);
        central.append(name);
    }
    const quint32 cdOffset = static_cast<quint32>(local.size());
    QByteArray out = local + central;
    appendU32(&out, 0x06054b50u);
    appendU16(&out, 0);
    appendU16(&out, 0);
    appendU16(&out, static_cast<quint16>(files.size()));
    appendU16(&out, static_cast<quint16>(files.size()));
    appendU32(&out, static_cast<quint32>(central.size()));
    appendU32(&out, cdOffset);
    appendU16(&out, 0);
    return out;
}

static bool fragmentBold(const QTextDocument &doc)
{
    for (QTextBlock block = doc.begin(); block.isValid(); block = block.next()) {
        for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            if (frag.isValid() && frag.text().contains(QLatin1String("Hi"))
                    && frag.charFormat().fontWeight() >= QFont::Bold) {
                return true;
            }
        }
    }
    return false;
}

int main(int argc, char **argv)
{
#ifdef Q_OS_LINUX
    qputenv("QT_QPA_PLATFORM", "offscreen");
#endif
    QApplication app(argc, argv);

    QTextDocument doc;
    QTextCursor cursor(&doc);
    cursor.insertText(QStringLiteral("Say "));
    QTextCharFormat bold;
    bold.setFontWeight(QFont::Bold);
    cursor.insertText(QStringLiteral("Hi"), bold);
    cursor.insertText(QStringLiteral(" there"));

    const QString html = RichText::canonicalFromDocument(&doc);
    CHECK(RichText::hasFormatting(html));
    CHECK(html.contains(QStringLiteral("<b>")));
    CHECK(!html.contains(QStringLiteral("<script"), Qt::CaseInsensitive));
    CHECK(RichText::samePlain(RichText::plainFromHtml(html), doc.toPlainText()));
    CHECK(RichText::plainFromHtml(html) == QStringLiteral("Say Hi there"));

    const QString body = RichText::sidecarBody(QStringLiteral("42"), html);
    CHECK(body.startsWith(QLatin1String("RICH:42:")));
    QString packetNo;
    QString parsed;
    CHECK(RichText::parseSidecar(body, &packetNo, &parsed));
    CHECK(packetNo == QStringLiteral("42"));
    CHECK(RichText::plainFromHtml(parsed) == QStringLiteral("Say Hi there"));
    CHECK(RichText::hasFormatting(parsed));
    const QString kept = parsed;

    CHECK(!RichText::parseSidecar(
            QStringLiteral("INSTANT:1:2:0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"),
            &packetNo, &parsed));
    CHECK(!RichText::parseSidecar(QStringLiteral("RICH:1:"), &packetNo, &parsed));
    CHECK(RichText::sidecarBody(QStringLiteral("1:2"), html).isEmpty());

    const QString dirty = QStringLiteral(
            "<html><head><script>alert(1)</script><style>b{color:red}</style></head>"
            "<body><b>Hi</b><object data=\"file://x\"></object></body></html>");
    const QString safe = RichText::sanitizeHtml(dirty);
    CHECK(safe.contains(QStringLiteral("Hi")));
    CHECK(!safe.contains(QStringLiteral("alert"), Qt::CaseInsensitive));
    CHECK(!safe.contains(QStringLiteral("object"), Qt::CaseInsensitive));
    CHECK(!safe.contains(QStringLiteral("script"), Qt::CaseInsensitive));
    CHECK(RichText::sanitizeHtml(QStringLiteral("{\\rtf1\\ansi hi}")).isEmpty());
    CHECK(RichText::sanitizeHtml(QString(QChar('\0')) + QStringLiteral("<b>Hi</b>")).isEmpty());

    QTextDocument colored;
    QTextCursor colorCursor(&colored);
    QTextCharFormat red;
    red.setForeground(QColor(QStringLiteral("#FF0000")));
    red.setFontPointSize(16);
    colorCursor.insertText(QStringLiteral("Go"), red);
    const QString colorHtml = RichText::canonicalFromDocument(&colored);
    CHECK(colorHtml.contains(QStringLiteral("#dc2626"), Qt::CaseInsensitive)
          || colorHtml.contains(QStringLiteral("#DC2626")));
    CHECK(colorHtml.contains(QStringLiteral("font-size:16pt")));

    CHECK(RichText::coversPlain(html, QStringLiteral("Say Hi there")));
    CHECK(RichText::coversPlain(html, QStringLiteral("Say Hi there\n[File attachment] a.png")));
    CHECK(!RichText::coversPlain(html, QStringLiteral("other")));
    CHECK(RichText::samePlain(QStringLiteral("Hi\n"), QStringLiteral("Hi")));

    QTextDocument shown;
    QTextCursor shownCursor(&shown);
    QTextCharFormat base;
    base.setProperty(RichText::packetProperty(), QStringLiteral("42"));
    RichText::insertFormatted(shownCursor, kept, base);
    CHECK(shown.toPlainText() == QStringLiteral("Say Hi there"));
    CHECK(fragmentBold(shown));

    QTextDocument plainDoc;
    QTextCursor plainCursor(&plainDoc);
    plainCursor.insertText(QStringLiteral("only plain"));
    const QString plainHtml = RichText::canonicalFromDocument(&plainDoc);
    CHECK(!RichText::hasFormatting(plainHtml));
    CHECK(RichText::sidecarBody(QStringLiteral("7"), plainHtml).isEmpty());

    const QRect raw(10, 10, 90, 70);
    QVector<QRect> targets;
    targets << QRect(0, 0, 200, 100);
    const QRect snapped = WindowSnap::snapSelection(raw, targets, 12, false);
    CHECK(snapped.left() == 0);
    CHECK(snapped.top() == 0);
    CHECK(snapped.right() == raw.right());
    CHECK(WindowSnap::snapSelection(raw, targets, 12, true) == raw.normalized());
    const QRect far(400, 400, 40, 40);
    CHECK(WindowSnap::snapSelection(far, targets, 12, false) == far.normalized());
    const QVector<QRect> bounds = WindowSnap::topLevelRects(QRect(0, 0, 640, 480), 0);
    CHECK(!bounds.isEmpty());
    CHECK(bounds.last() == QRect(0, 0, 640, 480));
    CHECK(WindowSnap::topLevelRects(QRect(), 0).isEmpty());

    QTemporaryDir dir;
    CHECK(dir.isValid());
    DiscussionStore::setStorageDirForTest(dir.path());
    QFile seed(dir.filePath(QStringLiteral("discussions.json")));
    CHECK(seed.open(QIODevice::WriteOnly | QIODevice::Truncate));
    seed.write("[{\"id\":\"9\",\"name\":\"old\",\"members\":["
               "{\"ip\":\"1.1.1.1\",\"name\":\"A\"},"
               "{\"ip\":\"2.2.2.2\",\"name\":\"B\"}]}]");
    seed.close();
    DiscussionStore loaded;
    CHECK(loaded.groupById(QStringLiteral("9")).notice.isEmpty());
    QString noticeDirty = QStringLiteral("hello");
    noticeDirty.append(QChar('\0'));
    noticeDirty.append(QStringLiteral("there"));
    CHECK(loaded.setNotice(QStringLiteral("9"), noticeDirty));
    CHECK(loaded.groupById(QStringLiteral("9")).notice == QStringLiteral("hellothere"));
    CHECK(!loaded.setNotice(QStringLiteral("9"), QStringLiteral("hellothere")));
    CHECK(loaded.setNotice(QStringLiteral("9"), QString(450, QLatin1Char('a'))));
    CHECK(loaded.groupById(QStringLiteral("9")).notice.size() == 400);
    DiscussionStore again;
    CHECK(again.groupById(QStringLiteral("9")).notice.size() == 400);
    CHECK(again.setNotice(QStringLiteral("9"), QString()));
    DiscussionStore cleared;
    CHECK(cleared.groupById(QStringLiteral("9")).notice.isEmpty());
    CHECK(cleared.groupById(QStringLiteral("9")).name == QStringLiteral("old"));
    CHECK(!cleared.setNotice(QStringLiteral("missing"), QStringLiteral("x")));

    const QString shareDir = dir.filePath(QStringLiteral("groupshare"));
    CHECK(QDir().mkpath(shareDir));
    CHECK(cleared.setSharePath(QStringLiteral("9"), shareDir));
    CHECK(cleared.groupById(QStringLiteral("9")).sharePath
          == QDir::cleanPath(shareDir));
    DiscussionStore withShare;
    CHECK(withShare.groupById(QStringLiteral("9")).sharePath
          == QDir::cleanPath(shareDir));
    QFile shareJson(dir.filePath(QStringLiteral("discussions.json")));
    CHECK(shareJson.open(QIODevice::ReadOnly));
    const QByteArray shareBytes = shareJson.readAll();
    shareJson.close();
    CHECK(shareBytes.contains("sharePath"));
    CHECK(withShare.groupById(QStringLiteral("9")).notice.isEmpty());
    CHECK(withShare.setSharePath(QStringLiteral("9"), QString()));
    DiscussionStore shareCleared;
    CHECK(shareCleared.groupById(QStringLiteral("9")).sharePath.isEmpty());
    QFile shareJson2(dir.filePath(QStringLiteral("discussions.json")));
    CHECK(shareJson2.open(QIODevice::ReadOnly));
    CHECK(!QString::fromUtf8(shareJson2.readAll()).contains(QStringLiteral("sharePath")));
    shareJson2.close();
    CHECK(!shareCleared.setSharePath(QStringLiteral("9"),
                                     dir.filePath(QStringLiteral("discussions.json"))));
    CHECK(!shareCleared.setSharePath(QStringLiteral("missing"), shareDir));

    QTemporaryDir skins;
    CHECK(skins.isValid());
    UserSkin::setRootForTest(skins.path());
    const QString ocean = skins.filePath(QStringLiteral("Ocean"));
    CHECK(QDir().mkpath(ocean));
    QFile skinJson(ocean + QLatin1String("/skin.json"));
    CHECK(skinJson.open(QIODevice::WriteOnly | QIODevice::Truncate));
    skinJson.write("{\"name\":\"Ocean\",\"base\":\"dark\",\"qss\":\"theme.qss\","
                   "\"colors\":{\"bubbleIncomingBg\":\"#123456\"}}");
    skinJson.close();
    QFile qssFile(ocean + QLatin1String("/theme.qss"));
    CHECK(qssFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    qssFile.write("QWidget { color: #abcdef; }\n");
    qssFile.close();
    const SkinPack oceanPack = UserSkin::load(QStringLiteral("Ocean"));
    CHECK(oceanPack.found);
    CHECK(oceanPack.base == QLatin1String("dark"));
    CHECK(oceanPack.qssOk);
    CHECK(oceanPack.qss.contains(QStringLiteral("#abcdef")));
    CHECK(oceanPack.bubbleInBg == QColor(QStringLiteral("#123456")));
    CHECK(UserSkin::installed().size() == 1);

    QFile badQss(ocean + QLatin1String("/theme.qss"));
    CHECK(badQss.open(QIODevice::WriteOnly | QIODevice::Truncate));
    badQss.write("QWidget { color: red; url(http://evil) }\n");
    badQss.close();
    const SkinPack unsafe = UserSkin::load(QStringLiteral("Ocean"));
    CHECK(unsafe.found);
    CHECK(!unsafe.qssOk);
    CHECK(!UserSkin::lastProblem().isEmpty());
    CHECK(unsafe.bubbleInBg == QColor(QStringLiteral("#123456")));

    CHECK(!UserSkin::load(QStringLiteral("missing")).found);
    CHECK(normalizeThemeName(QStringLiteral("skin:Ocean")) == QLatin1String("skin:Ocean"));

    const QByteArray qssRaw = QByteArrayLiteral("QWidget { color: #112233; }\n");
    const QByteArray qssZip = QByteArray::fromHex(
            "0b0ccf4c494f2d51a85648cecfc92fb25250363434323236b656a8e50200");
    const QByteArray jsonRaw = QByteArrayLiteral(
            "{\"name\":\"ZipSkin\",\"base\":\"light\",\"qss\":\"theme.qss\"}");
    QList<QPair<QString, QByteArray> > zipFiles;
    zipFiles.append(qMakePair(QStringLiteral("theme.qss"), qssRaw));
    zipFiles.append(qMakePair(QStringLiteral("skin.json"), jsonRaw));
    QList<quint16> methods;
    methods << 8 << 0;
    QList<QByteArray> payloads;
    payloads << qssZip << jsonRaw;
    QFile zipFile(skins.filePath(QStringLiteral("Packed.zip")));
    CHECK(zipFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    CHECK(zipFile.write(makeZip(zipFiles, methods, payloads)) > 0);
    zipFile.close();
    const SkinPack zipped = UserSkin::load(QStringLiteral("Packed"));
    CHECK(zipped.found);
    CHECK(zipped.qssOk);
    CHECK(zipped.qss == QString::fromUtf8(qssRaw));
    CHECK(zipped.name == QStringLiteral("ZipSkin"));

    QList<QPair<QString, QByteArray> > slipFiles;
    slipFiles.append(qMakePair(QStringLiteral("../skin.json"), jsonRaw));
    QList<quint16> slipMethods;
    slipMethods << 0;
    QList<QByteArray> slipPayloads;
    slipPayloads << jsonRaw;
    QFile slipFile(skins.filePath(QStringLiteral("Slip.zip")));
    CHECK(slipFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    CHECK(slipFile.write(makeZip(slipFiles, slipMethods, slipPayloads)) > 0);
    slipFile.close();
    QString slipId;
    QString slipError;
    CHECK(!UserSkin::importZip(slipFile.fileName(), &slipId, &slipError));

    NicRecord first;
    first.name = QStringLiteral("eth0");
    first.humanName = QStringLiteral("Ethernet");
    first.mac = QStringLiteral("AA-BB-CC-DD-EE-FF");
    first.ipv4 = QStringLiteral("10.0.0.8");
    first.broadcast = QStringLiteral("10.0.0.255");
    first.index = 2;
    NicRecord second;
    second.name = QStringLiteral("eth1");
    second.mac = QStringLiteral("11:22:33:44:55:66");
    second.ipv4 = QStringLiteral("10.1.0.8");
    second.broadcast = QStringLiteral("10.1.0.255");
    second.index = 3;
    QList<NicRecord> nics;
    nics << first << second;
    const NicChoice anyNic = NicBind::resolve(QString(), QString(), QString(), nics);
    CHECK(anyNic.any);
    bool includeLimited = false;
    const QStringList every = NicBind::discoveryBroadcasts(anyNic, nics, &includeLimited);
    CHECK(includeLimited);
    CHECK(every.contains(QStringLiteral("10.0.0.255")));
    CHECK(every.contains(QStringLiteral("10.1.0.255")));
    const NicChoice picked = NicBind::resolve(
                QStringLiteral("aa:bb:cc:dd:ee:ff"), QString(),
                QStringLiteral("10.9.9.9"), nics);
    CHECK(picked.found);
    CHECK(!picked.any);
    CHECK(picked.ipv4 == QStringLiteral("10.0.0.8"));
    CHECK(picked.name == QStringLiteral("eth0"));
    CHECK(picked.interfaceId == QStringLiteral("aa:bb:cc:dd:ee:ff"));
    const QStringList only = NicBind::discoveryBroadcasts(picked, nics, &includeLimited);
    CHECK(!includeLimited);
    CHECK(only.size() == 1);
    CHECK(only.at(0) == QStringLiteral("10.0.0.255"));
    const NicChoice absent = NicBind::resolve(
                QStringLiteral("aa:bb:cc:dd:ee:01"), QStringLiteral("eth1"),
                QStringLiteral("10.9.9.9"), nics);
    CHECK(!absent.any);
    CHECK(!absent.found);
    CHECK(absent.ipv4 == QStringLiteral("10.9.9.9"));
    CHECK(NicBind::discoveryBroadcasts(absent, nics, &includeLimited).isEmpty());
    CHECK(NicBind::normalizeMac(QStringLiteral("AABBCCDDEEFF"))
          == QStringLiteral("aa:bb:cc:dd:ee:ff"));
    CHECK(NicBind::interfaceId(QString(), QStringLiteral("eth0"))
          == QStringLiteral("eth0"));
    CHECK(NicBind::platformNote().size() > 8);

    if (g_fails != 0) {
        std::fprintf(stderr, "%d checks failed\n", g_fails);
        return 1;
    }
    std::printf("p2check ok\n");
    return 0;
}
