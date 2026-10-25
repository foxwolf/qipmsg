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

#include "winShare.h"

#include <QCoreApplication>
#include <QHostAddress>
#include <QStringList>

#ifdef Q_OS_WIN
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>
#endif

bool WinShare::platformSupports()
{
#ifdef Q_OS_WIN
    return true;
#else
    return false;
#endif
}

bool WinShare::isSafeIpv4(const QString &text, QString *normalized)
{
    if (normalized) {
        normalized->clear();
    }
    if (text.size() < 7 || text.size() > 15) {
        return false;
    }
    const QStringList parts = text.split(QLatin1Char('.'));
    if (parts.size() != 4) {
        return false;
    }
    int octets[4];
    for (int i = 0; i < 4; ++i) {
        const QString part = parts.at(i);
        if (part.isEmpty() || part.size() > 3) {
            return false;
        }
        if (part.size() > 1 && part.at(0) == QLatin1Char('0')) {
            return false;
        }
        for (int c = 0; c < part.size(); ++c) {
            if (!part.at(c).isDigit()) {
                return false;
            }
        }
        bool ok = false;
        const int value = part.toInt(&ok, 10);
        if (!ok || value < 0 || value > 255) {
            return false;
        }
        octets[i] = value;
    }
    if (octets[0] == 0 && octets[1] == 0 && octets[2] == 0 && octets[3] == 0) {
        return false;
    }
    // Class D/E and the limited broadcast are not share hosts.
    if (octets[0] >= 224) {
        return false;
    }
    const QHostAddress addr(text);
    if (addr.protocol() != QAbstractSocket::IPv4Protocol) {
        return false;
    }
    if (addr.toString() != text) {
        return false;
    }
    if (normalized) {
        *normalized = text;
    }
    return true;
}

WinShareOpen WinShare::openNeighborhood(const QString &ip)
{
    WinShareOpen opened;
    opened.result = winShareBadAddress;
    opened.osCode = 0;
    QString normalized;
    if (!isSafeIpv4(ip, &normalized)) {
        return opened;
    }
    opened.unc = QStringLiteral("\\\\") + normalized + QStringLiteral("\\");
    if (!platformSupports()) {
        opened.result = winShareNotWindows;
        return opened;
    }
#ifdef Q_OS_WIN
    const quintptr code = reinterpret_cast<quintptr>(ShellExecuteW(
                0, L"open",
                reinterpret_cast<LPCWSTR>(opened.unc.utf16()),
                0, 0, SW_SHOWNORMAL));
    if (code > 32) {
        opened.result = winShareOk;
        return opened;
    }
    opened.result = winShareOsFailed;
    opened.osCode = static_cast<int>(code);
#else
    opened.result = winShareNotWindows;
#endif
    return opened;
}

QString WinShare::describe(const WinShareOpen &opened)
{
    switch (opened.result) {
    case winShareOk:
        return QString();
    case winShareBadAddress:
        return QCoreApplication::translate(
                    "WinShare",
                    "The address is not a plain IPv4 address.");
    case winShareNotWindows:
        return QCoreApplication::translate(
                    "WinShare",
                    "Windows network neighborhood is only available on Windows.");
    case winShareOsFailed:
        return QCoreApplication::translate(
                    "WinShare",
                    "Windows could not open %1 (code %2). If the share needs a password, Windows asks for it. This program does not collect share credentials.")
                .arg(opened.unc).arg(opened.osCode);
    }
    return QCoreApplication::translate("WinShare", "The share could not be opened.");
}
