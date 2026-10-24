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

#include "owner.h"
#include "global.h"
#include "helper.h"
#include "send_msg.h"
#include "user_manager.h"
#include "preferences.h"
#include "constants.h"
#include "msg_thread.h"

#include <QColor>
#include <QDateTime>
#include <QStandardItemModel>
#include <QStringList>
#include <QTimer>

Owner UserManager::m_ourself;

UserManager::UserManager(QObject *parent)
    : QObject(parent), m_refreshTimer(0)
{
    updateOurself();
    createModel();

    m_refreshTimer = new QTimer(this);
    connect(m_refreshTimer, SIGNAL(timeout()),
            this, SLOT(autoRefreshPresence()));
    applyRefreshTimer();
}

void UserManager::updateOurself()
{
    if (Global::preferences->userName.isEmpty()) {
        m_ourself.setName(Helper::loginName());
    } else {
        m_ourself.setName(Global::preferences->userName);
    }

    m_ourself.setGroup(Global::preferences->groupName);
    m_ourself.setLoginName(Helper::loginName());
    m_ourself.setHost(Helper::hostname());
    m_ourself.setDisplayLevel(Global::preferences->displayLevel);
}

void UserManager::createModel()
{
    QStringList labels;
    labels << tr("user") << tr("group") << tr("host") << tr("ip")
        << tr("logname") << tr("display level") << tr("status");

    m_model = new QStandardItemModel(0, labels.size(), this);
    m_model->setHorizontalHeaderLabels(labels);
}

void UserManager::newUserMsg(Msg msg)
{
    if (!msg->owner().group().isEmpty()
            && !Global::preferences->groupNameList
            .contains(msg->owner().group())) {
        Global::preferences->groupNameList.prepend(msg->owner().group());
    }

    const bool absence =
        GET_MODE(msg->flags()) == IPMSG_BR_ABSENCE
        || (msg->flags() & IPMSG_ABSENCEOPT);

    if (contains(msg->ip())) {
        updateUser(msg->owner(), ipToRow(msg->ip()));
        touchUser(ipToRow(msg->ip()), absence);
    } else {
        int row = m_model->rowCount();
        m_model->insertRow(row);
        addUser(msg->owner(), row);
        touchUser(row, absence);

        emit userCountUpdated(m_model->rowCount());
    }
}

void UserManager::newExitMsg(Msg msg)
{
    int row;
    if ((row = ipToRow(msg->ip())) != -1) {
        m_model->removeRow(row);

        emit userCountUpdated(m_model->rowCount());
    }
}

void UserManager::updateUser(const Owner &owner, int row)
{
    m_model->setData(m_model->index(row, USER_VIEW_NAME_COLUMN),
                       owner.name());
    m_model->setData(m_model->index(row, USER_VIEW_GROUP_COLUMN),
                       owner.group());
    m_model->setData(m_model->index(row, USER_VIEW_HOST_COLUMN),
                       owner.host());
    m_model->setData(m_model->index(row, USER_VIEW_IP_COLUMN),
                       owner.ip());
    m_model->setData(m_model->index(row, USER_VIEW_LOGIN_NAME_COLUMN),
                       owner.loginName());
    m_model->setData(m_model->index(row, USER_VIEW_DISPLAY_LEVEL_COLUMN),
                       owner.displayLevel());
}

void UserManager::addUser(const Owner &owner, int row)
{
    updateUser(owner, row);
}

void UserManager::touchUser(int row, bool absence)
{
    m_model->setData(m_model->index(row, USER_VIEW_IP_COLUMN),
                     QDateTime::currentDateTime(), Qt::UserRole);
    setRowPresence(row, absence ? PRESENCE_AWAY : PRESENCE_ONLINE);
}

void UserManager::setRowPresence(int row, int presence)
{
    QString text;
    QColor color;
    switch (presence) {
    case PRESENCE_AWAY:
        text = tr("Away");
        color = QColor(180, 120, 0);
        break;
    case PRESENCE_STALE:
        text = tr("Stale");
        color = QColor(128, 128, 128);
        break;
    default:
        text = tr("Online");
        break;
    }

    m_model->setData(m_model->index(row, USER_VIEW_STATUS_COLUMN), text);
    m_model->setData(m_model->index(row, USER_VIEW_IP_COLUMN),
                     presence, Qt::UserRole + 1);

    const QVariant fg = color.isValid() ? QVariant(color) : QVariant();
    for (int col = 0; col < m_model->columnCount(); ++col) {
        m_model->setData(m_model->index(row, col), fg, Qt::ForegroundRole);
    }
}

void UserManager::applyRefreshTimer()
{
    if (!m_refreshTimer) {
        return;
    }

    const int seconds = qMax(5, Global::preferences->autoRefreshSeconds);
    if (Global::preferences->isAutoRefresh) {
        m_refreshTimer->start(seconds * 1000);
    } else {
        m_refreshTimer->stop();
    }
}

void UserManager::autoRefreshPresence()
{
    announcePresence();
    sweepStaleUsers();
}

void UserManager::sweepStaleUsers()
{
    const int interval = qMax(5, Global::preferences->autoRefreshSeconds);
    const int missed = qMax(1, Global::preferences->staleMissedRefreshCount);
    const qint64 staleMs = qint64(interval) * missed * 1000;
    const qint64 removeMs = staleMs * 2;
    const QDateTime now = QDateTime::currentDateTime();

    bool removed = false;
    for (int row = m_model->rowCount() - 1; row >= 0; --row) {
        QDateTime lastSeen = m_model->data(
                    m_model->index(row, USER_VIEW_IP_COLUMN),
                    Qt::UserRole).toDateTime();
        if (!lastSeen.isValid()) {
            lastSeen = now;
            m_model->setData(m_model->index(row, USER_VIEW_IP_COLUMN),
                             lastSeen, Qt::UserRole);
        }

        const qint64 age = lastSeen.msecsTo(now);
        if (age >= removeMs) {
            m_model->removeRow(row);
            removed = true;
        } else if (age >= staleMs) {
            const int presence = m_model->data(
                        m_model->index(row, USER_VIEW_IP_COLUMN),
                        Qt::UserRole + 1).toInt();
            if (presence != PRESENCE_STALE) {
                setRowPresence(row, PRESENCE_STALE);
            }
        }
    }

    if (removed) {
        emit userCountUpdated(m_model->rowCount());
    }
}

quint32 UserManager::presenceFlags(quint32 command) const
{
    quint32 flags = command | QIPMSG_CAPACITY;
    if (Global::preferences->isAway) {
        flags |= IPMSG_ABSENCEOPT;
    }
    return flags;
}

void UserManager::broadcastExit() const
{
    qDebug("UserManager::broadcastExit");

    SendMsg sendMsg(QHostAddress::Null, 0/* port */,
                    exitMessage(), ""/* extendedInfo */,
                    presenceFlags(IPMSG_BR_EXIT));

    Global::msgThread->addSendMsg(Msg(sendMsg));
}

QString UserManager::entryMessage() const
{
    return QString("%1%2%3%4").arg(m_ourself.name())
        .arg(QChar('\0'))
        .arg(m_ourself.group())
        .arg(QChar('\0'));
}

QString UserManager::exitMessage() const
{
    return entryMessage();
}

void UserManager::broadcastEntry() const
{
    qDebug("UserManager::broadcastEntry");

    SendMsg sendMsg(QHostAddress::Null, 0/* port */,
                    entryMessage(), ""/* extendedInfo */,
                    presenceFlags(IPMSG_BR_ENTRY));

    Global::msgThread->addSendMsg(Msg(sendMsg));
}

void UserManager::broadcastAbsence() const
{
    qDebug("UserManager::broadcastAbsence");

    SendMsg sendMsg(QHostAddress::Null, 0/* port */,
                    entryMessage(), ""/* extendedInfo */,
                    presenceFlags(IPMSG_BR_ABSENCE) | IPMSG_ABSENCEOPT);

    Global::msgThread->addSendMsg(Msg(sendMsg));
}

void UserManager::announcePresence() const
{
    if (Global::preferences->isAway) {
        broadcastAbsence();
    } else {
        broadcastEntry();
    }
}

bool UserManager::contains(QString ip) const
{
    return ipToRow(ip) != -1;
}

QString UserManager::ip(int row) const
{
    return m_model->data(m_model->index(row, USER_VIEW_IP_COLUMN))
        .toString();
}

int UserManager::ipToRow(QString ip) const
{
    for (int row = 0; row < m_model->rowCount(); ++row) {
        if (this->ip(row) == ip) {
            return row;
        }
    }

    return -1;
}

QString UserManager::name(int row) const
{
    return m_model->data(m_model->index(row, USER_VIEW_NAME_COLUMN))
        .toString();
}

QString UserManager::group(int row) const
{
    return m_model->data(m_model->index(row, USER_VIEW_GROUP_COLUMN))
        .toString();
}

QString UserManager::host(int row) const
{
    return m_model->data(m_model->index(row, USER_VIEW_HOST_COLUMN))
        .toString();
}

QString UserManager::loginName(int row) const
{
    return m_model->data(m_model->index(row, USER_VIEW_LOGIN_NAME_COLUMN))
        .toString();
}

QString UserManager::status(int row) const
{
    return m_model->data(m_model->index(row, USER_VIEW_STATUS_COLUMN))
        .toString();
}

int UserManager::userCount() const
{
    return m_model->rowCount();
}

QString UserManager::displayNameForIp(const QString &ip) const
{
    const int row = ipToRow(ip);
    if (row != -1) {
        const QString n = name(row);
        if (!n.isEmpty()) {
            return n;
        }
    }
    return ip;
}

QString UserManager::peerCaption(const QString &ip) const
{
    const int row = ipToRow(ip);
    if (row == -1) {
        return ip;
    }

    const QString groupName = group(row);
    const QString hostName = host(row);
    const QString extra = groupName.isEmpty()
            ? hostName
            : groupName + "/" + hostName;
    return QString("%1 (%2) %3").arg(name(row), extra, ip);
}
