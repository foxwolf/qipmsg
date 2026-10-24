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

#include "stickerPicker.h"
#include "emoji_pack.h"
#include "global.h"
#include "helper.h"
#include "stickerStore.h"

#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QIcon>
#include <QPixmap>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>

namespace {

const char *builtinGroupId = "builtin";

} // namespace

StickerPicker::StickerPicker(QWidget *parent)
    : QDialog(parent), m_groupCombo(0), m_gridHost(0), m_grid(0),
      m_status(0), m_renameGroupButton(0), m_deleteGroupButton(0),
      m_filling(false)
{
    setWindowTitle(tr("Emoji"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    resize(420, 460);

    m_groupCombo = new QComboBox;
    connect(m_groupCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(rebuildGrid()));

    m_gridHost = new QWidget;
    m_grid = new QGridLayout(m_gridHost);
    m_grid->setContentsMargins(8, 8, 8, 8);
    m_grid->setSpacing(4);
    m_grid->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    QScrollArea *scroll = new QScrollArea;
    scroll->setWidget(m_gridHost);
    scroll->setWidgetResizable(true);
    scroll->setMinimumHeight(260);
    scroll->setFrameShape(QFrame::NoFrame);

    QPushButton *importButton = new QPushButton(tr("Import"));
    importButton->setToolTip(tr("Copy PNG, GIF, WebP, or JPEG into your sticker folder"));
    QPushButton *newButton = new QPushButton(tr("New group"));
    m_renameGroupButton = new QPushButton(tr("Rename"));
    m_deleteGroupButton = new QPushButton(tr("Delete group"));
    connect(importButton, SIGNAL(clicked()), this, SLOT(importStickers()));
    connect(newButton, SIGNAL(clicked()), this, SLOT(createGroup()));
    connect(m_renameGroupButton, SIGNAL(clicked()), this, SLOT(renameCurrentGroup()));
    connect(m_deleteGroupButton, SIGNAL(clicked()), this, SLOT(deleteCurrentGroup()));

    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addWidget(importButton);
    buttons->addWidget(newButton);
    buttons->addWidget(m_renameGroupButton);
    buttons->addWidget(m_deleteGroupButton);

    m_status = new QLabel;
    m_status->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_groupCombo);
    layout->addWidget(scroll, 1);
    layout->addLayout(buttons);
    layout->addWidget(m_status);

    if (Global::stickerStore) {
        connect(Global::stickerStore, SIGNAL(stickersChanged()),
                this, SLOT(rebuildGrid()));
    }
    refillGroups(QString::fromLatin1(builtinGroupId));
}

void StickerPicker::refillGroups(const QString &selectId)
{
    if (!m_groupCombo) {
        return;
    }
    m_filling = true;
    m_groupCombo->blockSignals(true);
    m_groupCombo->clear();
    m_groupCombo->addItem(tr("Classic"), QString::fromLatin1(builtinGroupId));
    if (Global::stickerStore) {
        foreach (const StickerGroup &group, Global::stickerStore->groups()) {
            m_groupCombo->addItem(group.name, group.id);
        }
    }
    int index = 0;
    if (!selectId.isEmpty()) {
        const int found = m_groupCombo->findData(selectId);
        if (found >= 0) {
            index = found;
        }
    }
    m_groupCombo->setCurrentIndex(index);
    m_groupCombo->blockSignals(false);
    m_filling = false;
    rebuildGrid();
}

bool StickerPicker::showingBuiltin() const
{
    return currentGroupId() == QLatin1String(builtinGroupId);
}

QString StickerPicker::currentGroupId() const
{
    if (!m_groupCombo) {
        return QString();
    }
    return m_groupCombo->currentData().toString();
}

void StickerPicker::updateGroupButtons()
{
    const bool custom = !showingBuiltin() && !currentGroupId().isEmpty();
    if (m_renameGroupButton) {
        m_renameGroupButton->setEnabled(custom);
    }
    if (m_deleteGroupButton) {
        m_deleteGroupButton->setEnabled(custom);
    }
}

void StickerPicker::rebuildGrid()
{
    if (m_filling || !m_grid) {
        return;
    }
    while (QLayoutItem *child = m_grid->takeAt(0)) {
        if (QWidget *widget = child->widget()) {
            widget->deleteLater();
        }
        delete child;
    }

    updateGroupButtons();

    if (showingBuiltin()) {
        const QList<EmojiItem> items = EmojiPack::items();
        for (int i = 0; i < items.size(); ++i) {
            const EmojiItem item = items.at(i);
            QToolButton *button = new QToolButton(m_gridHost);
            button->setAutoRaise(true);
            button->setText(item.glyph);
            button->setToolTip(item.tip);
            button->setFixedSize(36, 36);
            QFont font = button->font();
            font.setPixelSize(18);
            button->setFont(font);
            connect(button, &QToolButton::clicked, this, [this, item]() {
                emit unicodeChosen(item.glyph);
                accept();
            });
            m_grid->addWidget(button, i / 8, i % 8);
        }
        if (m_status) {
            m_status->setText(tr("Classic emoji are inserted as text. Custom stickers are sent as image files."));
        }
        return;
    }

    if (!Global::stickerStore) {
        return;
    }
    const QString groupId = currentGroupId();
    const QList<StickerItem> items = Global::stickerStore->stickers(groupId);
    for (int i = 0; i < items.size(); ++i) {
        const StickerItem item = items.at(i);
        const QString path = Global::stickerStore->pathFor(item);
        QToolButton *button = new QToolButton(m_gridHost);
        button->setAutoRaise(true);
        button->setFixedSize(48, 48);
        button->setToolTip(tr("%1\nRight-click to delete or move").arg(item.title));
        const QPixmap thumb = Helper::imageThumbnail(path, 40, 40);
        if (thumb.isNull()) {
            button->setText(item.title.left(2));
        } else {
            button->setIcon(QIcon(thumb));
            button->setIconSize(QSize(40, 40));
        }
        const QString suffix = QFileInfo(path).suffix().toLower();
        if (suffix == QLatin1String("gif")) {
            button->setToolTip(button->toolTip() + QLatin1Char('\n') + tr("GIF"));
        }
        connect(button, &QToolButton::clicked, this, [this, path, item]() {
            emit stickerChosen(path, item.title);
            accept();
        });
        button->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(button, &QWidget::customContextMenuRequested, this,
                [this, button, item](const QPoint &pos) {
            if (!Global::stickerStore) {
                return;
            }
            QMenu menu(button);
            QAction *removeAction = menu.addAction(tr("Delete sticker"));
            QMenu *moveMenu = menu.addMenu(tr("Move to"));
            foreach (const StickerGroup &group, Global::stickerStore->groups()) {
                if (group.id == item.groupId) {
                    continue;
                }
                QAction *moveAction = moveMenu->addAction(group.name);
                moveAction->setData(group.id);
            }
            if (moveMenu->actions().isEmpty()) {
                moveMenu->setEnabled(false);
            }
            QAction *chosen = menu.exec(button->mapToGlobal(pos));
            if (!chosen) {
                return;
            }
            if (chosen == removeAction) {
                Global::stickerStore->removeSticker(item.id);
            } else if (!chosen->data().toString().isEmpty()) {
                Global::stickerStore->moveSticker(item.id, chosen->data().toString());
            }
        });
        m_grid->addWidget(button, i / 6, i % 6);
    }
    if (items.isEmpty() && m_status) {
        m_status->setText(tr("No stickers in this group. Import a PNG, GIF, or WebP."));
    } else if (m_status) {
        m_status->setText(tr("Click a sticker to send it as a file attachment."));
    }
}

void StickerPicker::importStickers()
{
    if (!Global::stickerStore) {
        return;
    }
    const QStringList paths = QFileDialog::getOpenFileNames(
                this, tr("Import stickers"), QString(),
                tr("Images (*.png *.gif *.webp *.jpg *.jpeg *.bmp)"));
    if (paths.isEmpty()) {
        return;
    }
    const QString target = showingBuiltin() ? QString() : currentGroupId();
    QString used;
    const QStringList added = Global::stickerStore->importFiles(paths, target, &used);
    if (!used.isEmpty()) {
        refillGroups(used);
    }
    if (m_status) {
        m_status->setText(tr("Imported %1, skipped %2.")
                          .arg(added.size())
                          .arg(paths.size() - added.size()));
    }
}

void StickerPicker::createGroup()
{
    if (!Global::stickerStore) {
        return;
    }
    bool ok = false;
    const QString name = QInputDialog::getText(
                this, tr("New sticker group"), tr("Group name:"),
                QLineEdit::Normal, QString(), &ok);
    if (!ok) {
        return;
    }
    const QString id = Global::stickerStore->addGroup(name);
    if (id.isEmpty()) {
        QMessageBox::information(this, tr("Emoji groups"),
                tr("Could not add that group. The name may be empty, too long, or the list is full."));
        return;
    }
    refillGroups(id);
}

void StickerPicker::renameCurrentGroup()
{
    if (!Global::stickerStore || showingBuiltin()) {
        return;
    }
    const QString id = currentGroupId();
    bool ok = false;
    const QString name = QInputDialog::getText(
                this, tr("Rename sticker group"), tr("Group name:"),
                QLineEdit::Normal, m_groupCombo->currentText(), &ok);
    if (!ok) {
        return;
    }
    if (!Global::stickerStore->renameGroup(id, name)) {
        QMessageBox::information(this, tr("Emoji groups"),
                tr("Could not rename. The name may be empty or already used."));
        return;
    }
    refillGroups(id);
}

void StickerPicker::deleteCurrentGroup()
{
    if (!Global::stickerStore || showingBuiltin()) {
        return;
    }
    const QString id = currentGroupId();
    const QString name = m_groupCombo->currentText();
    if (QMessageBox::question(this, tr("Emoji groups"),
            tr("Delete group \"%1\"? Stickers move to another group when one exists.")
                              .arg(name)) != QMessageBox::Yes) {
        return;
    }
    if (!Global::stickerStore->removeGroup(id)) {
        QMessageBox::information(this, tr("Emoji groups"),
                tr("Delete the stickers in the last group before removing it."));
        return;
    }
    refillGroups(QString());
}
