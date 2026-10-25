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

#ifndef DATABACKUP_H
#define DATABACKUP_H

#include <QCoreApplication>
#include <QString>

class QWidget;

// Zip (STORE, no Qt Sql) of private files under the qipmsg home directory.
// Received files and screenshots are left out on purpose.

class DataBackup
{
    Q_DECLARE_TR_FUNCTIONS(DataBackup)

public:
    static bool exportTree(const QString &root, const QString &zipPath,
                           QString *error, int *fileCount = 0);
    static bool importTree(const QString &root, const QString &zipPath,
                           QString *error, int *fileCount = 0);

    static void exportInteractive(QWidget *parent);
    static void importInteractive(QWidget *parent);
};

#endif // !DATABACKUP_H
