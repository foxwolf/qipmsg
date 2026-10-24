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

#include "message_store.h"
#include "helper.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTextStream>

namespace {

QStringList imagePathsFromJson(const QJsonValue &value)
{
    QStringList paths;
    if (value.isArray()) {
        const QJsonArray array = value.toArray();
        for (int i = 0; i < array.size(); ++i) {
            const QString path = array.at(i).toString();
            if (!path.isEmpty()) {
                paths << path;
            }
        }
    } else if (value.isString()) {
        const QString path = value.toString();
        if (!path.isEmpty()) {
            paths << path;
        }
    }
    return paths;
}

} // namespace

MessageStore::MessageStore()
{
    ensureHistoryDir();
}

QString MessageStore::historyDir() const
{
    return Helper::appHomePath() + "/history";
}

bool MessageStore::ensureHistoryDir() const
{
    QDir dir(historyDir());
    if (dir.exists()) {
        return true;
    }
    return dir.mkpath(".");
}

QString MessageStore::sanitizeIp(const QString &ip) const
{
    QString name = ip.trimmed();
    if (name.isEmpty()) {
        name = "unknown";
    }
    name.replace(':', '_');
    name.replace('/', '_');
    name.replace('\\', '_');
    name.replace(' ', '_');
    return name;
}

QString MessageStore::filePathForIp(const QString &ip) const
{
    return historyDir() + "/" + sanitizeIp(ip) + ".jsonl";
}

bool MessageStore::append(const ChatRecord &record)
{
    QMutexLocker locker(&m_lock);

    if (!ensureHistoryDir()) {
        return false;
    }

    QFile file(filePathForIp(record.peerIp));
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        return false;
    }

    QJsonObject obj;
    obj.insert("t", record.timestamp.toString(Qt::ISODate));
    obj.insert("d", record.direction);
    obj.insert("n", record.peerName);
    obj.insert("ip", record.peerIp);
    obj.insert("x", record.text);
    if (!record.imagePaths.isEmpty()) {
        QJsonArray images;
        foreach (const QString &path, record.imagePaths) {
            images.append(path);
        }
        obj.insert("img", images);
    }
    if (!record.packetNo.isEmpty()) {
        obj.insert("pkt", record.packetNo);
    }
    if (record.expectRead) {
        obj.insert("er", true);
    }
    if (record.readReceipt) {
        obj.insert("rd", true);
    }

    QTextStream ts(&file);
    ts << QJsonDocument(obj).toJson(QJsonDocument::Compact) << '\n';
    ts.flush();
    return true;
}

bool MessageStore::markRead(const QString &peerIp, const QString &packetNo)
{
    if (packetNo.isEmpty()) {
        return false;
    }

    QMutexLocker locker(&m_lock);
    QFile file(filePathForIp(peerIp));
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QStringList lines;
    bool changed = false;
    QTextStream input(&file);
    while (!input.atEnd()) {
        const QString line = input.readLine();
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty()) {
            lines << line;
            continue;
        }
        QJsonParseError error;
        const QJsonDocument doc = QJsonDocument::fromJson(trimmed.toUtf8(), &error);
        if (error.error != QJsonParseError::NoError || !doc.isObject()) {
            lines << line;
            continue;
        }
        QJsonObject obj = doc.object();
        const bool outgoing = obj.value("d").toString() == QLatin1String("out");
        if (outgoing && obj.value("pkt").toString() == packetNo
                && !obj.value("rd").toBool()) {
            obj.insert("rd", true);
            lines << QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
            changed = true;
        } else {
            lines << line;
        }
    }
    file.close();
    if (!changed) {
        return false;
    }

    QSaveFile out(file.fileName());
    if (!out.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    {
        QTextStream output(&out);
        for (int i = 0; i < lines.size(); ++i) {
            output << lines.at(i) << '\n';
        }
        output.flush();
    }
    return out.commit();
}

QList<ChatRecord> MessageStore::loadHistory(const QString &peerIp,
                                            int limit) const
{
    QMutexLocker locker(&m_lock);

    QList<ChatRecord> records;
    QFile file(filePathForIp(peerIp));
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return records;
    }

    QTextStream ts(&file);
    while (!ts.atEnd()) {
        const QString line = ts.readLine().trimmed();
        if (line.isEmpty()) {
            continue;
        }

        QJsonParseError error;
        const QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &error);
        if (error.error != QJsonParseError::NoError || !doc.isObject()) {
            continue;
        }

        const QJsonObject obj = doc.object();
        ChatRecord record;
        record.timestamp = QDateTime::fromString(obj.value("t").toString(),
                                                 Qt::ISODate);
        if (!record.timestamp.isValid()) {
            record.timestamp = QDateTime::currentDateTime();
        }
        record.peerIp = obj.value("ip").toString();
        if (record.peerIp.isEmpty()) {
            record.peerIp = peerIp;
        }
        record.peerName = obj.value("n").toString();
        record.direction = obj.value("d").toString();
        record.text = obj.value("x").toString();
        record.imagePaths = imagePathsFromJson(obj.value("img"));
        record.packetNo = obj.value("pkt").toString();
        record.expectRead = obj.value("er").toBool();
        record.readReceipt = obj.value("rd").toBool();
        records.append(record);
    }

    if (limit > 0 && records.size() > limit) {
        records = records.mid(records.size() - limit);
    }

    return records;
}

QList<ChatRecord> MessageStore::search(const QString &needle, int limit) const
{
    QList<ChatRecord> matches;
    if (needle.trimmed().isEmpty()) {
        return matches;
    }

    QMutexLocker locker(&m_lock);
    const QDir dir(historyDir());
    const QFileInfoList files = dir.entryInfoList(QStringList() << "*.jsonl",
                                                  QDir::Files, QDir::Name);
    foreach (const QFileInfo &info, files) {
        QFile file(info.absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }
        const QString stem = info.completeBaseName();
        QTextStream ts(&file);
        while (!ts.atEnd()) {
            const QString line = ts.readLine().trimmed();
            if (line.isEmpty()) {
                continue;
            }
            QJsonParseError error;
            const QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(),
                                                              &error);
            if (error.error != QJsonParseError::NoError || !doc.isObject()) {
                continue;
            }
            const QJsonObject obj = doc.object();
            const QString text = obj.value("x").toString();
            const QString name = obj.value("n").toString();
            QString ip = obj.value("ip").toString();
            if (ip.isEmpty()) {
                ip = stem;
            }
            if (!text.contains(needle, Qt::CaseInsensitive)
                    && !name.contains(needle, Qt::CaseInsensitive)
                    && !ip.contains(needle, Qt::CaseInsensitive)) {
                continue;
            }
            ChatRecord record;
            record.timestamp = QDateTime::fromString(obj.value("t").toString(),
                                                     Qt::ISODate);
            if (!record.timestamp.isValid()) {
                record.timestamp = QDateTime::currentDateTime();
            }
            record.peerIp = ip;
            record.peerName = name.isEmpty() ? ip : name;
            record.direction = obj.value("d").toString();
            record.text = text;
            record.imagePaths = imagePathsFromJson(obj.value("img"));
            matches.append(record);
            if (limit > 0 && matches.size() >= limit) {
                return matches;
            }
        }
    }
    return matches;
}
