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

#include "autostart.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QtGlobal>

namespace {

QString xmlEscape(const QString &text)
{
    QString out = text;
    out.replace(QLatin1String("&"), QLatin1String("&amp;"));
    out.replace(QLatin1String("<"), QLatin1String("&lt;"));
    out.replace(QLatin1String(">"), QLatin1String("&gt;"));
    return out;
}

QString quoteDesktopExec(const QString &exePath)
{
    QString escaped = exePath;
    escaped.replace(QLatin1String("\\"), QLatin1String("\\\\"));
    escaped.replace(QLatin1String("\""), QLatin1String("\\\""));
    escaped.replace(QLatin1String("`"), QLatin1String("\\`"));
    escaped.replace(QLatin1String("$"), QLatin1String("\\$"));
    return QStringLiteral("\"%1\"").arg(escaped);
}

QString xdgAutostartDir()
{
    const QString configured = qEnvironmentVariable("XDG_CONFIG_HOME");
    const QString base = configured.isEmpty()
            ? QDir::homePath() + QStringLiteral("/.config")
            : configured;
    return base + QStringLiteral("/autostart");
}

QString macAgentDir()
{
    return QDir::homePath() + QStringLiteral("/Library/LaunchAgents");
}

bool writeTextFile(const QString &dirPath, const QString &fileName,
                   const QString &body)
{
    QDir dir(dirPath);
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        qWarning("AutoStart: cannot create %s", qUtf8Printable(dirPath));
        return false;
    }
    QFile file(dir.filePath(fileName));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qWarning("AutoStart: cannot write %s", qUtf8Printable(file.fileName()));
        return false;
    }
    const QByteArray bytes = body.toUtf8();
    return file.write(bytes) == bytes.size();
}

bool removeFileIfPresent(const QString &path)
{
    if (!QFile::exists(path)) {
        return true;
    }
    if (!QFile::remove(path)) {
        qWarning("AutoStart: cannot remove %s", qUtf8Printable(path));
        return false;
    }
    return true;
}

QString applicationPath()
{
    return QCoreApplication::applicationFilePath();
}

#if defined(Q_OS_WIN)

bool applyWindows(bool enabled)
{
    QSettings run(QStringLiteral(
                      "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
                  QSettings::NativeFormat);
    const QString key = AutoStart::windowsValueName();
    if (!enabled) {
        run.remove(key);
        run.sync();
        return run.status() == QSettings::NoError;
    }

    const QString exe = applicationPath();
    if (exe.isEmpty()) {
        qWarning("AutoStart: empty application path");
        return false;
    }
    QString command = QDir::toNativeSeparators(exe);
    if (command.contains(QLatin1Char(' ')) || command.contains(QLatin1Char('\t'))) {
        command = QStringLiteral("\"%1\"").arg(command);
    }
    run.setValue(key, command);
    run.sync();
    return run.status() == QSettings::NoError;
}

#elif defined(Q_OS_MACOS)

bool applyMac(bool enabled)
{
    const QString path = macAgentDir() + QLatin1Char('/') + AutoStart::macFileName();
    if (!enabled) {
        return removeFileIfPresent(path);
    }
    const QString exe = applicationPath();
    if (exe.isEmpty()) {
        qWarning("AutoStart: empty application path");
        return false;
    }
    return writeTextFile(macAgentDir(), AutoStart::macFileName(),
                         AutoStart::macAgentText(exe));
}

#elif defined(Q_OS_UNIX)

bool applyXdg(bool enabled)
{
    const QString path = xdgAutostartDir() + QLatin1Char('/') + AutoStart::xdgFileName();
    if (!enabled) {
        return removeFileIfPresent(path);
    }
    const QString exe = applicationPath();
    if (exe.isEmpty()) {
        qWarning("AutoStart: empty application path");
        return false;
    }
    return writeTextFile(xdgAutostartDir(), AutoStart::xdgFileName(),
                         AutoStart::xdgDesktopText(exe));
}

#endif

} // namespace

bool AutoStart::supported()
{
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS) || defined(Q_OS_UNIX)
    return true;
#else
    return false;
#endif
}

bool AutoStart::apply(bool enabled)
{
#if defined(Q_OS_WIN)
    return applyWindows(enabled);
#elif defined(Q_OS_MACOS)
    return applyMac(enabled);
#elif defined(Q_OS_UNIX)
    return applyXdg(enabled);
#else
    Q_UNUSED(enabled);
    return !enabled;
#endif
}

QString AutoStart::describe()
{
#if defined(Q_OS_WIN)
    return QCoreApplication::translate(
                "AutoStart",
                "Off until you enable it. The next login starts QIpMsg from the current-user Run key (QIpMsg). FeiQ's own startup entry is left alone.");
#elif defined(Q_OS_MACOS)
    return QCoreApplication::translate(
                "AutoStart",
                "Off until you enable it. A per-user LaunchAgent starts QIpMsg at the next login. It is not loaded into the current session.");
#elif defined(Q_OS_UNIX)
    return QCoreApplication::translate(
                "AutoStart",
                "Off until you enable it. The next graphical login starts QIpMsg from an XDG autostart entry under your config directory.");
#else
    return QCoreApplication::translate(
                "AutoStart",
                "Start with the system is not available on this platform.");
#endif
}

QString AutoStart::xdgDesktopText(const QString &exePath)
{
    return QStringLiteral(
                "[Desktop Entry]\n"
                "Type=Application\n"
                "Version=1.0\n"
                "Name=QIpMsg\n"
                "Name[zh_CN]=飞鸽传书\n"
                "Comment=LAN messenger\n"
                "Exec=%1\n"
                "Icon=qipmsg\n"
                "Terminal=false\n"
                "Categories=Network;\n"
                "X-GNOME-Autostart-enabled=true\n"
                "X-QIpMsg-Autostart=true\n")
            .arg(quoteDesktopExec(exePath));
}

QString AutoStart::macAgentText(const QString &exePath)
{
    return QStringLiteral(
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" "
                "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
                "<plist version=\"1.0\">\n"
                "<dict>\n"
                "    <key>Label</key>\n"
                "    <string>org.qipmsg.login</string>\n"
                "    <key>ProgramArguments</key>\n"
                "    <array>\n"
                "        <string>%1</string>\n"
                "    </array>\n"
                "    <key>RunAtLoad</key>\n"
                "    <true/>\n"
                "    <key>LimitLoadToSessionType</key>\n"
                "    <string>Aqua</string>\n"
                "</dict>\n"
                "</plist>\n")
            .arg(xmlEscape(exePath));
}

QString AutoStart::xdgFileName()
{
    return QStringLiteral("qipmsg-autostart.desktop");
}

QString AutoStart::macFileName()
{
    return QStringLiteral("org.qipmsg.login.plist");
}

QString AutoStart::windowsValueName()
{
    return QStringLiteral("QIpMsg");
}
