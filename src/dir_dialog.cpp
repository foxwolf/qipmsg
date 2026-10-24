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

#include "dir_dialog.h"
#include "global.h"
#include "preferences.h"
#include "send_file_model.h"

#include <QtGui>
#include <QtCore>
#include <QtWidgets>
#include <QFileSystemModel>

DirDialog::DirDialog(SendFileModel *sendFileModel, QWidget *parent)
    : QDialog(parent), m_sendFileModel(sendFileModel)
{
    // Delete when close
    setAttribute(Qt::WA_DeleteOnClose, true);

    model = new QFileSystemModel(this);
    model->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    model->setRootPath(QDir::rootPath());
    tree = new QTreeView;

    tree->setModel(model);
    tree->hideColumn(1);    // Hide size column
    tree->hideColumn(2);    // Hide type column
    tree->hideColumn(3);    // Hide time column
    tree->header()->hide(); // Hide header

    selectionModel = new QItemSelectionModel(model);
    tree->setSelectionModel(selectionModel);

    // Expanded to the last send directory
    expandTree();
    QModelIndex lastIndex = model->index(Global::preferences->lastSendDirPath);
    // Scroll to last dir path
    tree->verticalScrollBar()
        ->setValue(lastIndex.row());

    // Set the default selected directory
    selectionModel->select(lastIndex,
            QItemSelectionModel::Select | QItemSelectionModel::Rows);

    QLabel *label = new QLabel(tr("Select folder to send"));

    createButtonLayout();
    createConnections();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(label);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(tree);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(buttonsLayout);

    setLayout(mainLayout);

    setWindowTitle(tr("Browser Folders"));
    adjustSize();
}

QSize DirDialog::sizeHint() const
{
    return QSize(400, 480);
}

void DirDialog::selectDir()
{
    QModelIndexList modelIndexList =
        selectionModel->selectedRows(0);

    foreach (QModelIndex ix, modelIndexList) {
        QFileInfo fi = model->fileInfo(ix);
        QString filePath = fi.absoluteFilePath();
        m_sendFileModel->addFile(filePath);
        Global::preferences->lastSendDirPath = fi.absoluteFilePath();
    }

    emit dirSelected();

    close();
}

void DirDialog::createConnections()
{
    connect(okButton, SIGNAL(clicked()),
            this, SLOT(selectDir()));
    connect(cancelButton, SIGNAL(clicked()),
            this, SLOT(close()));
}

void DirDialog::createButtonLayout()
{
    okButton = new QPushButton(tr("Ok"));
    cancelButton = new QPushButton(tr("Cancel"));

    buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(okButton);
    buttonsLayout->addSpacing(20);
    buttonsLayout->addWidget(cancelButton);
}

void DirDialog::expandTree()
{
    QDir dir(Global::preferences->lastSendDirPath);
    while (dir.cdUp()) {
        tree->expand(model->index(dir.absolutePath()));
    }
}

