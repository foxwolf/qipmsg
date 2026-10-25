// This file is part of QIpMsg.
//
// QIpMsg is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef THEMENAME_H
#define THEMENAME_H

#include <QString>

// Stored in qipmsg.ini [Appearance] theme.
// "light" / "dark" / "system", or "skin:<id>" for a pack under
// ~/.qipmsg/skins/. "system" follows the OS on Qt 6.5+ and otherwise
// uses the window-color lightness at startup.

inline bool skinIdOk(const QString &id)
{
    if (id.isEmpty() || id.size() > 64) {
        return false;
    }
    if (id == QLatin1String(".") || id == QLatin1String("..")) {
        return false;
    }
    for (int i = 0; i < id.size(); ++i) {
        const QChar c = id.at(i);
        if (c.isLetterOrNumber() || c == QLatin1Char('-') || c == QLatin1Char('.')) {
            continue;
        }
        return false;
    }
    return true;
}

inline QString themeSkinId(const QString &theme)
{
    const QString prefix = QStringLiteral("skin:");
    if (theme.size() <= prefix.size()) {
        return QString();
    }
    if (theme.left(prefix.size()).compare(prefix, Qt::CaseInsensitive) != 0) {
        return QString();
    }
    const QString id = theme.mid(prefix.size()).trimmed();
    return skinIdOk(id) ? id : QString();
}

inline QString normalizeThemeName(const QString &raw)
{
    const QString trimmed = raw.trimmed();
    const QString theme = trimmed.toLower();
    if (theme == QLatin1String("dark") || theme == QLatin1String("system")) {
        return theme;
    }
    const QString skinId = themeSkinId(trimmed);
    if (!skinId.isEmpty()) {
        return QStringLiteral("skin:") + skinId;
    }
    return QStringLiteral("light");
}

#endif // !THEMENAME_H
