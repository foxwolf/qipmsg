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

#include "offline_queue.h"
#include "global.h"
#include "helper.h"

#include <QAbstractItemView>
#include <QDialogButtonBox>
#include <QFile>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMutexLocker>
#include <QPushButton>
#include <QSaveFile>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace {

QString g_testFilePath;

}

OfflineQueue::OfflineQueue(QObject *parent)
    : QObject(parent), m_nextId(1)
{
    load();
}

void OfflineQueue::setFilePathForTest(const QString &path)
{
    g_testFilePath = path;
}

QString OfflineQueue::filePath() const
{
    if (!g_testFilePath.isEmpty()) {
        return g_testFilePath;
    }
    return Helper::appHomePath() + "/offline_queue.json";
}

QString OfflineQueue::previewText(const OfflineItem &item)
{
    if (item.kind == QLatin1String("shake")) {
        return tr("[Window shake]");
    }
    return item.text;
}

QString OfflineQueue::matchReachableIp(const QString &login, const QString &host,
                                       const QList<OfflinePeerHint> &peers)
{
    if (login.isEmpty() || host.isEmpty()) {
        return QString();
    }
    QString found;
    foreach (const OfflinePeerHint &peer, peers) {
        if (!peer.reachable || peer.login != login || peer.host != host) {
            continue;
        }
        if (!found.isEmpty()) {
            return QString();
        }
        found = peer.ip;
    }
    return found;
}

void OfflineQueue::load()
{
    QMutexLocker locker(&m_lock);
    m_items.clear();
    m_nextId = 1;

    QFile file(filePath());
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) {
        return;
    }
    const QJsonArray array = doc.array();
    for (int i = 0; i < array.size(); ++i) {
        const QJsonObject obj = array.at(i).toObject();
        OfflineItem item;
        item.id = obj.value("id").toString();
        item.ip = obj.value("ip").toString();
        item.text = obj.value("text").toString();
        item.kind = obj.value("kind").toString();
        if (item.kind.isEmpty()) {
            item.kind = QStringLiteral("text");
        }
        item.login = obj.value("login").toString();
        item.host = obj.value("host").toString();
        item.created = QDateTime::fromString(obj.value("created").toString(),
                                             Qt::ISODate);
        if (item.kind != QLatin1String("text") && item.kind != QLatin1String("shake")) {
            continue;
        }
        if (item.id.isEmpty() || item.ip.isEmpty() || item.text.isEmpty()) {
            continue;
        }
        if (!item.created.isValid()) {
            item.created = QDateTime::currentDateTime();
        }
        m_items.append(item);
        bool ok = false;
        const quint64 n = item.id.section('-', -1).toULongLong(&ok);
        if (ok && n >= m_nextId) {
            m_nextId = n + 1;
        }
    }
}

void OfflineQueue::persist() const
{
    QJsonArray array;
    foreach (const OfflineItem &item, m_items) {
        QJsonObject obj;
        obj.insert("id", item.id);
        obj.insert("ip", item.ip);
        obj.insert("text", item.text);
        obj.insert("kind", item.kind.isEmpty() ? QStringLiteral("text") : item.kind);
        obj.insert("login", item.login);
        obj.insert("host", item.host);
        obj.insert("created", item.created.toString(Qt::ISODate));
        array.append(obj);
    }
    QSaveFile file(filePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return;
    }
    file.write(QJsonDocument(array).toJson(QJsonDocument::Compact));
    file.commit();
}

void OfflineQueue::notifyChanged()
{
    emit queueChanged();
}

QString OfflineQueue::enqueue(const QString &ip, const QString &text,
                              const QString &kind, const QString &login,
                              const QString &host)
{
    const QString trimmed = text;
    QString storedKind = kind.isEmpty() ? QStringLiteral("text") : kind;
    if (storedKind != QLatin1String("text") && storedKind != QLatin1String("shake")) {
        return QString();
    }
    if (ip.isEmpty() || trimmed.isEmpty()) {
        return QString();
    }

    OfflineItem item;
    {
        QMutexLocker locker(&m_lock);
        item.id = QString("%1-%2")
                .arg(QDateTime::currentMSecsSinceEpoch())
                .arg(m_nextId++);
        item.ip = ip;
        item.text = trimmed;
        item.kind = storedKind;
        item.login = login;
        item.host = host;
        item.created = QDateTime::currentDateTime();
        m_items.append(item);
        persist();
    }
    notifyChanged();
    return item.id;
}

bool OfflineQueue::assignIp(const QString &id, const QString &newIp)
{
    if (id.isEmpty() || newIp.isEmpty()) {
        return false;
    }
    bool changed = false;
    {
        QMutexLocker locker(&m_lock);
        for (int i = 0; i < m_items.size(); ++i) {
            if (m_items.at(i).id != id || m_items.at(i).ip == newIp) {
                continue;
            }
            m_items[i].ip = newIp;
            persist();
            changed = true;
            break;
        }
    }
    if (changed) {
        notifyChanged();
    }
    return changed;
}

void OfflineQueue::restore(const QList<OfflineItem> &items)
{
    if (items.isEmpty()) {
        return;
    }
    {
        QMutexLocker locker(&m_lock);
        foreach (const OfflineItem &item, items) {
            if (item.id.isEmpty() || item.ip.isEmpty() || item.text.isEmpty()) {
                continue;
            }
            bool exists = false;
            foreach (const OfflineItem &have, m_items) {
                if (have.id == item.id) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                m_items.append(item);
            }
        }
        persist();
    }
    notifyChanged();
}

QList<OfflineItem> OfflineQueue::items() const
{
    QMutexLocker locker(&m_lock);
    return m_items;
}

QList<OfflineItem> OfflineQueue::itemsForIp(const QString &ip) const
{
    QList<OfflineItem> out;
    QMutexLocker locker(&m_lock);
    foreach (const OfflineItem &item, m_items) {
        if (item.ip == ip) {
            out.append(item);
        }
    }
    return out;
}

QList<OfflineItem> OfflineQueue::takeForIp(const QString &ip)
{
    QList<OfflineItem> out;
    {
        QMutexLocker locker(&m_lock);
        QList<OfflineItem> keep;
        foreach (const OfflineItem &item, m_items) {
            if (item.ip == ip) {
                out.append(item);
            } else {
                keep.append(item);
            }
        }
        if (out.isEmpty()) {
            return out;
        }
        m_items = keep;
        persist();
    }
    notifyChanged();
    return out;
}

bool OfflineQueue::removeById(const QString &id)
{
    bool removed = false;
    {
        QMutexLocker locker(&m_lock);
        for (int i = 0; i < m_items.size(); ++i) {
            if (m_items.at(i).id == id) {
                m_items.removeAt(i);
                persist();
                removed = true;
                break;
            }
        }
    }
    if (removed) {
        notifyChanged();
    }
    return removed;
}

int OfflineQueue::removeForIp(const QString &ip)
{
    int n = 0;
    {
        QMutexLocker locker(&m_lock);
        QList<OfflineItem> keep;
        foreach (const OfflineItem &item, m_items) {
            if (item.ip == ip) {
                ++n;
            } else {
                keep.append(item);
            }
        }
        if (n == 0) {
            return 0;
        }
        m_items = keep;
        persist();
    }
    notifyChanged();
    return n;
}

int OfflineQueue::count() const
{
    QMutexLocker locker(&m_lock);
    return m_items.size();
}

int OfflineQueue::countForIp(const QString &ip) const
{
    QMutexLocker locker(&m_lock);
    int n = 0;
    foreach (const OfflineItem &item, m_items) {
        if (item.ip == ip) {
            ++n;
        }
    }
    return n;
}

OfflineQueueDialog::OfflineQueueDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Offline message queue"));
    resize(560, 360);
    m_countLabel = new QLabel;

    m_tree = new QTreeWidget;
    m_tree->setColumnCount(3);
    QStringList labels;
    labels << tr("Peer") << tr("Time") << tr("Message");
    m_tree->setHeaderLabels(labels);
    m_tree->setRootIsDecorated(false);
    m_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tree->header()->setStretchLastSection(true);

    QPushButton *delButton = new QPushButton(tr("Delete selected"));
    connect(delButton, SIGNAL(clicked()), this, SLOT(deleteSelected()));

    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(box, SIGNAL(rejected()), this, SLOT(reject()));
    connect(box, SIGNAL(accepted()), this, SLOT(accept()));

    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addWidget(delButton);
    buttons->addStretch();
    buttons->addWidget(box);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_countLabel);
    layout->addWidget(new QLabel(tr(
        "Queued text and window-shake tokens are sent in order when the peer "
        "answers discovery. File attachments are not queued (no server).")));
    layout->addWidget(m_tree, 1);
    layout->addLayout(buttons);

    if (Global::offlineQueue) {
        connect(Global::offlineQueue, SIGNAL(queueChanged()),
                this, SLOT(refreshList()));
    }
    refreshList();
}

void OfflineQueueDialog::refreshList()
{
    m_tree->clear();
    if (!Global::offlineQueue) {
        return;
    }
    const QList<OfflineItem> queued = Global::offlineQueue->items();
    setWindowTitle(tr("Offline message queue (%1)").arg(queued.size()));
    if (m_countLabel) {
        m_countLabel->setText(tr("%n offline queued", "", queued.size()));
    }
    foreach (const OfflineItem &item, queued) {
        QTreeWidgetItem *row = new QTreeWidgetItem;
        const QString name = Global::peerLabelForIp(item.ip);
        row->setText(0, QString("%1\n%2").arg(name, item.ip));
        row->setText(1, item.created.toString("yyyy-MM-dd hh:mm:ss"));
        row->setText(2, OfflineQueue::previewText(item));
        row->setData(0, Qt::UserRole, item.id);
        m_tree->addTopLevelItem(row);
    }
    m_tree->resizeColumnToContents(0);
    m_tree->resizeColumnToContents(1);
}

void OfflineQueueDialog::deleteSelected()
{
    if (!Global::offlineQueue) {
        return;
    }
    foreach (QTreeWidgetItem *row, m_tree->selectedItems()) {
        Global::offlineQueue->removeById(row->data(0, Qt::UserRole).toString());
    }
}
