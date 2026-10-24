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

#ifndef MENTION_HIGHLIGHTER_H
#define MENTION_HIGHLIGHTER_H

#include <QColor>
#include <QPair>
#include <QSyntaxHighlighter>
#include <QStringList>
#include <QTextCharFormat>

class QTextCursor;

class MentionHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit MentionHighlighter(QTextDocument *parent = 0);

    void setNames(const QStringList &names);

    static void insertHighlighted(QTextCursor &cursor, const QString &text,
                                  const QTextCharFormat &base,
                                  const QStringList &names,
                                  const QColor &mentionColor);

protected:
    void highlightBlock(const QString &text) override;

private:
    static QList<QPair<int, int> > mentionRanges(const QString &text,
                                                 const QStringList &names);

    QStringList m_names;
    QTextCharFormat m_mentionFormat;
};

#endif // !MENTION_HIGHLIGHTER_H
