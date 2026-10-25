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

#include "avatarStore.h"
#include "helper.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QMessageBox>
#include <QSaveFile>

namespace {

const qint64 kMaxBytes = 20ll * 1024ll * 1024ll;


} // namespace

AvatarStore::AvatarStore(QObject *parent)
    : QObject(parent)
{
}

QString AvatarStore::selfKey()
{
    return QStringLiteral("self");
}

QString AvatarStore::fileForKey(const QString &key) const
{
    const QString trimmed = key.trimmed();
    if (trimmed.isEmpty()) {
        return QString();
    }

    QString token;
    if (trimmed == selfKey()) {
        token = QStringLiteral("self");
    } else {
        token = QStringLiteral("peer-");
        for (int i = 0; i < trimmed.size(); ++i) {
            const QChar c = trimmed.at(i);
            if (c.isLetterOrNumber()) {
                token.append(c);
            } else {
                token.append(QLatin1Char('-'));
            }
        }
    }
    if (token.isEmpty() || token == QLatin1String("peer-")) {
        return QString();
    }
    return Helper::appHomePath() + QLatin1String("/avatars/") + token
            + QLatin1String(".png");
}

QPixmap AvatarStore::rounded(const QString &key, int size, const QColor &status) const
{
    const QString path = fileForKey(key);
    if (path.isEmpty() || !QFileInfo::exists(path)) {
        return QPixmap();
    }

    QPixmap source = m_cache.value(path);
    if (source.isNull()) {
        QImageReader reader(path);
        reader.setAutoTransform(true);
        const QImage image = reader.read();
        if (image.isNull()) {
            return QPixmap();
        }
        source = QPixmap::fromImage(image);
        m_cache.insert(path, source);
    }
    return Helper::roundAvatar(source, size, status);
}

bool AvatarStore::setFromFile(const QString &key, const QString &source, QString *error)
{
    const QString dest = fileForKey(key);
    if (dest.isEmpty()) {
        if (error) {
            *error = tr("Could not store that photo.");
        }
        return false;
    }
    const QFileInfo info(source);
    if (!info.isFile() || info.size() <= 0 || info.size() > kMaxBytes) {
        if (error) {
            *error = tr("Choose a PNG or JPEG smaller than 20 MB.");
        }
        return false;
    }

    QImageReader reader(source);
    reader.setAutoTransform(true);
    const QByteArray format = reader.format().toLower();
    if (format != "png" && format != "jpeg" && format != "jpg") {
        if (error) {
            *error = tr("Choose a PNG or JPEG smaller than 20 MB.");
        }
        return false;
    }
    QSize scaled = reader.size();
    if (scaled.isValid() && (scaled.width() > 512 || scaled.height() > 512)) {
        scaled.scale(512, 512, Qt::KeepAspectRatio);
        reader.setScaledSize(scaled);
    }
    QImage image = reader.read();
    if (image.isNull()) {
        if (error) {
            *error = tr("Could not read that image.");
        }
        return false;
    }
    if (image.width() > 512 || image.height() > 512) {
        image = image.scaled(512, 512, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    QDir().mkpath(Helper::appHomePath() + QLatin1String("/avatars"));
    QSaveFile out(dest);
    if (!out.open(QIODevice::WriteOnly) || !image.save(&out, "PNG") || !out.commit()) {
        if (error) {
            *error = tr("Could not store that photo.");
        }
        return false;
    }
    m_cache.remove(dest);
    emit changed();
    return true;
}

void AvatarStore::clearKey(const QString &key)
{
    const QString dest = fileForKey(key);
    if (dest.isEmpty()) {
        return;
    }
    QFile::remove(dest);
    m_cache.remove(dest);
    emit changed();
}

bool AvatarStore::chooseAndSet(const QString &key, QWidget *parent)
{
    const QString path = QFileDialog::getOpenFileName(
                parent,
                tr("Choose photo"),
                QString(),
                tr("Images (*.png *.jpg *.jpeg)"));
    if (path.isEmpty()) {
        return false;
    }
    QString error;
    if (!setFromFile(key, path, &error)) {
        QMessageBox::warning(parent, tr("Choose photo"),
                             error.isEmpty()
                             ? tr("Could not store that photo.")
                             : error);
        return false;
    }
    return true;
}

void AvatarStore::noteFilesChanged()
{
    m_cache.clear();
    emit changed();
}
