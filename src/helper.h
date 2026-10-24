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

#ifndef HELPER_H
#define HELPER_H

#include <QRegularExpression>
#include <QFile>
#include <QFontMetrics>
#include <QDateTime>
#include <QColor>
#include <QPixmap>

class QWidget;
class QApplication;

class Helper {
public:
    static void setAppPath(QString path);
    static QString appPath();

    static QString appHomePath();

    static QString lockFile();

    static void setIniPath(QString path);
    static QString iniPath();

    static QString translationPath();
    static QString qtTranslationPath();

    static QString soundPath();
    static QString iconPath();

    static QString loginName();
    static QString hostname();

    static QString getEnvironmentVariable(const QRegularExpression &regExp);

    static qint64 dateTimeToSecsSinceEpoch(const QDateTime &dt);
    static int fontTextWidth(const QFontMetrics &fm, const QString &text);

    static void setPacketNo(qint64 n);
    static QString packetNoString();
    static qint64 packetNo();

    static void setInternalLogFileName(QString filePath);
    static void writeInternalLog(QString line);

    static QString fileCountString(int fileCount);
    static QString sizeStringUnit(double size = 0.0, QString sep = "");
    static QString secondStringUnit(int second);

    static QString defaultRecvPath();
    static QString screenshotDir();
    static QString uniqueFileName(const QString &dir, const QString &name);
    static QString shakeToken();
    static bool isShakeText(const QString &text);
    static void shakeWidget(QWidget *widget);

    static void applyAppTheme(QApplication *app);
    static QColor avatarColor(const QString &name);
    static QColor presenceColor(int presence);
    static QPixmap avatarPixmap(const QString &letter, int size,
                                const QColor &fill,
                                const QColor &status = QColor());

private:
    static void openInternalLogFile();

    static QString m_appPath;
    static QString m_iniPath;
    static qint64 m_packetNo;
    static QFile m_internalLogFile;
    static QString m_internalLogFileName;
};

#endif // !HELPER_H
