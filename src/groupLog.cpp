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

#include "groupLog.h"
#include "helper.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QMutexLocker>

namespace {

const int kMaxText = 500;
const int kMaxAuthor = 80;
const qint64 kMaxFileBytes = 2ll * 1024ll * 1024ll;

QString &storageDirOverride()
{
    static QString dir;
    return dir;
}

QMutex &logLock()
{
    static QMutex mutex;
    return mutex;
}

bool idOk(const QString &groupId)
{
    if (groupId.isEmpty() || groupId.size() > 64) {
        return false;
    }
    if (!groupId.at(0).isLetterOrNumber()) {
        return false;
    }
    for (int i = 0; i < groupId.size(); ++i) {
        const QChar c = groupId.at(i);
        if (c.isLetterOrNumber() || c == QLatin1Char('-') || c == QLatin1Char('.')) {
            continue;
        }
        return false;
    }
    return true;
}

QString oneLine(const QString &text, int limit)
{
    QString out;
    out.reserve(qMin(text.size(), limit));
    for (int i = 0; i < text.size() && out.size() < limit; ++i) {
        const QChar c = text.at(i);
        if (c == QChar(0)) {
            break;
        }
        if (c == QLatin1Char('\n') || c == QLatin1Char('\r') || c == QChar(0x2028)) {
            if (!out.isEmpty() && !out.endsWith(QLatin1Char(' '))) {
                out.append(QLatin1Char(' '));
            }
            continue;
        }
        out.append(c);
    }
    return out.trimmed();
}

QString storageBase()
{
    return storageDirOverride().isEmpty()
            ? Helper::appHomePath() : storageDirOverride();
}

} // namespace

void GroupLog::setStorageDirForTest(const QString &dir)
{
    storageDirOverride() = dir;
}

QString GroupLog::filePath(const QString &groupId)
{
    if (!idOk(groupId)) {
        return QString();
    }
    return storageBase() + QLatin1String("/grouplog/") + groupId
            + QLatin1String(".jsonl");
}

bool GroupLog::append(const QString &groupId, const QString &author,
                      const QString &text, QString *error)
{
    const QString path = filePath(groupId);
    if (path.isEmpty()) {
        if (error) {
            *error = QStringLiteral("id");
        }
        return false;
    }
    const QString body = oneLine(text, kMaxText);
    if (body.isEmpty()) {
        if (error) {
            *error = QStringLiteral("empty");
        }
        return false;
    }
    QString who = oneLine(author, kMaxAuthor);
    if (who.isEmpty()) {
        who = QStringLiteral("me");
    }

    QMutexLocker locker(&logLock());
    if (QFileInfo::exists(path) && QFileInfo(path).size() >= kMaxFileBytes) {
        if (error) {
            *error = QStringLiteral("full");
        }
        return false;
    }

    QJsonObject obj;
    obj.insert(QStringLiteral("t"),
               QDateTime::currentDateTime().toString(Qt::ISODate));
    obj.insert(QStringLiteral("who"), who);
    obj.insert(QStringLiteral("text"), body);
    QByteArray line = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    line.append('\n');

    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        if (error) {
            *error = QStringLiteral("io");
        }
        return false;
    }
    if (file.write(line) != line.size()) {
        if (error) {
            *error = QStringLiteral("io");
        }
        return false;
    }
    if (error) {
        error->clear();
    }
    return true;
}

QList<GroupLogEntry> GroupLog::load(const QString &groupId, int limit)
{
    QList<GroupLogEntry> rows;
    const QString path = filePath(groupId);
    if (path.isEmpty() || limit <= 0) {
        return rows;
    }
    QMutexLocker locker(&logLock());
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return rows;
    }
    QByteArray raw = file.readAll();
    if (raw.size() > 256 * 1024) {
        raw = raw.right(256 * 1024);
        const int nl = raw.indexOf('\n');
        if (nl >= 0) {
            raw = raw.mid(nl + 1);
        }
    }
    const QList<QByteArray> lines = raw.split('\n');
    for (int i = 0; i < lines.size(); ++i) {
        const QByteArray line = lines.at(i).trimmed();
        if (line.isEmpty()) {
            continue;
        }
        const QJsonDocument doc = QJsonDocument::fromJson(line);
        if (!doc.isObject()) {
            continue;
        }
        const QJsonObject obj = doc.object();
        GroupLogEntry entry;
        entry.when = obj.value(QStringLiteral("t")).toString();
        entry.author = obj.value(QStringLiteral("who")).toString();
        entry.text = obj.value(QStringLiteral("text")).toString();
        if (entry.text.isEmpty()) {
            continue;
        }
        rows.append(entry);
    }
    if (rows.size() > limit) {
        rows = rows.mid(rows.size() - limit);
    }
    return rows;
}

QString GroupLog::exportPlain(const QString &groupId)
{
    const QList<GroupLogEntry> rows = load(groupId, 5000);
    QString out;
    for (int i = 0; i < rows.size(); ++i) {
        const GroupLogEntry &entry = rows.at(i);
        out += entry.when;
        out += QLatin1Char(' ');
        out += entry.author;
        out += QLatin1Char('\n');
        out += entry.text;
        out += QLatin1String("\n\n");
    }
    return out;
}
