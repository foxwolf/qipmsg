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

#include "contactDelegate.h"
#include "helper.h"

#include <QIcon>
#include <QPainter>
#include <QPixmap>

namespace {
const int PeerHeight = 52;
const int GroupHeight = 28;
const int AvatarSize = 36;
}

ContactDelegate::ContactDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void ContactDelegate::paint(QPainter *painter,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    const QRect rect = opt.rect.adjusted(4, 1, -4, -1);
    const bool selected = opt.state & QStyle::State_Selected;
    const bool hover = opt.state & QStyle::State_MouseOver;
    const QString ip = index.data(Qt::UserRole).toString();
    const QString groupKey = index.data(Qt::UserRole + 1).toString();
    const bool isGroup = ip.isEmpty() && !groupKey.isEmpty();

    const bool dark = Helper::darkTheme();
    if (selected) {
        painter->setBrush(dark ? QColor("#1E3A5F") : QColor("#E8F1FF"));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(rect, 8, 8);
    } else if (hover && !isGroup) {
        painter->setBrush(dark ? QColor("#374151") : QColor("#F3F4F6"));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(rect, 8, 8);
    }

    if (isGroup) {
        QFont font = opt.font;
        font.setBold(true);
        font.setPixelSize(12);
        painter->setFont(font);
        painter->setPen(dark ? QColor("#9CA3AF") : QColor("#6B7280"));
        painter->drawText(rect.adjusted(8, 0, -8, 0),
                          Qt::AlignVCenter | Qt::AlignLeft,
                          index.data(Qt::DisplayRole).toString());
        painter->restore();
        return;
    }

    QPixmap avatar;
    const QVariant deco = index.data(Qt::DecorationRole);
    if (deco.canConvert<QIcon>()) {
        avatar = deco.value<QIcon>().pixmap(QSize(AvatarSize, AvatarSize));
    } else if (deco.canConvert<QPixmap>()) {
        avatar = deco.value<QPixmap>();
    }

    const QRect avatarRect(rect.left() + 8,
                           rect.center().y() - AvatarSize / 2,
                           AvatarSize, AvatarSize);
    if (!avatar.isNull()) {
        painter->drawPixmap(avatarRect, avatar.scaled(AvatarSize, AvatarSize,
                    Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    if (index.data(Qt::UserRole + 10).toBool()) {
        const QRect badge(avatarRect.right() - 14, avatarRect.bottom() - 14, 16, 16);
        painter->setBrush(QColor("#059669"));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(badge);
        QFont badgeFont = opt.font;
        badgeFont.setPixelSize(9);
        badgeFont.setBold(true);
        painter->setFont(badgeFont);
        painter->setPen(QColor("#FFFFFF"));
        painter->drawText(badge, Qt::AlignCenter, QStringLiteral("\u4eab"));
    }
    if (index.data(Qt::UserRole + 11).toBool()) {
        const QRect badge(avatarRect.right() - 14, avatarRect.top(), 16, 16);
        painter->setBrush(QColor("#D97706"));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(badge);
        QFont badgeFont = opt.font;
        badgeFont.setPixelSize(9);
        badgeFont.setBold(true);
        painter->setFont(badgeFont);
        painter->setPen(QColor("#FFFFFF"));
        painter->drawText(badge, Qt::AlignCenter, tr("D", "one-letter dial-up badge"));
    }

    QString title = index.data(Qt::UserRole + 6).toString();
    if (title.isEmpty()) {
        title = index.data(Qt::DisplayRole).toString().section('\n', 0, 0);
    }
    QString subtitle = index.data(Qt::UserRole + 4).toString();

    QRect textRect = rect.adjusted(8 + AvatarSize + 10, 6, -10, -6);
    QFont nameFont = opt.font;
    nameFont.setBold(true);
    nameFont.setPixelSize(13);
    painter->setFont(nameFont);
    painter->setPen(index.data(Qt::ForegroundRole).isValid()
                    ? index.data(Qt::ForegroundRole).value<QColor>()
                    : (dark ? QColor("#F9FAFB") : QColor("#111827")));
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignTop,
                      painter->fontMetrics().elidedText(title, Qt::ElideRight,
                                                        textRect.width()));

    if (index.data(Qt::UserRole + 9).toBool()) {
        const QString typing = tr("Typing…");
        subtitle = subtitle.isEmpty() ? typing
                : QString("%1 · %2").arg(typing, subtitle);
    }

    if (!subtitle.isEmpty()) {
        QFont subFont = opt.font;
        subFont.setBold(false);
        subFont.setPixelSize(11);
        painter->setFont(subFont);
        painter->setPen(dark ? QColor("#9CA3AF") : QColor("#8A8F98"));
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignBottom,
                          painter->fontMetrics().elidedText(subtitle, Qt::ElideRight,
                                                            textRect.width()));
    }

    painter->restore();
}

QSize ContactDelegate::sizeHint(const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    Q_UNUSED(option);
    const QString ip = index.data(Qt::UserRole).toString();
    const QString groupKey = index.data(Qt::UserRole + 1).toString();
    if (ip.isEmpty() && !groupKey.isEmpty()) {
        return QSize(160, GroupHeight);
    }
    return QSize(180, PeerHeight);
}
