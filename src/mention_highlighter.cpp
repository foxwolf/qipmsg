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

#include "mention_highlighter.h"

#include <QTextCursor>
#include <algorithm>

MentionHighlighter::MentionHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    m_mentionFormat.setForeground(QColor("#1D4ED8"));
    m_mentionFormat.setFontWeight(QFont::Bold);
    m_mentionFormat.setBackground(QColor("#DBEAFE"));
}

void MentionHighlighter::setNames(const QStringList &names)
{
    m_names = names;
    std::sort(m_names.begin(), m_names.end(),
              [](const QString &a, const QString &b) {
                  return a.size() > b.size();
              });
    rehighlight();
}

QList<QPair<int, int> > MentionHighlighter::mentionRanges(const QString &text,
                                                          const QStringList &names)
{
    QList<QPair<int, int> > ranges;
    if (text.isEmpty() || names.isEmpty()) {
        return ranges;
    }
    QStringList sorted = names;
    std::sort(sorted.begin(), sorted.end(),
              [](const QString &a, const QString &b) {
                  return a.size() > b.size();
              });
    int i = 0;
    while (i < text.size()) {
        if (text.at(i) != QLatin1Char('@')) {
            ++i;
            continue;
        }
        bool matched = false;
        foreach (const QString &name, sorted) {
            if (name.isEmpty()) {
                continue;
            }
            if (text.mid(i + 1, name.size()).compare(name, Qt::CaseInsensitive) == 0) {
                ranges.append(qMakePair(i, name.size() + 1));
                i += name.size() + 1;
                matched = true;
                break;
            }
        }
        if (!matched) {
            ++i;
        }
    }
    return ranges;
}

void MentionHighlighter::highlightBlock(const QString &text)
{
    const QList<QPair<int, int> > ranges = mentionRanges(text, m_names);
    for (int r = 0; r < ranges.size(); ++r) {
        setFormat(ranges.at(r).first, ranges.at(r).second, m_mentionFormat);
    }
}

void MentionHighlighter::insertHighlighted(QTextCursor &cursor, const QString &text,
                                           const QTextCharFormat &base,
                                           const QStringList &names,
                                           const QColor &mentionColor)
{
    QTextCharFormat mention = base;
    mention.setFontWeight(QFont::Bold);
    mention.setBackground(QColor(mentionColor.red(), mentionColor.green(),
                                 mentionColor.blue(), 80));
    const QList<QPair<int, int> > ranges = mentionRanges(text, names);
    int pos = 0;
    for (int r = 0; r < ranges.size(); ++r) {
        const int start = ranges.at(r).first;
        const int len = ranges.at(r).second;
        if (start > pos) {
            cursor.insertText(text.mid(pos, start - pos), base);
        }
        cursor.insertText(text.mid(start, len), mention);
        pos = start + len;
    }
    if (pos < text.size()) {
        cursor.insertText(text.mid(pos), base);
    }
}
