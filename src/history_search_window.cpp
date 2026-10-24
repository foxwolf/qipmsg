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

#include "history_search_window.h"
#include "constants.h"
#include "global.h"
#include "message_store.h"
#include "window_manager.h"

#include <QAbstractItemView>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

HistorySearchWindow::HistorySearchWindow(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    createUi();
    setWindowTitle(tr("Find in history"));
    if (Global::iconSet.contains("normal")) {
        setWindowIcon(*Global::iconSet.value("normal"));
    }
}

QSize HistorySearchWindow::sizeHint() const
{
    return QSize(640, 420);
}

void HistorySearchWindow::createUi()
{
    m_queryEdit = new QLineEdit;
    m_queryEdit->setPlaceholderText(tr("Search all local chat history"));
    m_searchButton = new QPushButton(tr("&Find"));
    connect(m_searchButton, SIGNAL(clicked()), this, SLOT(runSearch()));
    connect(m_queryEdit, SIGNAL(returnPressed()), this, SLOT(runSearch()));

    QHBoxLayout *queryLayout = new QHBoxLayout;
    queryLayout->addWidget(m_queryEdit);
    queryLayout->addWidget(m_searchButton);

    m_resultTable = new QTableWidget(0, 4);
    QStringList labels;
    labels << tr("Time") << tr("Peer") << tr("Dir") << tr("Text");
    m_resultTable->setHorizontalHeaderLabels(labels);
    m_resultTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_resultTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_resultTable->verticalHeader()->hide();
    m_resultTable->horizontalHeader()->setStretchLastSection(true);
    connect(m_resultTable, SIGNAL(cellDoubleClicked(int,int)),
            this, SLOT(openSelected()));

    m_statusLabel = new QLabel(tr("Type a keyword and press Find."));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(queryLayout);
    mainLayout->addWidget(m_resultTable, 1);
    mainLayout->addWidget(m_statusLabel);
    setLayout(mainLayout);

    m_queryEdit->setFocus();
}

void HistorySearchWindow::runSearch()
{
    const QString query = m_queryEdit->text().trimmed();
    m_resultTable->setRowCount(0);
    if (query.isEmpty()) {
        m_statusLabel->setText(tr("Enter a search keyword."));
        return;
    }

    const QList<ChatRecord> records =
        Global::messageStore->search(query, HISTORY_SEARCH_LIMIT);
    for (int i = 0; i < records.size(); ++i) {
        const ChatRecord &record = records.at(i);
        const int row = m_resultTable->rowCount();
        m_resultTable->insertRow(row);
        m_resultTable->setItem(row, 0, new QTableWidgetItem(
                    record.timestamp.toString("yyyy-MM-dd hh:mm:ss")));
        QTableWidgetItem *peerItem = new QTableWidgetItem(record.peerName);
        peerItem->setData(Qt::UserRole, record.peerIp);
        m_resultTable->setItem(row, 1, peerItem);
        m_resultTable->setItem(row, 2, new QTableWidgetItem(
                    record.direction == QLatin1String("out")
                    ? tr("To") : tr("From")));
        QString preview = record.text;
        preview.replace('\n', ' ');
        if (preview.size() > 120) {
            preview = preview.left(120) + "...";
        }
        m_resultTable->setItem(row, 3, new QTableWidgetItem(preview));
    }
    m_statusLabel->setText(tr("Found %1 message(s). Double-click to open chat.")
                           .arg(records.size()));
}

void HistorySearchWindow::openSelected()
{
    const int row = m_resultTable->currentRow();
    if (row < 0) {
        return;
    }
    QTableWidgetItem *peerItem = m_resultTable->item(row, 1);
    if (!peerItem) {
        return;
    }
    const QString ip = peerItem->data(Qt::UserRole).toString();
    if (!ip.isEmpty()) {
        Global::windowManager->openChat(ip, true);
    }
}
