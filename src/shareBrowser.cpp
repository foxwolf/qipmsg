// This file is part of QIpMsg.
//
// QIpMsg is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "shareBrowser.h"

#include "fileHashIndex.h"
#include "global.h"
#include "helper.h"
#include "ipmsg.h"
#include "preferences.h"
#include "user_manager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QHostAddress>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QTableWidget>
#include <QTcpSocket>
#include <QVBoxLayout>

namespace {

const int ConnectTimeoutMs = 8000;
const int IoTimeoutMs = 8000;
const int MaxListBytes = 512 * 1024;

bool readUntil(QTcpSocket *socket, QByteArray *buf, int maxBytes, QString *error,
               bool (*done)(const QByteArray &))
{
    while (!done(*buf)) {
        if (buf->size() > maxBytes) {
            if (error) {
                *error = QObject::tr("The share listing is too large.");
            }
            return false;
        }
        if (!socket->waitForReadyRead(IoTimeoutMs)) {
            if (socket->state() == QAbstractSocket::UnconnectedState && done(*buf)) {
                break;
            }
            if (error) {
                *error = socket->errorString();
            }
            return false;
        }
        buf->append(socket->readAll());
    }
    return true;
}

bool listDone(const QByteArray &buf)
{
    return buf.contains("\nEND\n") || buf.contains("\nTRUNC\n")
            || buf.endsWith("\nEND\n") || buf.endsWith("\nTRUNC\n")
            || buf.contains("\nERR");
}

QString statusText(const QString &code)
{
    if (code == QLatin1String("auth")) {
        return QObject::tr("Wrong share password.");
    }
    if (code == QLatin1String("off")) {
        return QObject::tr("That peer is not sharing a folder.");
    }
    if (code == QLatin1String("missing")) {
        return QObject::tr("That shared path is gone.");
    }
    if (code == QLatin1String("badpath")) {
        return QObject::tr("That path is outside the shared folder.");
    }
    if (code.isEmpty()) {
        return QObject::tr("The peer closed the share connection.");
    }
    return code;
}

} // namespace

ShareBrowser::ShareBrowser(const QString &ip, QWidget *parent)
    : QDialog(parent), m_ip(ip), m_busy(false), m_status(0), m_pathLabel(0),
      m_passwordEdit(0), m_upButton(0), m_refreshButton(0),
      m_downloadButton(0), m_table(0)
{
    init();
}

ShareBrowser::ShareBrowser(const QString &ip, const QString &groupId, QWidget *parent)
    : QDialog(parent), m_ip(ip), m_groupId(groupId), m_busy(false), m_status(0),
      m_pathLabel(0), m_passwordEdit(0), m_upButton(0), m_refreshButton(0),
      m_downloadButton(0), m_table(0)
{
    init();
}

void ShareBrowser::init()
{
    const bool groupMode = !m_groupId.isEmpty();
    setWindowTitle(groupMode ? tr("Discussion folder") : tr("Browse peer share"));
    resize(560, 420);

    m_status = new QLabel;
    m_status->setWordWrap(true);
    m_pathLabel = new QLabel(QStringLiteral("/"));
    m_passwordEdit = new QLineEdit;
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText(tr("Share password, if the peer set one"));

    m_upButton = new QPushButton(tr("Up"));
    m_refreshButton = new QPushButton(tr("Refresh"));
    m_downloadButton = new QPushButton(tr("Download"));
    connect(m_upButton, SIGNAL(clicked()), this, SLOT(goUp()));
    connect(m_refreshButton, SIGNAL(clicked()), this, SLOT(refreshList()));
    connect(m_downloadButton, SIGNAL(clicked()), this, SLOT(downloadSelected()));

    m_table = new QTableWidget(0, 3);
    m_table->setHorizontalHeaderLabels(
                QStringList() << tr("Name") << tr("Kind") << tr("Size"));
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(m_table, SIGNAL(cellDoubleClicked(int,int)),
            this, SLOT(enterRow(int,int)));

    QHBoxLayout *pathRow = new QHBoxLayout;
    pathRow->addWidget(m_upButton);
    pathRow->addWidget(m_pathLabel, 1);
    pathRow->addWidget(m_refreshButton);

    QHBoxLayout *passRow = new QHBoxLayout;
    passRow->addWidget(new QLabel(tr("Password")));
    passRow->addWidget(m_passwordEdit, 1);
    passRow->addWidget(m_downloadButton);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_status);
    layout->addLayout(pathRow);
    layout->addWidget(m_table, 1);
    layout->addLayout(passRow);

    if (!m_groupId.isEmpty()) {
        m_passwordEdit->setEnabled(false);
        m_passwordEdit->setPlaceholderText(tr("Group folder uses membership, not a password"));
    }
    if (!peerReady()) {
        m_table->setEnabled(false);
        m_upButton->setEnabled(false);
        m_refreshButton->setEnabled(false);
        m_downloadButton->setEnabled(false);
        m_passwordEdit->setEnabled(false);
        if (!m_groupId.isEmpty()) {
            m_status->setText(tr(
                "This peer is not a reachable qipmsg. FeiQ cannot serve a "
                "discussion folder. The owner must be online with this version."));
        } else {
            m_status->setText(tr(
                "This peer is not advertising a qipmsg shared folder. "
                "FeiQ's share protocol is not in the IPMSG spec and has no "
                "verified packet format here, so a FeiQ share cannot be listed. "
                "Turn on sharing in qipmsg Preferences, Detail, File."));
        }
        return;
    }
    if (!m_groupId.isEmpty()) {
        m_status->setText(tr(
            "Listing this discussion folder. Files stay under that folder only. "
            "FeiQ peers still have no share browser."));
    } else if (Global::userManager
            && ShareFolder::tokenHasLock(Global::userManager->peerQipToken(m_ip))) {
        m_status->setText(tr("This share asks for a password."));
    } else {
        m_status->setText(tr(
            "Listing a qipmsg shared folder. FeiQ peers still have no share browser."));
    }
    refreshList();
}

bool ShareBrowser::peerReady() const
{
    if (!Global::userManager) {
        return false;
    }
    if (!Global::userManager->isReachable(m_ip)) {
        return false;
    }
    const QString token = Global::userManager->peerQipToken(m_ip);
    if (!m_groupId.isEmpty()) {
        return ShareFolder::tokenIsQipmsg(token) && ShareFolder::groupIdOk(m_groupId);
    }
    return ShareFolder::tokenHasShare(token);
}

QByteArray ShareBrowser::makeRequest(const QString &verb, const QString &relative) const
{
    if (!m_groupId.isEmpty()) {
        return ShareFolder::buildGroupRequest(verb, relative, m_groupId, 0);
    }
    return ShareFolder::buildRequest(verb, relative, m_password, 0);
}

QString ShareBrowser::currentRelative() const
{
    return m_relative;
}

void ShareBrowser::refreshList()
{
    if (m_busy || !peerReady()) {
        return;
    }
    m_password = m_passwordEdit->text();
    if (!fetchList()) {
        return;
    }
}

void ShareBrowser::goUp()
{
    if (m_relative.isEmpty()) {
        return;
    }
    const int slash = m_relative.lastIndexOf(QLatin1Char('/'));
    m_relative = slash < 0 ? QString() : m_relative.left(slash);
    refreshList();
}

void ShareBrowser::enterRow(int row, int column)
{
    Q_UNUSED(column);
    if (row < 0 || row >= m_entries.size()) {
        return;
    }
    const ShareEntry entry = m_entries.at(row);
    if (!entry.directory) {
        downloadEntry(entry);
        return;
    }
    m_relative = m_relative.isEmpty()
            ? entry.name
            : m_relative + QLatin1Char('/') + entry.name;
    refreshList();
}

void ShareBrowser::downloadSelected()
{
    const int row = m_table->currentRow();
    if (row < 0 || row >= m_entries.size()) {
        return;
    }
    const ShareEntry entry = m_entries.at(row);
    if (entry.directory) {
        m_relative = m_relative.isEmpty()
                ? entry.name
                : m_relative + QLatin1Char('/') + entry.name;
        refreshList();
        return;
    }
    downloadEntry(entry);
}

bool ShareBrowser::fetchList()
{
    m_busy = true;
    QString error;
    const QByteArray request = makeRequest(QStringLiteral("LIST"), m_relative);
    const QByteArray payload = exchange(request, &error, QString(), 0);
    m_busy = false;
    if (!error.isEmpty()) {
        m_status->setText(statusText(error));
        return false;
    }
    QList<ShareEntry> entries;
    bool truncated = false;
    QString decodeError;
    if (!ShareFolder::decodeList(payload, &entries, &truncated, &decodeError)) {
        m_status->setText(statusText(decodeError));
        return false;
    }
    m_entries = entries;
    m_pathLabel->setText(m_relative.isEmpty()
                         ? QStringLiteral("/") : QStringLiteral("/") + m_relative);
    m_table->setRowCount(entries.size());
    for (int i = 0; i < entries.size(); ++i) {
        const ShareEntry &entry = entries.at(i);
        m_table->setItem(i, 0, new QTableWidgetItem(entry.name));
        m_table->setItem(i, 1, new QTableWidgetItem(
                             entry.directory ? tr("Folder") : tr("File")));
        m_table->setItem(i, 2, new QTableWidgetItem(
                             entry.directory
                             ? QString()
                             : Helper::sizeStringUnit(entry.size)));
    }
    m_status->setText(truncated
                      ? tr("Showing the first %1 entries.").arg(entries.size())
                      : tr("%n item(s)", "", entries.size()));
    return true;
}

bool ShareBrowser::downloadEntry(const ShareEntry &entry)
{
    if (entry.directory || m_busy) {
        return false;
    }
    m_password = m_passwordEdit->text();
    const QString rel = m_relative.isEmpty()
            ? entry.name
            : m_relative + QLatin1Char('/') + entry.name;
    QString dir = Global::preferences
            ? Global::preferences->autoRecvPath : Helper::defaultRecvPath();
    if (dir.isEmpty()) {
        dir = Helper::defaultRecvPath();
    }
    QDir().mkpath(dir);
    const QString saveName = Helper::uniqueFileName(dir, entry.name);
    const QString savePath = QDir(dir).filePath(saveName);

    m_busy = true;
    QString error;
    const QByteArray request = m_groupId.isEmpty()
            ? ShareFolder::buildRequest(QStringLiteral("GET"), rel, m_password, 0)
            : ShareFolder::buildGroupRequest(QStringLiteral("GET"), rel, m_groupId, 0);
    qint64 wrote = 0;
    exchange(request, &error, savePath, &wrote);
    m_busy = false;
    if (!error.isEmpty()) {
        QFile::remove(savePath);
        QMessageBox::warning(this, tr("Browse peer share"), statusText(error));
        return false;
    }
    qint64 size = -1;
    const QString sha = FileHashIndex::hashFile(savePath, &size);
    if (!sha.isEmpty()) {
        FileHashIndex::note(savePath, sha, size, QStringLiteral("share"), m_ip);
    }
    m_status->setText(tr("Saved %1").arg(savePath));
    return true;
}

QByteArray ShareBrowser::exchange(const QByteArray &request, QString *error,
                                  const QString &savePath, qint64 *wrote)
{
    if (wrote) {
        *wrote = 0;
    }
    quint16 port = IPMSG_DEFAULT_PORT;
    if (Global::userManager) {
        const quint16 peerPort = Global::userManager->peerPort(m_ip);
        if (peerPort != 0) {
            port = peerPort;
        }
    }
    QTcpSocket socket;
    socket.connectToHost(QHostAddress(m_ip), port);
    if (!socket.waitForConnected(ConnectTimeoutMs)) {
        if (error) {
            *error = socket.errorString();
        }
        return QByteArray();
    }
    socket.write(request);
    if (!socket.waitForBytesWritten(IoTimeoutMs)) {
        if (error) {
            *error = socket.errorString();
        }
        return QByteArray();
    }

    QByteArray buf;
    const bool download = !savePath.isEmpty();
    if (!download) {
        if (!readUntil(&socket, &buf, MaxListBytes, error, listDone)) {
            return QByteArray();
        }
        return buf;
    }

    while (buf.count('\n') < 2) {
        if (!socket.waitForReadyRead(IoTimeoutMs)) {
            if (error) {
                *error = socket.errorString();
            }
            return QByteArray();
        }
        buf.append(socket.readAll());
        if (buf.size() > MaxListBytes) {
            if (error) {
                *error = tr("Unexpected share reply.");
            }
            return QByteArray();
        }
    }
    const int firstNl = buf.indexOf('\n');
    const int secondNl = buf.indexOf('\n', firstNl + 1);
    const QByteArray line1 = buf.left(firstNl).trimmed();
    const QByteArray line2 = buf.mid(firstNl + 1, secondNl - firstNl - 1).trimmed();
    if (line1 != "QIP1") {
        if (error) {
            *error = tr("Unexpected share reply.");
        }
        return QByteArray();
    }
    if (line2.startsWith("ERR")) {
        if (error) {
            *error = QString::fromUtf8(line2.mid(3).trimmed());
        }
        return QByteArray();
    }
    if (!line2.startsWith("OK ")) {
        if (error) {
            *error = tr("Unexpected share reply.");
        }
        return QByteArray();
    }
    bool ok = false;
    const qint64 remain = line2.mid(3).trimmed().toLongLong(&ok);
    if (!ok || remain < 0) {
        if (error) {
            *error = tr("Unexpected share reply.");
        }
        return QByteArray();
    }
    QFile file(savePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (error) {
            *error = file.errorString();
        }
        return QByteArray();
    }
    QByteArray pending = buf.mid(secondNl + 1);
    qint64 got = 0;
    QProgressDialog progress(tr("Downloading…"), tr("Cancel"), 0,
                             remain > 0 ? 100 : 0, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(400);
    while (got < remain) {
        if (progress.wasCanceled()) {
            if (error) {
                *error = tr("Cancelled");
            }
            return QByteArray();
        }
        if (pending.isEmpty()) {
            if (!socket.waitForReadyRead(IoTimeoutMs)) {
                if (socket.state() == QAbstractSocket::UnconnectedState
                        && got >= remain) {
                    break;
                }
                if (error) {
                    *error = socket.errorString();
                }
                return QByteArray();
            }
            pending = socket.readAll();
            if (pending.isEmpty()) {
                continue;
            }
        }
        const qint64 need = remain - got;
        const QByteArray chunk = pending.left(static_cast<int>(qMin(need, (qint64)pending.size())));
        if (file.write(chunk) != chunk.size()) {
            if (error) {
                *error = file.errorString();
            }
            return QByteArray();
        }
        pending.remove(0, chunk.size());
        got += chunk.size();
        if (remain > 0) {
            progress.setValue(static_cast<int>((got * 100) / remain));
        }
    }
    file.close();
    if (wrote) {
        *wrote = got;
    }
    if (got != remain) {
        if (error) {
            *error = tr("The download ended early.");
        }
        return QByteArray();
    }
    return QByteArray("OK");
}
