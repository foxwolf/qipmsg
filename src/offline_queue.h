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

class QLabel;

class QTreeWidget;

class OfflinePeerHint
{
public:
    QString ip;
    QString login;
    QString host;
    bool reachable;
    OfflinePeerHint() : reachable(false) {}
};

class OfflineItem
{
public:
    QString id;
    QString ip;
    QString text;
    QString kind;
    QString login;
    QString host;
    QDateTime created;
};

class OfflineQueue : public QObject
{
    Q_OBJECT

public:
    OfflineQueue(QObject *parent = 0);

    static void setFilePathForTest(const QString &path);
    static QString matchReachableIp(const QString &login, const QString &host,
                                    const QList<OfflinePeerHint> &peers);
    static QString previewText(const OfflineItem &item);

    QString enqueue(const QString &ip, const QString &text,
                    const QString &kind = QString(),
                    const QString &login = QString(),
                    const QString &host = QString());
    bool assignIp(const QString &id, const QString &newIp);
    void restore(const QList<OfflineItem> &items);
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
    QLabel *m_countLabel;
};

#endif // !OFFLINE_QUEUE_H
