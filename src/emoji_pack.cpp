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

#include "emoji_pack.h"

#include <QCoreApplication>
#include <QObject>

QList<EmojiItem> EmojiPack::items()
{
    QList<EmojiItem> list;
    const struct {
        const char *glyph;
        const char *tip;
        const char *token;
    } raw[] = {
        { "😀", QT_TRANSLATE_NOOP("EmojiPack", "Smile"), "/微笑" },
        { "😄", QT_TRANSLATE_NOOP("EmojiPack", "Happy"), "/开心" },
        { "😁", QT_TRANSLATE_NOOP("EmojiPack", "Grin"), "/偷笑" },
        { "😊", QT_TRANSLATE_NOOP("EmojiPack", "Blush"), "/害羞" },
        { "😍", QT_TRANSLATE_NOOP("EmojiPack", "Heart eyes"), "/色" },
        { "😏", QT_TRANSLATE_NOOP("EmojiPack", "Smirk"), "/得意" },
        { "😢", QT_TRANSLATE_NOOP("EmojiPack", "Tear"), "/流泪" },
        { "😭", QT_TRANSLATE_NOOP("EmojiPack", "Sob"), "/大哭" },
        { "😅", QT_TRANSLATE_NOOP("EmojiPack", "Sweat smile"), "/尴尬" },
        { "😡", QT_TRANSLATE_NOOP("EmojiPack", "Angry"), "/发怒" },
        { "😜", QT_TRANSLATE_NOOP("EmojiPack", "Wink"), "/调皮" },
        { "😲", QT_TRANSLATE_NOOP("EmojiPack", "Surprised"), "/惊讶" },
        { "😞", QT_TRANSLATE_NOOP("EmojiPack", "Sad"), "/难过" },
        { "😎", QT_TRANSLATE_NOOP("EmojiPack", "Cool"), "/酷" },
        { "😰", QT_TRANSLATE_NOOP("EmojiPack", "Cold sweat"), "/冷汗" },
        { "😫", QT_TRANSLATE_NOOP("EmojiPack", "Tired"), "/抓狂" },
        { "😴", QT_TRANSLATE_NOOP("EmojiPack", "Sleep"), "/睡" },
        { "🤐", QT_TRANSLATE_NOOP("EmojiPack", "Zipper"), "/闭嘴" },
        { "👍", QT_TRANSLATE_NOOP("EmojiPack", "Thumbs up"), "/强" },
        { "👎", QT_TRANSLATE_NOOP("EmojiPack", "Thumbs down"), "/弱" },
        { "👏", QT_TRANSLATE_NOOP("EmojiPack", "Clap"), "/鼓掌" },
        { "❤️", QT_TRANSLATE_NOOP("EmojiPack", "Heart"), "/爱心" },
        { "🌹", QT_TRANSLATE_NOOP("EmojiPack", "Rose"), "/玫瑰" },
        { "🎉", QT_TRANSLATE_NOOP("EmojiPack", "Party"), "/庆祝" }
    };

    const int count = static_cast<int>(sizeof(raw) / sizeof(raw[0]));
    for (int i = 0; i < count; ++i) {
        EmojiItem item;
        item.glyph = QString::fromUtf8(raw[i].glyph);
        item.tip = QCoreApplication::translate("EmojiPack", raw[i].tip);
        item.token = QString::fromUtf8(raw[i].token);
        list << item;
    }
    return list;
}

QString EmojiPack::applyAliases(const QString &text)
{
    QString result = text;
    foreach (const EmojiItem &item, items()) {
        if (!item.token.isEmpty()) {
            result.replace(item.token, item.glyph);
        }
    }
    return result;
}
