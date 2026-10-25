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

#ifndef ZONEBROWSER_H
#define ZONEBROWSER_H

#include <QDialog>

#include "zoneStore.h"

class QCloseEvent;
class QLabel;
class QListWidget;
class QPlainTextEdit;
class QPushButton;

class ZoneBrowser : public QDialog
{
    Q_OBJECT

public:
    explicit ZoneBrowser(QWidget *parent = 0);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void reload();
    void selectionChanged();
    void noteEdited();
    void createNote();
    void saveNote();
    void deleteNote();
    void openFolder();

private:
    bool commitDirty();
    void loadCurrent(const QString &name);
    QString currentName() const;

    ZoneStore m_store;
    QLabel *m_hint;
    QListWidget *m_list;
    QPlainTextEdit *m_editor;
    QPushButton *m_saveButton;
    QPushButton *m_deleteButton;
    QString m_loadedName;
    bool m_dirty;
    bool m_loading;
};

#endif // !ZONEBROWSER_H
