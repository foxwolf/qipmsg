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

#ifndef GROUPLOG_H
#define GROUPLOG_H

#include <QList>
#include <QString>

// Append-only discussion notes under ~/.qipmsg/grouplog/<id>.jsonl.
// Not a FeiQ room log and not Qt Sql. Each machine keeps its own file.
// Telling members is a separate plain SENDMSG, not a sync protocol.

class GroupLogEntry
{
public:
    QString when;
    QString author;
    QString text;
};

class GroupLog
{
public:
    static void setStorageDirForTest(const QString &dir);
    static QString filePath(const QString &groupId);
    static bool append(const QString &groupId, const QString &author,
                       const QString &text, QString *error);
    static QList<GroupLogEntry> load(const QString &groupId, int limit);
    static QString exportPlain(const QString &groupId);
};

#endif // !GROUPLOG_H
