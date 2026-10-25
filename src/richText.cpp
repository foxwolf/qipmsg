// This file is part of QIpMsg.
//
// QIpMsg is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// QIpMsg is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with QIpMsg.  If not, see <http://www.gnu.org/licenses/>.
//

#include "richText.h"

#include <QAction>
#include <QColor>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QMenu>
#include <QMimeData>
#include <QPalette>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>
#include <QToolButton>

namespace {

const int kMaxHtmlBytes = 3500;
const char *kColorHex[] = {
    "#DC2626", "#2563EB", "#16A34A", "#D97706", "#7C3AED"
};

QString normalizeNewlines(QString text)
{
    text.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    text.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    text.remove(QChar(QChar::ObjectReplacementCharacter));
    text.remove(QChar('\0'));
    return text;
}

QString escapeHtml(const QString &text)
{
    QString out;
    out.reserve(text.size());
    for (int i = 0; i < text.size(); ++i) {
        const QChar c = text.at(i);
        switch (c.unicode()) {
        case '&':
            out += QStringLiteral("&amp;");
            break;
        case '<':
            out += QStringLiteral("&lt;");
            break;
        case '>':
            out += QStringLiteral("&gt;");
            break;
        case '"':
            out += QStringLiteral("&quot;");
            break;
        default:
            if (c.unicode() < 32 && c != QLatin1Char('\t')) {
                break;
            }
            if (c == QChar(QChar::ObjectReplacementCharacter)) {
                break;
            }
            out += c;
            break;
        }
    }
    return out;
}

int snapPointSize(qreal pointSize)
{
    if (qAbs(pointSize - 9.0) < 0.6) {
        return 9;
    }
    if (qAbs(pointSize - 16.0) < 0.6) {
        return 16;
    }
    return 0;
}

QColor snapColor(const QColor &color)
{
    if (!color.isValid()) {
        return QColor();
    }
    const QColor rgb = color.toRgb();
    int best = -1;
    int bestDist = 181;
    for (int i = 0; i < 5; ++i) {
        const QColor preset = QColor(QLatin1String(kColorHex[i]));
        const int dist = qAbs(rgb.red() - preset.red())
                + qAbs(rgb.green() - preset.green())
                + qAbs(rgb.blue() - preset.blue());
        if (dist < bestDist) {
            bestDist = dist;
            best = i;
        }
    }
    if (best < 0) {
        return QColor();
    }
    return QColor(QLatin1String(kColorHex[best]));
}

QString wrapFragment(const QString &escaped, const QTextCharFormat &fmt)
{
    QString style;
    if (fmt.hasProperty(QTextFormat::FontPointSize)) {
        const int pointSize = snapPointSize(fmt.fontPointSize());
        if (pointSize > 0) {
            style += QStringLiteral("font-size:%1pt;").arg(pointSize);
        }
    }
    if (fmt.hasProperty(QTextFormat::ForegroundBrush)
            && fmt.foreground().style() != Qt::NoBrush) {
        const QColor color = snapColor(fmt.foreground().color());
        if (color.isValid()) {
            style += QStringLiteral("color:%1;").arg(color.name(QColor::HexRgb));
        }
    }

    QString body = escaped;
    if (!style.isEmpty()) {
        body = QStringLiteral("<span style=\"%1\">%2</span>").arg(style, body);
    }
    if (fmt.fontUnderline()) {
        body = QStringLiteral("<u>%1</u>").arg(body);
    }
    if (fmt.fontItalic()) {
        body = QStringLiteral("<i>%1</i>").arg(body);
    }
    if (fmt.fontWeight() >= QFont::Bold) {
        body = QStringLiteral("<b>%1</b>").arg(body);
    }
    return body;
}

QString stripActiveContent(QString html)
{
    const QRegularExpression block(
            QStringLiteral("<\\s*(script|style|iframe|object|embed|applet|meta|link)\\b[^>]*>.*?</\\s*\\1\\s*>"),
            QRegularExpression::CaseInsensitiveOption
            | QRegularExpression::DotMatchesEverythingOption);
    html.remove(block);
    const QRegularExpression leftover(
            QStringLiteral("<\\s*/?\\s*(script|style|iframe|object|embed|applet|meta|link)\\b[^>]*>"),
            QRegularExpression::CaseInsensitiveOption);
    html.remove(leftover);
    return html;
}

struct RichStyle {
    bool bold;
    bool italic;
    bool underline;
    int pointSize;
    QColor color;

    RichStyle()
        : bold(false), italic(false), underline(false), pointSize(0) {}
};

QTextCharFormat formatFor(const RichStyle &style, const QTextCharFormat &base)
{
    QTextCharFormat fmt = base;
    if (style.bold) {
        fmt.setFontWeight(QFont::Bold);
    }
    if (style.italic) {
        fmt.setFontItalic(true);
    }
    if (style.underline) {
        fmt.setFontUnderline(true);
    }
    if (style.pointSize == 9 || style.pointSize == 16) {
        fmt.setFontPointSize(style.pointSize);
    }
    if (style.color.isValid()) {
        fmt.setForeground(style.color);
    }
    return fmt;
}

template <typename TextFn, typename BreakFn>
void walkCanonical(const QString &html, TextFn onText, BreakFn onBreak)
{
    QList<RichStyle> stack;
    stack.append(RichStyle());
    QString token;
    auto flush = [&]() {
        if (token.isEmpty() || stack.isEmpty()) {
            token.clear();
            return;
        }
        onText(token, stack.last());
        token.clear();
    };

    for (int i = 0; i < html.size();) {
        if (html.at(i) != QLatin1Char('<')) {
            if (html.mid(i, 5) == QLatin1String("&amp;")) {
                token += QLatin1Char('&');
                i += 5;
                continue;
            }
            if (html.mid(i, 4) == QLatin1String("&lt;")) {
                token += QLatin1Char('<');
                i += 4;
                continue;
            }
            if (html.mid(i, 4) == QLatin1String("&gt;")) {
                token += QLatin1Char('>');
                i += 4;
                continue;
            }
            if (html.mid(i, 6) == QLatin1String("&quot;")) {
                token += QLatin1Char('"');
                i += 6;
                continue;
            }
            token += html.at(i);
            ++i;
            continue;
        }

        flush();
        const int end = html.indexOf(QLatin1Char('>'), i);
        if (end < 0) {
            break;
        }
        QString tag = html.mid(i + 1, end - i - 1).trimmed();
        i = end + 1;
        if (tag.endsWith(QLatin1Char('/'))) {
            tag.chop(1);
            tag = tag.trimmed();
        }
        const bool closing = tag.startsWith(QLatin1Char('/'));
        QString name = closing ? tag.mid(1).trimmed() : tag;
        QString attrs;
        const int space = name.indexOf(QLatin1Char(' '));
        if (space >= 0) {
            attrs = name.mid(space + 1);
            name = name.left(space);
        }
        name = name.toLower();
        if (name == QLatin1String("br")) {
            onBreak();
            continue;
        }
        if (closing) {
            if (stack.size() > 1) {
                stack.removeLast();
            }
            continue;
        }
        RichStyle next = stack.last();
        if (name == QLatin1String("b") || name == QLatin1String("strong")) {
            next.bold = true;
        } else if (name == QLatin1String("i") || name == QLatin1String("em")) {
            next.italic = true;
        } else if (name == QLatin1String("u")) {
            next.underline = true;
        } else if (name == QLatin1String("span")) {
            const QRegularExpression colorRe(
                    QStringLiteral("color\\s*:\\s*(#[0-9A-Fa-f]{6})"));
            const QRegularExpression sizeRe(
                    QStringLiteral("font-size\\s*:\\s*(\\d+)"));
            const QRegularExpressionMatch colorMatch = colorRe.match(attrs);
            if (colorMatch.hasMatch()) {
                const QColor snapped = snapColor(QColor(colorMatch.captured(1)));
                if (snapped.isValid()) {
                    next.color = snapped;
                }
            }
            const QRegularExpressionMatch sizeMatch = sizeRe.match(attrs);
            if (sizeMatch.hasMatch()) {
                const int pointSize = snapPointSize(sizeMatch.captured(1).toDouble());
                if (pointSize > 0) {
                    next.pointSize = pointSize;
                }
            }
        }
        stack.append(next);
    }
    flush();
}

bool looksLikeImagePath(const QString &path)
{
    const QString ext = QFileInfo(path).suffix().toLower();
    return ext == QLatin1String("png")
            || ext == QLatin1String("jpg")
            || ext == QLatin1String("jpeg")
            || ext == QLatin1String("gif")
            || ext == QLatin1String("bmp")
            || ext == QLatin1String("webp");
}

QString loosePlain(QString text)
{
    text = normalizeNewlines(text);
    while (text.endsWith(QLatin1Char('\n'))) {
        text.chop(1);
    }
    return text;
}

} // namespace

int RichText::packetProperty()
{
    return QTextFormat::UserProperty + 22;
}

QString RichText::canonicalFromDocument(const QTextDocument *doc)
{
    if (!doc) {
        return QString();
    }
    QString out;
    bool firstBlock = true;
    for (QTextBlock block = doc->begin(); block.isValid(); block = block.next()) {
        if (!firstBlock) {
            out += QStringLiteral("<br/>");
        }
        firstBlock = false;
        for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            if (!frag.isValid()) {
                continue;
            }
            if (frag.charFormat().isImageFormat()) {
                continue;
            }
            const QString text = frag.text();
            if (text.isEmpty()) {
                continue;
            }
            out += wrapFragment(escapeHtml(text), frag.charFormat());
        }
    }
    if (out.toUtf8().size() > kMaxHtmlBytes) {
        return QString();
    }
    return out;
}

QString RichText::sanitizeHtml(const QString &html)
{
    const QString trimmed = html.trimmed();
    if (trimmed.isEmpty() || trimmed.contains(QChar('\0'))) {
        return QString();
    }
    if (trimmed.startsWith(QStringLiteral("{\\rtf"), Qt::CaseInsensitive)) {
        return QString();
    }
    QTextDocument doc;
    doc.setHtml(stripActiveContent(html));
    const QString canonical = canonicalFromDocument(&doc);
    if (plainFromHtml(canonical).trimmed().isEmpty()) {
        return QString();
    }
    return canonical;
}

QString RichText::plainFromHtml(const QString &html)
{
    QString out;
    walkCanonical(html,
                  [&out](const QString &text, const RichStyle &) { out += text; },
                  [&out]() { out += QLatin1Char('\n'); });
    return normalizeNewlines(out);
}

bool RichText::hasFormatting(const QString &canonicalHtml)
{
    return canonicalHtml.contains(QStringLiteral("<b>"), Qt::CaseInsensitive)
            || canonicalHtml.contains(QStringLiteral("<i>"), Qt::CaseInsensitive)
            || canonicalHtml.contains(QStringLiteral("<u>"), Qt::CaseInsensitive)
            || canonicalHtml.contains(QStringLiteral("<span"), Qt::CaseInsensitive);
}

bool RichText::samePlain(const QString &left, const QString &right)
{
    return loosePlain(left) == loosePlain(right);
}

bool RichText::coversPlain(const QString &html, const QString &plain)
{
    const QString body = loosePlain(plainFromHtml(html));
    const QString text = loosePlain(plain);
    if (body.isEmpty()) {
        return false;
    }
    if (text == body) {
        return true;
    }
    return text.startsWith(body + QLatin1Char('\n'));
}

QString RichText::sidecarBody(const QString &packetNo, const QString &html)
{
    if (packetNo.isEmpty() || packetNo.contains(QLatin1Char(':'))
            || html.isEmpty() || !hasFormatting(html)) {
        return QString();
    }
    const QByteArray utf8 = html.toUtf8();
    if (utf8.isEmpty() || utf8.size() > kMaxHtmlBytes || utf8.contains('\0')) {
        return QString();
    }
    return QStringLiteral("RICH:%1:%2")
            .arg(packetNo, QString::fromLatin1(utf8.toBase64()));
}

bool RichText::parseSidecar(const QString &body, QString *packetNo, QString *html)
{
    if (packetNo) {
        packetNo->clear();
    }
    if (html) {
        html->clear();
    }
    if (!body.startsWith(QLatin1String("RICH:"))) {
        return false;
    }
    const int second = body.indexOf(QLatin1Char(':'), 5);
    if (second <= 5) {
        return false;
    }
    const QString pkt = body.mid(5, second - 5);
    if (pkt.isEmpty() || pkt.contains(QLatin1Char(':'))) {
        return false;
    }
    const QByteArray raw = QByteArray::fromBase64(body.mid(second + 1).toLatin1());
    if (raw.isEmpty() || raw.size() > kMaxHtmlBytes || raw.contains('\0')) {
        return false;
    }
    const QString safe = sanitizeHtml(QString::fromUtf8(raw));
    if (!hasFormatting(safe)) {
        return false;
    }
    if (packetNo) {
        *packetNo = pkt;
    }
    if (html) {
        *html = safe;
    }
    return true;
}

void RichText::insertFormatted(QTextCursor &cursor, const QString &html,
                               const QTextCharFormat &base)
{
    walkCanonical(html,
                  [&cursor, &base](const QString &text, const RichStyle &style) {
        cursor.insertText(text, formatFor(style, base));
    },
                  [&cursor, &base]() {
        cursor.insertText(QStringLiteral("\n"), base);
    });
}

RichComposer::RichComposer(QWidget *parent)
    : QTextEdit(parent)
{
    setAcceptRichText(true);
    setAcceptDrops(false);
}

bool RichComposer::canInsertFromMimeData(const QMimeData *source) const
{
    if (!source) {
        return false;
    }
    if (source->hasImage() || source->hasText() || source->hasHtml()) {
        return true;
    }
    if (!source->hasUrls()) {
        return false;
    }
    foreach (const QUrl &url, source->urls()) {
        if (looksLikeImagePath(url.toLocalFile())) {
            return true;
        }
    }
    return false;
}

void RichComposer::insertFromMimeData(const QMimeData *source)
{
    if (!source) {
        return;
    }
    if (source->hasImage()) {
        const QPixmap pixmap = qvariant_cast<QPixmap>(source->imageData());
        if (!pixmap.isNull()) {
            emit imagePasted(pixmap);
        }
        return;
    }
    if (source->hasUrls()) {
        QStringList images;
        foreach (const QUrl &url, source->urls()) {
            const QString path = url.toLocalFile();
            if (!path.isEmpty() && looksLikeImagePath(path)) {
                images << path;
            }
        }
        if (!images.isEmpty()) {
            emit imageFilesPasted(images);
            return;
        }
    }
    if (source->hasHtml()) {
        const QString safe = RichText::sanitizeHtml(source->html());
        if (!safe.isEmpty() && RichText::hasFormatting(safe)) {
            const bool textAgrees = !source->hasText()
                    || RichText::samePlain(RichText::plainFromHtml(safe), source->text());
            if (textAgrees) {
                textCursor().insertHtml(safe);
                return;
            }
        }
    }
    if (source->hasText()) {
        QString text = source->text();
        text.remove(QChar('\0'));
        textCursor().insertText(text);
    }
}

RichComposerBar::RichComposerBar(QTextEdit *edit, QWidget *parent)
    : QWidget(parent), m_edit(edit), m_boldButton(0), m_italicButton(0),
      m_underlineButton(0)
{
    setObjectName(QStringLiteral("richToolBar"));
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);

    m_boldButton = new QToolButton;
    m_boldButton->setText(QStringLiteral("B"));
    m_boldButton->setCheckable(true);
    m_boldButton->setAutoRaise(true);
    m_boldButton->setToolTip(tr("Bold"));
    QFont boldFont = m_boldButton->font();
    boldFont.setBold(true);
    m_boldButton->setFont(boldFont);

    m_italicButton = new QToolButton;
    m_italicButton->setText(QStringLiteral("I"));
    m_italicButton->setCheckable(true);
    m_italicButton->setAutoRaise(true);
    m_italicButton->setToolTip(tr("Italic"));
    QFont italicFont = m_italicButton->font();
    italicFont.setItalic(true);
    m_italicButton->setFont(italicFont);

    m_underlineButton = new QToolButton;
    m_underlineButton->setText(QStringLiteral("U"));
    m_underlineButton->setCheckable(true);
    m_underlineButton->setAutoRaise(true);
    m_underlineButton->setToolTip(tr("Underline"));
    QFont underFont = m_underlineButton->font();
    underFont.setUnderline(true);
    m_underlineButton->setFont(underFont);

    QToolButton *sizeButton = new QToolButton;
    sizeButton->setText(tr("Size"));
    sizeButton->setAutoRaise(true);
    sizeButton->setPopupMode(QToolButton::InstantPopup);
    sizeButton->setToolTip(tr("Font size"));
    QMenu *sizeMenu = new QMenu(sizeButton);
    sizeMenu->addAction(tr("Small"), this, SLOT(applySmall()));
    sizeMenu->addAction(tr("Normal"), this, SLOT(applyNormal()));
    sizeMenu->addAction(tr("Large"), this, SLOT(applyLarge()));
    sizeButton->setMenu(sizeMenu);

    QToolButton *colorButton = new QToolButton;
    colorButton->setText(tr("Color"));
    colorButton->setAutoRaise(true);
    colorButton->setPopupMode(QToolButton::InstantPopup);
    colorButton->setToolTip(tr("Font color"));
    QMenu *colorMenu = new QMenu(colorButton);
    const QStringList colorNames = QStringList()
            << tr("Red") << tr("Blue") << tr("Green")
            << tr("Orange") << tr("Purple");
    for (int i = 0; i < 5; ++i) {
        QAction *action = colorMenu->addAction(colorNames.at(i));
        action->setData(QColor(QLatin1String(kColorHex[i])));
        connect(action, SIGNAL(triggered()), this, SLOT(applyColor()));
    }
    colorMenu->addAction(tr("Default color"), this, SLOT(clearColor()));
    colorButton->setMenu(colorMenu);

    layout->addWidget(m_boldButton);
    layout->addWidget(m_italicButton);
    layout->addWidget(m_underlineButton);
    layout->addWidget(sizeButton);
    layout->addWidget(colorButton);
    layout->addStretch();

    connect(m_boldButton, SIGNAL(clicked()), this, SLOT(toggleBold()));
    connect(m_italicButton, SIGNAL(clicked()), this, SLOT(toggleItalic()));
    connect(m_underlineButton, SIGNAL(clicked()), this, SLOT(toggleUnderline()));
    if (m_edit) {
        connect(m_edit, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
                this, SLOT(syncState()));
        connect(m_edit, SIGNAL(cursorPositionChanged()),
                this, SLOT(syncState()));
    }
}

void RichComposerBar::syncState()
{
    if (!m_edit) {
        return;
    }
    const QTextCharFormat fmt = m_edit->currentCharFormat();
    m_boldButton->blockSignals(true);
    m_italicButton->blockSignals(true);
    m_underlineButton->blockSignals(true);
    m_boldButton->setChecked(fmt.fontWeight() >= QFont::Bold);
    m_italicButton->setChecked(fmt.fontItalic());
    m_underlineButton->setChecked(fmt.fontUnderline());
    m_boldButton->blockSignals(false);
    m_italicButton->blockSignals(false);
    m_underlineButton->blockSignals(false);
}

void RichComposerBar::mergeFormat(const QTextCharFormat &fmt)
{
    if (!m_edit) {
        return;
    }
    m_edit->mergeCurrentCharFormat(fmt);
    m_edit->setFocus();
}

void RichComposerBar::toggleBold()
{
    QTextCharFormat fmt;
    fmt.setFontWeight(m_boldButton->isChecked() ? QFont::Bold : QFont::Normal);
    mergeFormat(fmt);
}

void RichComposerBar::toggleItalic()
{
    QTextCharFormat fmt;
    fmt.setFontItalic(m_italicButton->isChecked());
    mergeFormat(fmt);
}

void RichComposerBar::toggleUnderline()
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(m_underlineButton->isChecked());
    mergeFormat(fmt);
}

void RichComposerBar::applyPointSize(int pointSize)
{
    if (!m_edit) {
        return;
    }
    QTextCharFormat fmt;
    if (pointSize <= 0) {
        const int base = m_edit->font().pointSize() > 0 ? m_edit->font().pointSize() : 10;
        fmt.setFontPointSize(base);
    } else {
        fmt.setFontPointSize(pointSize);
    }
    mergeFormat(fmt);
}

void RichComposerBar::applySmall()
{
    applyPointSize(9);
}

void RichComposerBar::applyNormal()
{
    applyPointSize(0);
}

void RichComposerBar::applyLarge()
{
    applyPointSize(16);
}

void RichComposerBar::applyColor()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action) {
        return;
    }
    const QColor color = action->data().value<QColor>();
    if (!color.isValid()) {
        return;
    }
    QTextCharFormat fmt;
    fmt.setForeground(color);
    mergeFormat(fmt);
}

void RichComposerBar::clearColor()
{
    if (!m_edit) {
        return;
    }
    QTextCharFormat fmt;
    fmt.setForeground(m_edit->palette().color(QPalette::Text));
    mergeFormat(fmt);
}
