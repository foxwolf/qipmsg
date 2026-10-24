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

#ifndef OFFLINE_QUEUE_H
#define OFFLINE_QUEUE_H

#include <QDateTime>
#include <QDialog>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QString>

class QTreeWidget;

class OfflineItem
{
public:
    QString id;
    QString ip;
    QString text;
    QDateTime created;
};

class OfflineQueue : public QObject
{
    Q_OBJECT

public:
    OfflineQueue(QObject *parent = 0);

    QString enqueue(const QString &ip, const QString &text);
    QList<OfflineItem> items() const;
    QList<OfflineItem> itemsForIp(const QString &ip) const;
    QList<OfflineItem> takeForIp(const QString &ip);
    bool removeById(const QString &id);
    int removeForIp(const QString &ip);
    int count() const;
    int countForIp(const QString &ip) const;

signals:
    void queueChanged();

private:
    QString filePath() const;
    void load();
    void persist() const;
    void notifyChanged();

    mutable QMutex m_lock;
    QList<OfflineItem> m_items;
    quint64 m_nextId;
};

class OfflineQueueDialog : public QDialog
{
    Q_OBJECT

public:
    OfflineQueueDialog(QWidget *parent = 0);

private slots:
    void refreshList();
    void deleteSelected();

private:
    QTreeWidget *m_tree;
};

#endif // !OFFLINE_QUEUE_H
