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

#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include "owner.h"
#include "msg.h"

#include <QObject>

class QStandardItemModel;
class QTimer;

class UserManager : public QObject
{
    Q_OBJECT

public:
    friend class MainWindow;

    UserManager(QObject *parent = 0);
    ~UserManager();

    void broadcastExit() const;
    void broadcastEntry() const;
    void broadcastAbsence() const;
    void announcePresence() const;
    void applyRefreshTimer();

    bool contains(QString ip) const;

    QString ip(int row) const;
    int ipToRow(QString ip) const;

    QString name(int row) const;
    QString group(int row) const;
    QString host(int row) const;
    QString loginName(int row) const;
    QString status(int row) const;
    int presence(int row) const;
    int presenceForIp(const QString &ip) const;
    int userCount() const;
    int onlineCount() const;
    QString displayNameForIp(const QString &ip) const;
    QString peerCaption(const QString &ip) const;
    QString signature(int row) const;
    QString signatureForIp(const QString &ip) const;

    Owner ourself() const { return m_ourself; }
    void updateOurself();

    QString entryMessage() const;
    QString exitMessage() const;

signals:
    void userCountUpdated(int userCount);
    void usersChanged();

private slots:
    void newUserMsg(Msg msg);
    void newExitMsg(Msg msg);
    void autoRefreshPresence();
    void savePersistedPeers();

private:
    void createModel();
    void updateUser(const Owner &owner, int row);
    void addUser(const Owner &owner, int row);
    void touchUser(int row, bool absence);
    void setRowPresence(int row, int presence);
    void sweepStaleUsers();
    quint32 presenceFlags(quint32 command) const;
    void loadPersistedPeers();
    void scheduleSavePeers();
    QString peersFilePath() const;
    void notifyUsersChanged();

    static Owner m_ourself;

    QStandardItemModel *m_model;
    QTimer *m_refreshTimer;
    QTimer *m_saveTimer;
};

#endif // !USER_MANAGER_H
