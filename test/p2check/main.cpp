#include "richText.h"
#include "windowSnap.h"
#include "discussion_store.h"
#include "helper.h"

#include <QApplication>
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

    if (g_fails != 0) {
        std::fprintf(stderr, "%d checks failed\n", g_fails);
        return 1;
    }
    std::printf("p2check ok\n");
    return 0;
}
