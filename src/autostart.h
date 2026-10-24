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

#ifndef AUTOSTART_H
#define AUTOSTART_H

#include <QString>

// Opt-in login start. Nothing is registered until the preference is turned on.
class AutoStart
{
public:
    static bool supported();
    static bool apply(bool enabled);
    static QString describe();

    // Text writers are pure so the self-check can cover them without touching
    // the user profile.
    static QString xdgDesktopText(const QString &exePath);
    static QString macAgentText(const QString &exePath);

    static QString xdgFileName();
    static QString macFileName();
    static QString windowsValueName();
};

#endif // !AUTOSTART_H
