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

#include "zoneBrowser.h"

#include <QCloseEvent>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

ZoneBrowser::ZoneBrowser(QWidget *parent)
    : QDialog(parent), m_hint(0), m_list(0), m_editor(0), m_saveButton(0),
      m_deleteButton(0), m_dirty(false), m_loading(false)
{
    setWindowTitle(tr("Local notes"));
    setModal(true);
    resize(720, 480);

    m_hint = new QLabel(tr(
            "Notes in %1. Markdown and text only. They stay on this computer. "
            "This is not a FeiQ zone database and nothing here is sent to peers.")
                        .arg(m_store.rootPath()));
    m_hint->setWordWrap(true);

    m_list = new QListWidget;
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(m_list, SIGNAL(currentRowChanged(int)),
            this, SLOT(selectionChanged()));

    m_editor = new QPlainTextEdit;
    m_editor->setPlaceholderText(tr("Select a note, or create one."));
    connect(m_editor, SIGNAL(textChanged()), this, SLOT(noteEdited()));

    QPushButton *newButton = new QPushButton(tr("New"));
    m_saveButton = new QPushButton(tr("Save"));
    m_deleteButton = new QPushButton(tr("Delete"));
    QPushButton *folderButton = new QPushButton(tr("Open folder"));
    QPushButton *closeButton = new QPushButton(tr("Close"));
    connect(newButton, SIGNAL(clicked()), this, SLOT(createNote()));
    connect(m_saveButton, SIGNAL(clicked()), this, SLOT(saveNote()));
    connect(m_deleteButton, SIGNAL(clicked()), this, SLOT(deleteNote()));
    connect(folderButton, SIGNAL(clicked()), this, SLOT(openFolder()));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(reject()));

    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addWidget(newButton);
    buttons->addWidget(m_saveButton);
    buttons->addWidget(m_deleteButton);
    buttons->addWidget(folderButton);
    buttons->addStretch();
    buttons->addWidget(closeButton);

    QHBoxLayout *body = new QHBoxLayout;
    body->addWidget(m_list, 1);
    body->addWidget(m_editor, 2);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_hint);
    layout->addLayout(body, 1);
    layout->addLayout(buttons);

    QString error;
    m_store.ensureRoot(&error);
    reload();
}

void ZoneBrowser::closeEvent(QCloseEvent *event)
{
    if (!commitDirty()) {
        event->ignore();
        return;
    }
    QDialog::closeEvent(event);
}

void ZoneBrowser::reload()
{
    const QString keep = m_loadedName;
    m_list->blockSignals(true);
    m_list->clear();
    const QStringList names = m_store.noteNames();
    for (int i = 0; i < names.size(); ++i) {
        m_list->addItem(names.at(i));
    }
    int row = -1;
    for (int i = 0; i < m_list->count(); ++i) {
        if (m_list->item(i)->text() == keep) {
            row = i;
            break;
        }
    }
    if (row < 0 && m_list->count() > 0) {
        row = 0;
    }
    if (row >= 0) {
        m_list->setCurrentRow(row);
    }
    m_list->blockSignals(false);
    loadCurrent(row >= 0 ? m_list->item(row)->text() : QString());
}

void ZoneBrowser::selectionChanged()
{
    if (m_loading) {
        return;
    }
    const QString name = currentName();
    if (name == m_loadedName) {
        return;
    }
    if (!commitDirty()) {
        m_loading = true;
        for (int i = 0; i < m_list->count(); ++i) {
            if (m_list->item(i)->text() == m_loadedName) {
                m_list->setCurrentRow(i);
                break;
            }
        }
        m_loading = false;
        return;
    }
    loadCurrent(name);
}

void ZoneBrowser::noteEdited()
{
    if (m_loading) {
        return;
    }
    m_dirty = true;
}

void ZoneBrowser::createNote()
{
    if (!commitDirty()) {
        return;
    }
    bool ok = false;
    const QString title = QInputDialog::getText(
                this, tr("Local notes"), tr("Title"), QLineEdit::Normal,
                QString(), &ok);
    if (!ok) {
        return;
    }
    QString error;
    const QString name = m_store.createNote(title, &error);
    if (name.isEmpty()) {
        QMessageBox::warning(this, tr("Local notes"),
                             error.isEmpty() ? tr("Could not create the note.") : error);
        return;
    }
    m_loadedName = name;
    m_dirty = false;
    reload();
}

void ZoneBrowser::saveNote()
{
    if (m_loadedName.isEmpty()) {
        return;
    }
    QString error;
    if (!m_store.writeNote(m_loadedName, m_editor->toPlainText(), &error)) {
        QMessageBox::warning(this, tr("Local notes"),
                             error.isEmpty() ? tr("Could not save the note.") : error);
        return;
    }
    m_dirty = false;
}

void ZoneBrowser::deleteNote()
{
    const QString name = m_loadedName;
    if (name.isEmpty()) {
        return;
    }
    const int answer = QMessageBox::question(
                this, tr("Local notes"),
                tr("Delete %1? This only removes the file on this computer.")
                .arg(name));
    if (answer != QMessageBox::Yes) {
        return;
    }
    QString error;
    if (!m_store.removeNote(name, &error)) {
        QMessageBox::warning(this, tr("Local notes"),
                             error.isEmpty() ? tr("Could not delete the note.") : error);
        return;
    }
    m_dirty = false;
    m_loadedName.clear();
    reload();
}

void ZoneBrowser::openFolder()
{
    QString error;
    if (!m_store.ensureRoot(&error)) {
        QMessageBox::warning(this, tr("Local notes"), error);
        return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_store.rootPath()));
}

bool ZoneBrowser::commitDirty()
{
    if (!m_dirty || m_loadedName.isEmpty()) {
        m_dirty = false;
        return true;
    }
    const int answer = QMessageBox::question(
                this, tr("Local notes"),
                tr("Save changes to %1?").arg(m_loadedName),
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                QMessageBox::Yes);
    if (answer == QMessageBox::Cancel) {
        return false;
    }
    if (answer == QMessageBox::No) {
        m_dirty = false;
        return true;
    }
    QString error;
    if (!m_store.writeNote(m_loadedName, m_editor->toPlainText(), &error)) {
        QMessageBox::warning(this, tr("Local notes"),
                             error.isEmpty() ? tr("Could not save the note.") : error);
        return false;
    }
    m_dirty = false;
    return true;
}

void ZoneBrowser::loadCurrent(const QString &name)
{
    m_loading = true;
    m_loadedName = name;
    m_dirty = false;
    if (name.isEmpty()) {
        m_editor->clear();
        m_editor->setEnabled(false);
    } else {
        QString error;
        const QString body = m_store.readNote(name, &error);
        if (!error.isEmpty() && body.isEmpty()) {
            m_editor->setPlainText(QString());
            m_hint->setText(error);
        } else {
            m_editor->setPlainText(body);
            m_hint->setText(tr(
                    "Notes in %1. Markdown and text only. They stay on this computer. "
                    "This is not a FeiQ zone database and nothing here is sent to peers.")
                            .arg(m_store.rootPath()));
        }
        m_editor->setEnabled(error.isEmpty());
    }
    m_loading = false;
    m_dirty = false;
    if (m_saveButton) {
        m_saveButton->setEnabled(!name.isEmpty());
    }
    if (m_deleteButton) {
        m_deleteButton->setEnabled(!name.isEmpty());
    }
}

QString ZoneBrowser::currentName() const
{
    QListWidgetItem *item = m_list ? m_list->currentItem() : 0;
    if (!item) {
        return QString();
    }
    return item->text();
}
