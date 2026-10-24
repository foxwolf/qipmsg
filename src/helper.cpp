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

#include <QApplication>
#include <QStyle>
#include <QLibraryInfo>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QFont>
#include <QPainter>
#include <QPropertyAnimation>
#include <QWidget>
#include <QProcess>
#include <QProcessEnvironment>
#include <QTextStream>
#include <QSysInfo>
#include <QRegularExpression>
#include <QtGlobal>
#include <QStyleFactory>
#include <QImageReader>
#include <QUrl>

#include "helper.h"
#include "constants.h"
#include "global.h"
#include "preferences.h"

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
        if (!m_internalLogFile.open(QIODevice::WriteOnly)) {
            return;
        }
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

bool Helper::isImageFile(const QString &path)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    return suffix == QLatin1String("png")
        || suffix == QLatin1String("jpg")
        || suffix == QLatin1String("jpeg")
        || suffix == QLatin1String("gif")
        || suffix == QLatin1String("bmp")
        || suffix == QLatin1String("webp");
}

QStringList Helper::imageSearchDirs()
{
    QStringList dirs;
    dirs << screenshotDir() << defaultRecvPath();
    if (Global::preferences) {
        if (!Global::preferences->autoRecvPath.isEmpty()) {
            dirs.prepend(Global::preferences->autoRecvPath);
        }
        if (!Global::preferences->lastSaveFilePath.isEmpty()) {
            dirs << Global::preferences->lastSaveFilePath;
        }
    }
    dirs.removeDuplicates();
    return dirs;
}

QString Helper::resolveImagePath(const QString &nameOrPath,
                                 const QStringList &searchDirs)
{
    const QString trimmed = nameOrPath.trimmed();
    if (trimmed.isEmpty()) {
        return QString();
    }

    QString path = trimmed;
    if (path.startsWith(QLatin1String("file://"))) {
        path = QUrl(path).toLocalFile();
    }

    const QFileInfo info(path);
    if (info.isAbsolute() && info.isFile() && isImageFile(path)) {
        return info.absoluteFilePath();
    }

    const QString name = info.fileName();
    if (!isImageFile(name)) {
        return QString();
    }

    foreach (const QString &dir, searchDirs) {
        if (dir.isEmpty()) {
            continue;
        }
        const QString candidate = QDir(dir).filePath(name);
        if (QFileInfo::exists(candidate) && isImageFile(candidate)) {
            return QFileInfo(candidate).absoluteFilePath();
        }
    }
    return QString();
}

QPixmap Helper::imageThumbnail(const QString &path, int maxWidth, int maxHeight)
{
    QImageReader reader(path);
    reader.setAutoTransform(true);
    if (!reader.canRead()) {
        return QPixmap();
    }

    QSize size = reader.size();
    if (size.isValid()) {
        size.scale(maxWidth, maxHeight, Qt::KeepAspectRatio);
        reader.setScaledSize(size);
    }

    const QImage image = reader.read();
    if (image.isNull()) {
        return QPixmap();
    }
    if (!size.isValid()) {
        return QPixmap::fromImage(image.scaled(maxWidth, maxHeight,
                    Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    return QPixmap::fromImage(image);
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

void Helper::applyAppTheme(QApplication *app)
{
    if (!app) {
        return;
    }

    if (QStyle *fusion = QStyleFactory::create("Fusion")) {
        app->setStyle(fusion);
    }

    QFont font = app->font();
#ifdef Q_OS_WIN
    font.setFamily("Microsoft YaHei UI");
#elif defined(Q_OS_MAC)
    font.setFamily("PingFang SC");
#endif
    font.setPixelSize(13);
    app->setFont(font);

    QFile file(":/theme.qss");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app->setStyleSheet(QString::fromUtf8(file.readAll()));
    }
}

QColor Helper::avatarColor(const QString &name)
{
    static const QColor palette[] = {
        QColor(0x25, 0x63, 0xEB),
        QColor(0x7C, 0x3A, 0xED),
        QColor(0x0D, 0x94, 0x88),
        QColor(0xD9, 0x77, 0x06),
        QColor(0xDC, 0x26, 0x26),
        QColor(0xDB, 0x27, 0x77),
        QColor(0x4F, 0x46, 0xE5),
        QColor(0x05, 0x96, 0x69)
    };
    const uint hash = qHash(name.isEmpty() ? QString("?") : name);
    return palette[hash % 8];
}

QColor Helper::presenceColor(int presence)
{
    switch (presence) {
    case PRESENCE_AWAY:
        return QColor(0xF5, 0x9E, 0x0B);
    case PRESENCE_STALE:
        return QColor(0x9C, 0xA3, 0xAF);
    default:
        return QColor(0x22, 0xC5, 0x5E);
    }
}

QPixmap Helper::avatarPixmap(const QString &letter, int size,
                             const QColor &fill, const QColor &status)
{
    if (size < 16) {
        size = 16;
    }

    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(fill);
    p.setPen(Qt::NoPen);
    const int inset = 1;
    p.drawEllipse(inset, inset, size - inset * 2, size - inset * 2);

    p.setPen(Qt::white);
    QFont font = p.font();
    font.setBold(true);
    font.setPixelSize(qMax(10, size * 12 / 36));
    p.setFont(font);
    QString ch = letter.trimmed();
    ch = ch.isEmpty() ? QString("?") : ch.left(1);
    p.drawText(pix.rect(), Qt::AlignCenter, ch);

    if (status.isValid()) {
        const int dot = qMax(8, size / 4);
        const QRect dotRect(size - dot - 1, size - dot - 1, dot, dot);
        p.setBrush(Qt::white);
        p.drawEllipse(dotRect.adjusted(-1, -1, 1, 1));
        p.setBrush(status);
        p.drawEllipse(dotRect);
    }
    return pix;
}

