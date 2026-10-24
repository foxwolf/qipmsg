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
#include <QDir>
#include <QFileInfo>
#include <QLocale>
#include <QSet>

#include "translator.h"
#include "helper.h"
#include "constants.h"

Q_GLOBAL_STATIC(Translator, translator);

Translator::Translator()
{
    qApp->installTranslator(&appTrans_);
    qApp->installTranslator(&qtTrans_);
    qApp->installTranslator(&qtBaseTrans_);
}

Translator* Translator::instance()
{
    return translator();
}

bool Translator::loadCatalog(QTranslator &t, QString name, QString locale,
                      QString dir)
{
    QString s = name + "_" + locale;
    bool r = t.load(s, dir);
    if (r) {
        qDebug("Translator::loadCatalog: successfully load %s from %s",
               s.toUtf8().data(), dir.toUtf8().data());
    } else {
        qDebug("Translator::loadCatalog: can't load %s from %s",
               s.toUtf8().data(), dir.toUtf8().data());
    }

    return r;
}

QStringList Translator::catalogDirs() const
{
    QStringList dirs;
    const QString configured = Helper::translationPath();
    if (!configured.isEmpty()) {
        dirs << configured;
    }
    if (!Helper::appPath().isEmpty()) {
        dirs << Helper::appPath() + "/translations";
        dirs << Helper::appPath();
    }
    dirs << QDir::currentPath() + "/translations";
    dirs << QDir::currentPath();
    // Embedded snapshot is last so a freshly built side-by-side catalog wins.
    dirs << QString(":/translations");

    QStringList unique;
    QSet<QString> seen;
    foreach (const QString &dir, dirs) {
        if (dir.isEmpty() || seen.contains(dir)) {
            continue;
        }
        seen.insert(dir);
        unique << dir;
    }
    return unique;
}

QStringList Translator::localeCandidates(const QString &requested) const
{
    QStringList locales;
    if (!requested.isEmpty()) {
        locales << requested;
        const int under = requested.indexOf('_');
        if (under > 0) {
            locales << requested.left(under);
        }
    }
    // Product default: Simplified Chinese UI.
    locales << "zh_CN" << "zh";

    QStringList unique;
    foreach (const QString &locale, locales) {
        if (!locale.isEmpty() && !unique.contains(locale)) {
            unique << locale;
        }
    }
    return unique;
}

void Translator::load(QString locale)
{
    if (locale.isEmpty()) {
        const QString sys = QLocale::system().name();
        if (sys.startsWith(QLatin1String("zh"))) {
            locale = sys;
        } else {
            locale = QString("zh_CN");
        }
    }

    const QStringList dirs = catalogDirs();
    const QStringList locales = localeCandidates(locale);

    QStringList qtDirs = dirs;
    qtDirs << Helper::qtTranslationPath();

    bool qtLoaded = false;
    bool qtBaseLoaded = false;
    foreach (const QString &loc, locales) {
        foreach (const QString &dir, qtDirs) {
            if (!qtLoaded && loadCatalog(qtTrans_, "qt", loc, dir)) {
                qtLoaded = true;
            }
            if (!qtBaseLoaded && loadCatalog(qtBaseTrans_, "qtbase", loc, dir)) {
                qtBaseLoaded = true;
            }
            if (qtLoaded && qtBaseLoaded) {
                break;
            }
        }
        if (qtLoaded && qtBaseLoaded) {
            break;
        }
    }

    foreach (const QString &loc, locales) {
        foreach (const QString &dir, dirs) {
            if (loadCatalog(appTrans_, PROGRAM, loc, dir)) {
                return;
            }
        }
    }
}
