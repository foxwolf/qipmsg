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

#ifndef WINDOWSNAP_H
#define WINDOWSNAP_H

#include <QRect>
#include <QVector>
#include <QtGlobal>
#include <QtGui/qwindowdefs.h>

// Top-level window edges for the region-capture overlay.
// Windows uses EnumWindows. macOS uses CGWindowList (best effort).
// Linux uses libX11 at runtime when it is present; otherwise only the
// screen rectangle is returned. Coordinates are local to screenGlobal.

class WindowSnap
{
public:
    static QVector<QRect> topLevelRects(const QRect &screenGlobal, WId skip);
    static QRect snapSelection(const QRect &raw, const QVector<QRect> &targets,
                               int thresholdPx, bool disabled);
};

#endif // !WINDOWSNAP_H
