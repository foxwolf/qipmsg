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

#ifndef RICHTEXT_H
#define RICHTEXT_H

#include <QPixmap>
#include <QString>
#include <QStringList>
#include <QTextEdit>

class QMimeData;
class QTextCharFormat;
class QTextCursor;
class QTextDocument;

// Practical rich-text subset for the chat composer.
// Wire text stays plain. A qipmsg peer may also get a UTF-8 HTML sidecar
// (see RichText::sidecarBody). FeiQ never receives that packet.

class RichComposer : public QTextEdit
{
    Q_OBJECT

public:
    explicit RichComposer(QWidget *parent = 0);

signals:
    void imagePasted(const QPixmap &pixmap);
    void imageFilesPasted(const QStringList &paths);

protected:
    void insertFromMimeData(const QMimeData *source);
    bool canInsertFromMimeData(const QMimeData *source) const;
};

class RichComposerBar : public QWidget
{
    Q_OBJECT

public:
    explicit RichComposerBar(QTextEdit *edit, QWidget *parent = 0);

private slots:
    void syncState();
    void toggleBold();
    void toggleItalic();
    void toggleUnderline();
    void applySmall();
    void applyNormal();
    void applyLarge();
    void applyColor();
    void clearColor();

private:
    void mergeFormat(const QTextCharFormat &fmt);
    void applyPointSize(int pointSize);

    QTextEdit *m_edit;
    class QToolButton *m_boldButton;
    class QToolButton *m_italicButton;
    class QToolButton *m_underlineButton;
};

class RichText
{
public:
    static int packetProperty();

    static QString canonicalFromDocument(const QTextDocument *doc);
    static QString sanitizeHtml(const QString &html);
    static QString plainFromHtml(const QString &html);
    static bool hasFormatting(const QString &canonicalHtml);
    static bool samePlain(const QString &left, const QString &right);
    static bool coversPlain(const QString &html, const QString &plain);

    static QString sidecarBody(const QString &packetNo, const QString &html);
    static bool parseSidecar(const QString &body, QString *packetNo, QString *html);

    static void insertFormatted(QTextCursor &cursor, const QString &html,
                                const QTextCharFormat &base);
};

#endif // !RICHTEXT_H
