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
#include "avatarWire.h"
#include "helper.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QMessageBox>
#include <QMutexLocker>
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

QString AvatarStore::wirePathFor(const QString &key) const
{
    const QString manual = fileForKey(key);
    if (manual.isEmpty() || key.trimmed() == selfKey()) {
        return QString();
    }
    const QString base = QFileInfo(manual).completeBaseName();
    return Helper::appHomePath() + QLatin1String("/avatars/wire/") + base
            + QLatin1String(".png");
}

QString AvatarStore::wireIdPath(const QString &key) const
{
    const QString png = wirePathFor(key);
    if (png.isEmpty()) {
        return QString();
    }
    return png.left(png.size() - 4) + QLatin1String(".id");
}

QString AvatarStore::skipPath(const QString &key) const
{
    const QString png = wirePathFor(key);
    if (png.isEmpty()) {
        return QString();
    }
    return png.left(png.size() - 4) + QLatin1String(".skip");
}

QString AvatarStore::selfWirePath() const
{
    return Helper::appHomePath() + QLatin1String("/avatars/self-wire.png");
}

QPixmap AvatarStore::rounded(const QString &key, int size, const QColor &status) const
{
    QMutexLocker locker(&m_lock);
    QString path = fileForKey(key);
    const bool manual = !path.isEmpty() && QFileInfo::exists(path);
    if (!manual) {
        if (key.trimmed() != selfKey() && m_wireHidden.value(key, false)) {
            return QPixmap();
        }
        path = (key.trimmed() == selfKey()) ? QString() : wirePathFor(key);
    }
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
    locker.unlock();
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
    {
        QMutexLocker locker(&m_lock);
        m_cache.remove(dest);
    }
    emit changed();
    if (key == selfKey()) {
        emit selfPhotoChanged();
    } else {
        setStatus(key, tr("Using a photo set on this computer. "
                          "It overrides the peer photo."));
    }
    return true;
}

void AvatarStore::clearKey(const QString &key)
{
    const QString dest = fileForKey(key);
    if (dest.isEmpty()) {
        return;
    }
    QFile::remove(dest);
    if (key == selfKey()) {
        QFile::remove(selfWirePath());
    }
    {
        QMutexLocker locker(&m_lock);
        m_cache.remove(dest);
        m_cache.remove(selfWirePath());
    }
    emit changed();
    if (key == selfKey()) {
        emit selfPhotoChanged();
    }
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
    QMutexLocker locker(&m_lock);
    m_cache.clear();
    locker.unlock();
    emit changed();
}

bool AvatarStore::ensureSelfWire()
{
    const QString selfPath = fileForKey(selfKey());
    const QString wirePath = selfWirePath();
    if (selfPath.isEmpty() || !QFileInfo::exists(selfPath)) {
        QFile::remove(wirePath);
        return false;
    }
    const QFileInfo selfInfo(selfPath);
    const QFileInfo wireInfo(wirePath);
    if (wireInfo.exists() && wireInfo.lastModified() >= selfInfo.lastModified()
            && wireInfo.size() > 0 && wireInfo.size() <= AvatarWire::maxPngBytes) {
        return true;
    }

    QImageReader reader(selfPath);
    reader.setAutoTransform(true);
    const QImage image = reader.read();
    QString error;
    const QByteArray png = AvatarWire::makeWirePng(image, &error);
    if (png.isEmpty()) {
        QFile::remove(wirePath);
        return false;
    }
    QDir().mkpath(QFileInfo(wirePath).absolutePath());
    QSaveFile out(wirePath);
    if (!out.open(QIODevice::WriteOnly) || out.write(png) != png.size() || !out.commit()) {
        QFile::remove(wirePath);
        return false;
    }
    return true;
}

QString AvatarStore::selfWireId()
{
    QMutexLocker locker(&m_lock);
    if (!ensureSelfWire()) {
        return QString();
    }
    QFile file(selfWirePath());
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    const QByteArray bytes = file.read(AvatarWire::maxPngBytes + 1);
    if (!AvatarWire::pngLooksOk(bytes)) {
        return QString();
    }
    return AvatarWire::shortIdOf(bytes);
}

QByteArray AvatarStore::selfWireBytes()
{
    QMutexLocker locker(&m_lock);
    if (!ensureSelfWire()) {
        return QByteArray();
    }
    QFile file(selfWirePath());
    if (!file.open(QIODevice::ReadOnly)) {
        return QByteArray();
    }
    const QByteArray bytes = file.read(AvatarWire::maxPngBytes + 1);
    if (!AvatarWire::pngLooksOk(bytes)) {
        return QByteArray();
    }
    return bytes;
}

bool AvatarStore::hasManual(const QString &key) const
{
    if (key.trimmed().isEmpty() || key.trimmed() == selfKey()) {
        return false;
    }
    const QString path = fileForKey(key);
    return !path.isEmpty() && QFileInfo::exists(path);
}

bool AvatarStore::hasWire(const QString &key) const
{
    const QString path = wirePathFor(key);
    return !path.isEmpty() && QFileInfo::exists(path);
}

QString AvatarStore::wireId(const QString &key) const
{
    const QString idPath = wireIdPath(key);
    if (!idPath.isEmpty()) {
        QFile file(idPath);
        if (file.open(QIODevice::ReadOnly)) {
            const QString id = QString::fromLatin1(file.readAll()).trimmed().toLower();
            if (AvatarWire::idOk(id)) {
                return id;
            }
        }
    }
    const QString pngPath = wirePathFor(key);
    QFile png(pngPath);
    if (!png.open(QIODevice::ReadOnly)) {
        return QString();
    }
    const QByteArray bytes = png.read(AvatarWire::maxPngBytes + 1);
    if (!AvatarWire::pngLooksOk(bytes)) {
        return QString();
    }
    return AvatarWire::shortIdOf(bytes);
}

QString AvatarStore::skipId(const QString &key) const
{
    const QString path = skipPath(key);
    if (path.isEmpty()) {
        return QString();
    }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    return QString::fromLatin1(file.readAll()).trimmed();
}

QString AvatarStore::status(const QString &key) const
{
    QMutexLocker locker(&m_lock);
    return m_status.value(key);
}

void AvatarStore::setStatus(const QString &key, const QString &text)
{
    {
        QMutexLocker locker(&m_lock);
        if (m_status.value(key) == text) {
            return;
        }
        m_status.insert(key, text);
    }
    emit changed();
}

void AvatarStore::setWireHidden(const QString &key, bool hidden)
{
    {
        QMutexLocker locker(&m_lock);
        if (m_wireHidden.value(key, false) == hidden) {
            return;
        }
        m_wireHidden.insert(key, hidden);
        m_cache.remove(wirePathFor(key));
    }
    emit changed();
}

void AvatarStore::writeSkip(const QString &key, const QString &id)
{
    const QString path = skipPath(key);
    if (path.isEmpty()) {
        return;
    }
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return;
    }
    file.write(id.toLatin1());
}

void AvatarStore::clearManual(const QString &key)
{
    const QString dest = fileForKey(key);
    if (dest.isEmpty()) {
        return;
    }
    QFile::remove(dest);
    {
        QMutexLocker locker(&m_lock);
        m_cache.remove(dest);
    }
    emit changed();
}

void AvatarStore::clearWire(const QString &key)
{
    const QString dest = wirePathFor(key);
    if (!dest.isEmpty()) {
        QFile::remove(dest);
        QFile::remove(wireIdPath(key));
    }
    {
        QMutexLocker locker(&m_lock);
        m_cache.remove(dest);
    }
    emit changed();
}

bool AvatarStore::saveWire(const QString &key, const QByteArray &png,
                           const QString &id, QString *error)
{
    if (!AvatarWire::pngLooksOk(png) || AvatarWire::shortIdOf(png) != id) {
        if (error) {
            *error = tr("The photo data was not a readable PNG.");
        }
        return false;
    }
    const QString path = wirePathFor(key);
    const QString idFile = wireIdPath(key);
    if (path.isEmpty() || idFile.isEmpty()) {
        if (error) {
            *error = tr("Could not store that photo.");
        }
        return false;
    }
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile out(path);
    if (!out.open(QIODevice::WriteOnly) || out.write(png) != png.size() || !out.commit()) {
        if (error) {
            *error = tr("Could not store that photo.");
        }
        return false;
    }
    QFile idOut(idFile);
    if (!idOut.open(QIODevice::WriteOnly | QIODevice::Truncate)
            || idOut.write(id.toLatin1()) != id.size()) {
        if (error) {
            *error = tr("Could not store that photo.");
        }
        return false;
    }
    QFile::remove(skipPath(key));
    {
        QMutexLocker locker(&m_lock);
        m_cache.remove(path);
        m_wireHidden.insert(key, false);
    }
    if (error) {
        error->clear();
    }
    emit changed();
    return true;
}
