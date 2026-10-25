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

#ifndef SCHEDULEDIALOG_H
#define SCHEDULEDIALOG_H

#include <QDialog>

class QCheckBox;
class QComboBox;
class QDateTimeEdit;
class QLabel;
class QListWidget;
class QPlainTextEdit;
class QPushButton;

class ScheduleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScheduleDialog(const QString &preferredPeerIp, QWidget *parent = 0);

private slots:
    void reload();
    void applySelection();
    void saveCurrent();
    void deleteCurrent();
    void clearForm();

private:
    void fillPeers(const QString &selectedIp);
    QString selectedId() const;

    QString m_preferredPeerIp;
    QListWidget *m_list;
    QPlainTextEdit *m_textEdit;
    QDateTimeEdit *m_dueEdit;
    QCheckBox *m_notifyCheck;
    QComboBox *m_peerCombo;
    QLabel *m_hint;
    QPushButton *m_saveButton;
    QPushButton *m_deleteButton;
};

#endif // !SCHEDULEDIALOG_H
