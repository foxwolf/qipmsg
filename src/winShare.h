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

#ifndef WINSHARE_H
#define WINSHARE_H

#include <QString>

// Open \\ipv4\ in the system file manager. Windows asks for SMB credentials.
// This program does not collect a share password. Other systems do not open it.

enum WinShareResult {
    winShareOk = 0,
    winShareBadAddress,
    winShareNotWindows,
    winShareOsFailed
};

struct WinShareOpen {
    WinShareResult result;
    int osCode;
    QString unc;
};

class WinShare {
public:
    static bool platformSupports();
    static bool isSafeIpv4(const QString &text, QString *normalized);
    static WinShareOpen openNeighborhood(const QString &ip);
    static QString describe(const WinShareOpen &opened);
};

#endif // !WINSHARE_H
