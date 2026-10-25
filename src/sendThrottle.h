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

#ifndef SENDTHROTTLE_H
#define SENDTHROTTLE_H

#include <functional>

#include <QtGlobal>

// Process-wide cap for outbound TCP file bytes. 0 means unlimited.
// The limit is shared by every SendFileThread so parallel sends
// stay under one KB/s budget.

class SendThrottle
{
public:
    static int clampKBps(int kbps);
    static void setLimitKBps(int kbps);
    static int limitKBps();
    static QString formatLimit(int kbps);

    // Account for nbytes just written. Sleeps outside the lock so a
    // cancel callback can run. Returns false when shouldAbort is true.
    static bool pace(qint64 nbytes,
                     const std::function<bool()> &shouldAbort
                     = std::function<bool()>());
};

#endif // !SENDTHROTTLE_H
