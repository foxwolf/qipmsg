// This file is part of QIpMsg.
//
// QIpMsg is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef THEMENAME_H
#define THEMENAME_H

#include <QString>

// Stored in qipmsg.ini [Appearance] theme. "system" follows the OS on
// Qt 6.5+ and otherwise uses the window-color lightness at startup.
inline QString normalizeThemeName(const QString &raw)
{
    const QString theme = raw.trimmed().toLower();
    if (theme == QLatin1String("dark") || theme == QLatin1String("system")) {
        return theme;
    }
    return QStringLiteral("light");
}

#endif // !THEMENAME_H
