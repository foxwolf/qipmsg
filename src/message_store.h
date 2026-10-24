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

#ifndef MESSAGE_STORE_H
#define MESSAGE_STORE_H

#include "constants.h"

#include <QDateTime>
#include <QList>
#include <QMutex>
#include <QString>

class ChatRecord
{
public:
    QDateTime timestamp;
    QString peerIp;
    QString peerName;
    QString direction;
    QString text;
};

class MessageStore
{
public:
    MessageStore();

    bool append(const ChatRecord &record);
    QList<ChatRecord> loadHistory(const QString &peerIp,
                                  int limit = CHAT_HISTORY_LOAD_LIMIT) const;
    QString historyDir() const;

private:
    QString filePathForIp(const QString &ip) const;
    QString sanitizeIp(const QString &ip) const;
    bool ensureHistoryDir() const;

    mutable QMutex m_lock;
};

#endif // !MESSAGE_STORE_H
