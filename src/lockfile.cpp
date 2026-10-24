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

#include "lockfile.h"

#include <QLockFile>

Q_GLOBAL_STATIC(LockFile, lockFile);

LockFile::LockFile(QObject *parent)
    : QObject(parent)
{
}

LockFile *LockFile::instance()
{
    return lockFile();
}

bool LockFile::lock()
{
    if (m_lockFile && m_lockFile->isLocked()) {
        return true;
    }

    if (fileName.isEmpty()) {
        qWarning("LockFile::lock: lock file name is empty.");
        return false;
    }

    m_lockFile.reset(new QLockFile(fileName));
    if (!m_lockFile->tryLock()) {
        qWarning("LockFile::lock: can not lock %s.",
                 fileName.toUtf8().data());
        return false;
    }

    return true;
}

void LockFile::setLockFile(QString name)
{
    fileName = name;
}
