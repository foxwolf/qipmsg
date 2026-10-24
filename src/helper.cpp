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

#include <QLibraryInfo>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QPropertyAnimation>
#include <QWidget>
#include <QProcess>
#include <QProcessEnvironment>
#include <QTextStream>
#include <QSysInfo>
#include <QRegularExpression>
#include <QtGlobal>

#include "helper.h"
#include "constants.h"

QString Helper::m_iniPath;
QString Helper::m_appPath;
qint64 Helper::m_packetNo;
QFile Helper::m_internalLogFile;
QString Helper::m_internalLogFileName;

void Helper::setAppPath(QString path)
{
    m_appPath = path;
}

QString Helper::appPath()
{
    return m_appPath;
}

void Helper::setIniPath(QString path)
{
    m_iniPath = path;
}

QString Helper::iconPath()
{
#ifdef ICON_PATH
    QString path = QString(ICON_PATH);
    if (!path.isEmpty()) {
        return path;
    } else {
        return m_appPath + "/icons";
    }
#else
    return m_appPath + "/icons";
#endif
}

QString Helper::appHomePath()
{
    return QDir::homePath() + "/.qipmsg";
}

QString Helper::lockFile()
{
    return appHomePath() + "/qipmsg.lock";
}

QString Helper::iniPath()
{
    if (!m_iniPath.isEmpty()) {
        return m_iniPath;
    } else {
        if (QFile::exists(appHomePath())) {
            return appHomePath();
        }
    }
    return "";
}

QString Helper::loginName()
{
    const QRegularExpression::PatternOptions opts
        = QRegularExpression::CaseInsensitiveOption;
    QString name = getEnvironmentVariable(QRegularExpression("^LOGNAME$", opts));
    if (name.isEmpty()) {
        name = getEnvironmentVariable(QRegularExpression("^USER$", opts));
    }
    if (name.isEmpty()) {
        name = getEnvironmentVariable(QRegularExpression("^USERNAME$", opts));
    }
    return name;
}

QString Helper::hostname()
{
    QString name = getEnvironmentVariable(QRegularExpression("^HOSTNAME$"));

    // QSysInfo::machineHostName() works across Windows / Linux / macOS.
    if (name.isEmpty()) {
        name = QSysInfo::machineHostName();
    }

    if (name.isEmpty()) {
        name = QObject::tr("Unknown hostname");
    }

    return name;
}

QString Helper::translationPath()
{
#ifdef TRANSLATION_PATH
    QString path = QString(TRANSLATION_PATH);
    if (!path.isEmpty()) {
        return path;
    } else {
        return appPath() + "/translations";
    }
#else
    return appPath() + "/translations";
#endif
}

QString Helper::qtTranslationPath()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#else
    return QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif
}

QString Helper::getEnvironmentVariable(const QRegularExpression &regExp)
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QStringList keys = env.keys().filter(regExp);

    if (!keys.isEmpty()) {
        return env.value(keys.first());
    }

    return QString();
}

void Helper::setPacketNo(qint64 n)
{
    m_packetNo = n;
}

QString Helper::packetNoString()
{
    ++m_packetNo;
    return QString("%1").arg(m_packetNo);
}

qint64 Helper::packetNo()
{
    ++m_packetNo;
    return m_packetNo;
}

void Helper::setInternalLogFileName(QString filePath)
{
    m_internalLogFileName = filePath;
}

void Helper::openInternalLogFile()
{
    if (!m_internalLogFile.isOpen()) {
        m_internalLogFile.setFileName(m_internalLogFileName);
        m_internalLogFile.open(QIODevice::WriteOnly);
    }
}

void Helper::writeInternalLog(QString line)
{
    openInternalLogFile();
    QTextStream ts(&m_internalLogFile);

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    ts << line << Qt::endl;
#else
    ts << line << endl;
#endif
}

QString Helper::soundPath()
{
#ifdef SOUND_PATH
    QString path = QString(SOUND_PATH);
    if (!path.isEmpty()) {
        return path;
    } else {
        return m_appPath + "/sounds";
    }
#else
    return m_appPath + "/sounds";
#endif
}

QString Helper::fileCountString(int fileCount)
{
    if (fileCount <= 1) {
        return QString(QObject::tr("%1 file")).arg(fileCount);
    } else {
        return QString(QObject::tr("%1 files")).arg(fileCount);
    }

    // this should never be reached
    return QString();
}

QString Helper::sizeStringUnit(double size, QString sep)
{
    QString s;
    if (size >= ONE_MB) {
        s = QString("%1%2MB")
            .arg(size/ONE_MB, 0, 'f', 1)
            .arg(sep);
    } else {
        s = QString("%1%2KB")
            .arg(qMax(size/ONE_KB, 1.0), 0, 'f', 0)
            .arg(sep);
    }

    return s;
}

QString Helper::secondStringUnit(int second)
{
    if (second <= 1) {
        return QString(QObject::tr("%1 second")).arg(second);
    } else {
        return QString(QObject::tr("%1 seconds")).arg(second);
    }

    // this should never be reached
    return QString();
}

qint64 Helper::dateTimeToSecsSinceEpoch(const QDateTime &dt)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return dt.toSecsSinceEpoch();
#else
    return dt.toTime_t();
#endif
}

int Helper::fontTextWidth(const QFontMetrics &fm, const QString &text)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    return fm.horizontalAdvance(text);
#else
    return fm.width(text);
#endif
}

QString Helper::defaultRecvPath()
{
    return appHomePath() + "/recv";
}

QString Helper::screenshotDir()
{
    return appHomePath() + "/screenshots";
}

QString Helper::uniqueFileName(const QString &dir, const QString &name)
{
    QDir d(dir);
    if (!d.exists(name)) {
        return name;
    }

    const QFileInfo info(name);
    const QString base = info.completeBaseName();
    const QString suffix = info.suffix();
    for (int i = 1; i < 10000; ++i) {
        const QString candidate = suffix.isEmpty()
                ? QString("%1 (%2)").arg(base).arg(i)
                : QString("%1 (%2).%3").arg(base).arg(i).arg(suffix);
        if (!d.exists(candidate)) {
            return candidate;
        }
    }
    return name;
}

QString Helper::shakeToken()
{
    return QString::fromUtf8(SHAKE_TOKEN);
}

bool Helper::isShakeText(const QString &text)
{
    const QString t = text.trimmed();
    if (t.isEmpty()) {
        return false;
    }
    if (t == QLatin1String(SHAKE_TOKEN)
            || t == QLatin1String("{shake}")
            || t.compare(QLatin1String("SHAKE"), Qt::CaseInsensitive) == 0
            || t == QString::fromUtf8("窗口抖动")
            || t == QLatin1String("#qipmsg-shake#")) {
        return true;
    }
    if (t.contains(QChar(0x1b))
            && t.contains(QLatin1String("SHAKE"), Qt::CaseInsensitive)) {
        return true;
    }
    return false;
}

void Helper::shakeWidget(QWidget *widget)
{
    if (!widget) {
        return;
    }
    widget->show();
    widget->raise();
    widget->activateWindow();

    const QPoint origin = widget->pos();
    QPropertyAnimation *anim = new QPropertyAnimation(widget, "pos", widget);
    anim->setDuration(420);
    anim->setKeyValueAt(0.00, origin);
    anim->setKeyValueAt(0.12, origin + QPoint(10, 0));
    anim->setKeyValueAt(0.24, origin + QPoint(-10, 0));
    anim->setKeyValueAt(0.36, origin + QPoint(8, 0));
    anim->setKeyValueAt(0.48, origin + QPoint(-8, 0));
    anim->setKeyValueAt(0.60, origin + QPoint(5, 0));
    anim->setKeyValueAt(0.72, origin + QPoint(-5, 0));
    anim->setKeyValueAt(0.86, origin + QPoint(2, 0));
    anim->setKeyValueAt(1.00, origin);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

