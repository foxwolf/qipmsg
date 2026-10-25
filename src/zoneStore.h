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

#ifndef ZONESTORE_H
#define ZONESTORE_H

#include <QString>
#include <QStringList>

// Local markdown and text notes under ~/.qipmsg/zone/.
// This is not FeiQ's zone/*.db blog and nothing here is sent to peers.

class ZoneStore {
public:
    explicit ZoneStore(const QString &root = QString());

    QString rootPath() const;
    bool ensureRoot(QString *error);
    QStringList noteNames() const;
    static bool isNoteName(const QString &name);
    QString createNote(const QString &title, QString *error);
    QString readNote(const QString &name, QString *error) const;
    bool writeNote(const QString &name, const QString &body, QString *error);
    bool removeNote(const QString &name, QString *error);

    static const int maxNoteBytes;

private:
    QString notePath(const QString &name, QString *error) const;

    QString m_root;
};

#endif // !ZONESTORE_H
