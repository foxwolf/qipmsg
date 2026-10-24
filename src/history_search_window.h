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

#ifndef HISTORY_SEARCH_WINDOW_H
#define HISTORY_SEARCH_WINDOW_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QTableWidget;
class QLabel;

class HistorySearchWindow : public QWidget
{
    Q_OBJECT

public:
    HistorySearchWindow(QWidget *parent = 0);
    virtual QSize sizeHint() const;

private slots:
    void runSearch();
    void openSelected();

private:
    void createUi();

    QLineEdit *m_queryEdit;
    QPushButton *m_searchButton;
    QTableWidget *m_resultTable;
    QLabel *m_statusLabel;
};

#endif // !HISTORY_SEARCH_WINDOW_H
