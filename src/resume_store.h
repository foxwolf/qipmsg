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

#ifndef RESUME_STORE_H
#define RESUME_STORE_H

#include <QMap>
#include <QMutex>
#include <QString>

class ResumeRecord
{
public:
    QString key;
    QString ip;
    QString packetNo;
    int fileId;
    QString name;
    qint64 size;
    qint64 offset;
    QString saveDir;
};

class ResumeStore
{
public:
    ResumeStore();

    static QString makeKey(const QString &ip, const QString &packetNo,
                           int fileId, const QString &name, qint64 size);

    ResumeRecord lookup(const QString &key) const;
    void save(const ResumeRecord &record);
    void remove(const QString &key);

private:
    QString filePath() const;
    void load();
    void persist() const;

    mutable QMutex m_lock;
    QMap<QString, ResumeRecord> m_items;
};

#endif // !RESUME_STORE_H
