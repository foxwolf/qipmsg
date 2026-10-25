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

#ifndef FAVORITEDIALOG_H
#define FAVORITEDIALOG_H

#include <QDialog>
#include <QString>

class QLineEdit;
class QListWidget;
class QPushButton;

class FavoriteDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FavoriteDialog(QWidget *parent = 0);

    void setContext(const QString &peerIp, const QString &peerName,
                    const QString &selectedText, const QString &peerNote);

private slots:
    void reload();
    void saveSelectedText();
    void saveFilePath();
    void savePeerNote();
    void openCurrent();
    void deleteCurrent();
    void updateActions();

private:
    QString selectedId() const;
    QString selectedPath() const;
    void rememberId(const QString &id);

    QString m_peerIp;
    QString m_peerName;
    QString m_selectedText;
    QString m_peerNote;
    QString m_keepId;
    QLineEdit *m_searchEdit;
    QListWidget *m_list;
    QPushButton *m_textButton;
    QPushButton *m_fileButton;
    QPushButton *m_noteButton;
    QPushButton *m_openButton;
    QPushButton *m_deleteButton;
};

#endif // !FAVORITEDIALOG_H
