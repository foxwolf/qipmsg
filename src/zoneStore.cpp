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

#include "zoneStore.h"
#include "helper.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>

namespace {

const int kMaxNameChars = 80;
const int kMaxStemChars = 40;

QString trStore(const char *text)
{
    return QCoreApplication::translate("ZoneStore", text);
}

bool insideRoot(const QString &root, const QString &path)
{
    const QString rootClean = QDir::cleanPath(QFileInfo(root).absoluteFilePath());
    const QString pathClean = QDir::cleanPath(path);
#ifdef Q_OS_WIN
    const QString rootCmp = rootClean.toLower();
    const QString pathCmp = pathClean.toLower();
#else
    const QString rootCmp = rootClean;
    const QString pathCmp = pathClean;
#endif
    return pathCmp == rootCmp
            || pathCmp.startsWith(rootCmp + QLatin1Char('/'));
}

bool reservedStem(const QString &stem)
{
    const QString upper = stem.toUpper();
    if (upper == QLatin1String("CON") || upper == QLatin1String("PRN")
            || upper == QLatin1String("AUX") || upper == QLatin1String("NUL")) {
        return true;
    }
    if (upper.size() == 4
            && (upper.startsWith(QLatin1String("COM"))
                || upper.startsWith(QLatin1String("LPT")))) {
        return upper.at(3).isDigit();
    }
    return false;
}

QString stemFromTitle(const QString &title)
{
    QString stem;
    const QString trimmed = title.trimmed();
    for (int i = 0; i < trimmed.size() && stem.size() < kMaxStemChars; ++i) {
        const QChar c = trimmed.at(i);
        if (c.unicode() < 32) {
            continue;
        }
        const bool keep = c.isLetterOrNumber() || c.unicode() > 127
                || c == QLatin1Char('-') || c == QLatin1Char('_');
        if (c.isSpace()) {
            if (!stem.isEmpty() && !stem.endsWith(QLatin1Char('-'))) {
                stem += QLatin1Char('-');
            }
            continue;
        }
        if (keep) {
            stem += c;
            continue;
        }
        if (!stem.isEmpty() && !stem.endsWith(QLatin1Char('-'))) {
            stem += QLatin1Char('-');
        }
    }
    while (stem.endsWith(QLatin1Char('-')) || stem.endsWith(QLatin1Char('.'))
           || stem.endsWith(QLatin1Char(' '))) {
        stem.chop(1);
    }
    if (stem.isEmpty() || stem == QLatin1String(".") || stem == QLatin1String("..")
            || reservedStem(stem)) {
        stem = QStringLiteral("note");
    }
    return stem;
}

} // namespace

const int ZoneStore::maxNoteBytes = 256 * 1024;

ZoneStore::ZoneStore(const QString &root)
    : m_root(root.isEmpty()
             ? Helper::appHomePath() + QStringLiteral("/zone")
             : QDir::cleanPath(root))
{
}

QString ZoneStore::rootPath() const
{
    return m_root;
}

bool ZoneStore::ensureRoot(QString *error)
{
    if (error) {
        error->clear();
    }
    QDir dir(m_root);
    if (dir.exists()) {
        return true;
    }
    if (QDir().mkpath(m_root)) {
        return true;
    }
    if (error) {
        *error = trStore("Could not create the local note folder.");
    }
    return false;
}

bool ZoneStore::isNoteName(const QString &name)
{
    if (name.isEmpty() || name.size() > kMaxNameChars) {
        return false;
    }
    if (name.startsWith(QLatin1Char('.'))) {
        return false;
    }
    if (name.contains(QLatin1Char('/')) || name.contains(QLatin1Char('\\'))
            || name.contains(QLatin1Char(':')) || name == QLatin1String(".")
            || name == QLatin1String("..")) {
        return false;
    }
    for (int i = 0; i < name.size(); ++i) {
        if (name.at(i).unicode() < 32) {
            return false;
        }
    }
    const QString lower = name.toLower();
    return lower.endsWith(QLatin1String(".md"))
            || lower.endsWith(QLatin1String(".txt"))
            || lower.endsWith(QLatin1String(".markdown"));
}

QString ZoneStore::notePath(const QString &name, QString *error) const
{
    if (!isNoteName(name)) {
        if (error) {
            *error = trStore("That note name is not allowed.");
        }
        return QString();
    }
    const QString path = QDir(m_root).filePath(name);
    if (!insideRoot(m_root, path)) {
        if (error) {
            *error = trStore("That note is outside the local folder.");
        }
        return QString();
    }
    return QDir::cleanPath(path);
}

QStringList ZoneStore::noteNames() const
{
    QStringList names;
    QDir dir(m_root);
    if (!dir.exists()) {
        return names;
    }
    const QFileInfoList list = dir.entryInfoList(
                QDir::Files | QDir::NoDotAndDotDot | QDir::Readable,
                QDir::Name);
    for (int i = 0; i < list.size(); ++i) {
        const QFileInfo info = list.at(i);
        if (info.isSymLink() || !info.isFile()) {
            continue;
        }
        if (!isNoteName(info.fileName())) {
            continue;
        }
        names.append(info.fileName());
        if (names.size() >= 500) {
            break;
        }
    }
    return names;
}

QString ZoneStore::createNote(const QString &title, QString *error)
{
    if (error) {
        error->clear();
    }
    const QString trimmed = title.trimmed();
    if (trimmed.isEmpty()) {
        if (error) {
            *error = trStore("Enter a title.");
        }
        return QString();
    }
    if (!ensureRoot(error)) {
        return QString();
    }
    const QString stem = stemFromTitle(trimmed);
    QString name;
    for (int n = 1; n < 200; ++n) {
        const QString candidate = (n == 1)
                ? stem + QStringLiteral(".md")
                : stem + QLatin1Char('-') + QString::number(n) + QStringLiteral(".md");
        const QString path = notePath(candidate, error);
        if (path.isEmpty()) {
            return QString();
        }
        if (!QFileInfo::exists(path)) {
            name = candidate;
            break;
        }
    }
    if (name.isEmpty()) {
        if (error) {
            *error = trStore("Could not choose a file name.");
        }
        return QString();
    }
    QString heading = trimmed;
    const int nl = heading.indexOf(QLatin1Char('\n'));
    if (nl >= 0) {
        heading = heading.left(nl).trimmed();
    }
    const QString body = QStringLiteral("# ") + heading + QStringLiteral("\n\n");
    if (!writeNote(name, body, error)) {
        return QString();
    }
    return name;
}

QString ZoneStore::readNote(const QString &name, QString *error) const
{
    if (error) {
        error->clear();
    }
    const QString path = notePath(name, error);
    if (path.isEmpty()) {
        return QString();
    }
    const QFileInfo info(path);
    if (info.isSymLink()) {
        if (error) {
            *error = trStore("That note is not a regular file.");
        }
        return QString();
    }
    if (!info.exists() || !info.isFile()) {
        if (error) {
            *error = trStore("That note is missing.");
        }
        return QString();
    }
    if (info.size() > maxNoteBytes) {
        if (error) {
            *error = trStore("That note is too large to open here.");
        }
        return QString();
    }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error) {
            *error = trStore("Could not read the note.");
        }
        return QString();
    }
    return QString::fromUtf8(file.readAll());
}

bool ZoneStore::writeNote(const QString &name, const QString &body, QString *error)
{
    if (error) {
        error->clear();
    }
    const QByteArray bytes = body.toUtf8();
    if (bytes.size() > maxNoteBytes) {
        if (error) {
            *error = trStore("That note is too large.");
        }
        return false;
    }
    const QString path = notePath(name, error);
    if (path.isEmpty()) {
        return false;
    }
    if (!ensureRoot(error)) {
        return false;
    }
    const QFileInfo info(path);
    if (info.isSymLink()) {
        if (error) {
            *error = trStore("That note is not a regular file.");
        }
        return false;
    }
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (error) {
            *error = trStore("Could not write the note.");
        }
        return false;
    }
    if (file.write(bytes) != bytes.size() || !file.commit()) {
        if (error) {
            *error = trStore("Could not write the note.");
        }
        return false;
    }
    return true;
}

bool ZoneStore::removeNote(const QString &name, QString *error)
{
    if (error) {
        error->clear();
    }
    const QString path = notePath(name, error);
    if (path.isEmpty()) {
        return false;
    }
    const QFileInfo info(path);
    if (info.isSymLink()) {
        if (error) {
            *error = trStore("That note is not a regular file.");
        }
        return false;
    }
    if (!info.exists()) {
        return true;
    }
    if (!info.isFile()) {
        if (error) {
            *error = trStore("That note is not a regular file.");
        }
        return false;
    }
    if (!QFile::remove(path)) {
        if (error) {
            *error = trStore("Could not delete the note.");
        }
        return false;
    }
    return true;
}
