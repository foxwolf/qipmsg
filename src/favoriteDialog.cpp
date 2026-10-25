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

#include "favoriteDialog.h"
#include "favoriteStore.h"
#include "global.h"
#include "window_manager.h"

#include <QClipboard>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

namespace {

QString kindLabel(const QString &kind, const QString &text,
                  const QString &file, const QString &note)
{
    if (kind == QLatin1String("file")) {
        return file;
    }
    if (kind == QLatin1String("note")) {
        return note;
    }
    return text;
}

} // namespace

FavoriteDialog::FavoriteDialog(QWidget *parent)
    : QDialog(parent), m_searchEdit(0), m_list(0), m_textButton(0),
      m_fileButton(0), m_noteButton(0), m_openButton(0), m_deleteButton(0)
{
    setWindowTitle(tr("Favorites"));
    setModal(true);
    resize(520, 460);

    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText(tr("Search favorites"));
    m_searchEdit->setClearButtonEnabled(true);
    connect(m_searchEdit, SIGNAL(textChanged(QString)),
            this, SLOT(reload()));

    m_list = new QListWidget;
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(m_list, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            this, SLOT(openCurrent()));
    connect(m_list, SIGNAL(currentRowChanged(int)),
            this, SLOT(updateActions()));

    m_textButton = new QPushButton(tr("Save selected text"));
    m_fileButton = new QPushButton(tr("Save file path"));
    m_noteButton = new QPushButton(tr("Save peer note"));
    m_openButton = new QPushButton(tr("Open"));
    m_deleteButton = new QPushButton(tr("Delete"));
    QPushButton *closeButton = new QPushButton(tr("Close"));
    connect(m_textButton, SIGNAL(clicked()), this, SLOT(saveSelectedText()));
    connect(m_fileButton, SIGNAL(clicked()), this, SLOT(saveFilePath()));
    connect(m_noteButton, SIGNAL(clicked()), this, SLOT(savePeerNote()));
    connect(m_openButton, SIGNAL(clicked()), this, SLOT(openCurrent()));
    connect(m_deleteButton, SIGNAL(clicked()), this, SLOT(deleteCurrent()));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));

    QLabel *hint = new QLabel(tr(
            "Saved in favorites.json on this computer. Nothing is synced "
            "or sent unless you paste it into a chat yourself."));
    hint->setWordWrap(true);

    QHBoxLayout *saveRow = new QHBoxLayout;
    saveRow->addWidget(m_textButton);
    saveRow->addWidget(m_fileButton);
    saveRow->addWidget(m_noteButton);

    QHBoxLayout *actionRow = new QHBoxLayout;
    actionRow->addWidget(m_openButton);
    actionRow->addWidget(m_deleteButton);
    actionRow->addStretch();
    actionRow->addWidget(closeButton);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_searchEdit);
    layout->addWidget(m_list, 1);
    layout->addLayout(saveRow);
    layout->addWidget(hint);
    layout->addLayout(actionRow);

    if (Global::favoriteStore) {
        connect(Global::favoriteStore, SIGNAL(favoritesChanged()),
                this, SLOT(reload()));
    }
    reload();
}

void FavoriteDialog::setContext(const QString &peerIp, const QString &peerName,
                                const QString &selectedText,
                                const QString &peerNote)
{
    m_peerIp = peerIp.trimmed();
    m_peerName = peerName.trimmed();
    m_selectedText = selectedText.trimmed();
    m_peerNote = peerNote.trimmed();
    m_textButton->setEnabled(!m_selectedText.isEmpty());
    if (selectedPath().isEmpty()) {
        m_fileButton->setToolTip(tr("Choose a file on this computer."));
    } else {
        m_fileButton->setToolTip(selectedPath());
    }
    m_noteButton->setEnabled(!m_peerIp.isEmpty());
}

QString FavoriteDialog::selectedPath() const
{
    if (m_selectedText.isEmpty() || m_selectedText.contains(QLatin1Char('\n'))) {
        return QString();
    }
    const bool absolute = m_selectedText.startsWith(QLatin1Char('/'))
            || m_selectedText.startsWith(QLatin1String("\\\\"))
            || (m_selectedText.size() > 2
                && m_selectedText.at(1) == QLatin1Char(':'));
    if (!absolute) {
        return QString();
    }
    const QFileInfo info(m_selectedText);
    if (!info.exists()) {
        return QString();
    }
    return info.absoluteFilePath();
}

QString FavoriteDialog::selectedId() const
{
    QListWidgetItem *item = m_list->currentItem();
    if (!item) {
        return QString();
    }
    return item->data(Qt::UserRole).toString();
}

void FavoriteDialog::rememberId(const QString &id)
{
    m_keepId = id;
    reload();
}

void FavoriteDialog::reload()
{
    const QString keep = m_keepId.isEmpty() ? selectedId() : m_keepId;
    m_keepId.clear();
    m_list->clear();
    QList<FavoriteEntry> rows;
    if (Global::favoriteStore) {
        rows = Global::favoriteStore->search(m_searchEdit->text());
    }
    int selectRow = -1;
    const QString textLabel = tr("Text");
    const QString fileLabel = tr("File");
    const QString noteLabel = tr("Note");
    for (int i = rows.size() - 1; i >= 0; --i) {
        const FavoriteEntry &item = rows.at(i);
        QString title = item.title;
        title.replace(QLatin1Char('\n'), QLatin1Char(' '));
        QString line = QString("[%1] %2").arg(
                    kindLabel(item.kind, textLabel, fileLabel, noteLabel), title);
        if (!item.peerName.isEmpty()) {
            line += QStringLiteral("  · ") + item.peerName;
        }
        QListWidgetItem *row = new QListWidgetItem(line, m_list);
        row->setData(Qt::UserRole, item.id);
        row->setToolTip(item.body);
        if (item.id == keep) {
            selectRow = m_list->count() - 1;
        }
    }
    if (selectRow >= 0) {
        m_list->setCurrentRow(selectRow);
    }
    updateActions();
}

void FavoriteDialog::updateActions()
{
    const bool hasRow = m_list && m_list->currentItem() != 0;
    if (m_openButton) {
        m_openButton->setEnabled(hasRow);
    }
    if (m_deleteButton) {
        m_deleteButton->setEnabled(hasRow);
    }
}

void FavoriteDialog::saveSelectedText()
{
    if (!Global::favoriteStore || m_selectedText.isEmpty()) {
        return;
    }
    const QString id = Global::favoriteStore->add(
                QStringLiteral("text"), QString(), m_selectedText,
                m_peerIp, m_peerName);
    if (id.isEmpty()) {
        QMessageBox::warning(this, tr("Favorites"),
                             tr("Could not save the selected text."));
        return;
    }
    rememberId(id);
}

void FavoriteDialog::saveFilePath()
{
    if (!Global::favoriteStore) {
        return;
    }
    QString path = selectedPath();
    if (path.isEmpty()) {
        path = QFileDialog::getOpenFileName(this, tr("Save file path"),
                                             QString(),
                                             tr("All files (*)"));
    }
    if (path.isEmpty()) {
        return;
    }
    const QFileInfo info(path);
    const QString id = Global::favoriteStore->add(
                QStringLiteral("file"), info.fileName(),
                info.absoluteFilePath(), m_peerIp, m_peerName);
    if (id.isEmpty()) {
        QMessageBox::warning(this, tr("Favorites"),
                             tr("Could not save the file path."));
        return;
    }
    rememberId(id);
}

void FavoriteDialog::savePeerNote()
{
    if (!Global::favoriteStore || m_peerIp.isEmpty()) {
        return;
    }
    QString note = m_peerNote;
    if (note.isEmpty()) {
        bool ok = false;
        note = QInputDialog::getText(this, tr("Save peer note"),
                                      tr("Note about %1:").arg(
                                          m_peerName.isEmpty() ? m_peerIp
                                                               : m_peerName),
                                      QLineEdit::Normal, QString(), &ok).trimmed();
        if (!ok || note.isEmpty()) {
            return;
        }
        m_peerNote = note;
    }
    const QString title = tr("Note about %1").arg(
                m_peerName.isEmpty() ? m_peerIp : m_peerName);
    const QString id = Global::favoriteStore->add(
                QStringLiteral("note"), title, note, m_peerIp, m_peerName);
    if (id.isEmpty()) {
        QMessageBox::warning(this, tr("Favorites"),
                             tr("Could not save the note."));
        return;
    }
    rememberId(id);
}

void FavoriteDialog::openCurrent()
{
    const QString id = selectedId();
    if (id.isEmpty() || !Global::favoriteStore) {
        return;
    }
    const QList<FavoriteEntry> rows = Global::favoriteStore->items();
    FavoriteEntry item;
    bool found = false;
    for (int i = 0; i < rows.size(); ++i) {
        if (rows.at(i).id == id) {
            item = rows.at(i);
            found = true;
            break;
        }
    }
    if (!found) {
        return;
    }
    if (item.kind == QLatin1String("file")) {
        const QFileInfo info(item.body);
        if (!info.exists()) {
            QMessageBox::warning(this, tr("Favorites"),
                                 tr("That path is not on this computer:\n%1")
                                 .arg(item.body));
            return;
        }
        QDesktopServices::openUrl(QUrl::fromLocalFile(info.absoluteFilePath()));
        return;
    }

    QMessageBox box(this);
    box.setWindowTitle(item.title.isEmpty() ? tr("Favorites") : item.title);
    box.setText(item.body);
    box.setTextInteractionFlags(Qt::TextSelectableByMouse);
    QPushButton *copyButton = box.addButton(tr("Copy"), QMessageBox::ActionRole);
    QPushButton *chatButton = 0;
    if (!item.peerIp.isEmpty() && Global::windowManager) {
        chatButton = box.addButton(tr("Open chat"), QMessageBox::ActionRole);
    }
    box.addButton(QMessageBox::Close);
    box.exec();
    if (box.clickedButton() == copyButton) {
        if (QGuiApplication::clipboard()) {
            QGuiApplication::clipboard()->setText(item.body);
        }
    } else if (chatButton && box.clickedButton() == chatButton) {
        Global::windowManager->openChat(item.peerIp, true, QString());
    }
}

void FavoriteDialog::deleteCurrent()
{
    const QString id = selectedId();
    if (id.isEmpty() || !Global::favoriteStore) {
        return;
    }
    if (!Global::favoriteStore->remove(id)) {
        QMessageBox::warning(this, tr("Favorites"),
                             tr("Could not delete the favorite."));
        return;
    }
}
