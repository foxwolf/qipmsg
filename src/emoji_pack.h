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

#ifndef EMOJI_PACK_H
#define EMOJI_PACK_H

#include <QList>
#include <QString>

class EmojiItem
{
public:
    QString glyph;
    QString tip;
    QString token;
};

class EmojiPack
{
public:
    static QList<EmojiItem> items();
    static QString applyAliases(const QString &text);
};

#endif // !EMOJI_PACK_H
