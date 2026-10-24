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

#include <QSettings>
#include <QApplication>
#include <QtDebug>
#include <QDir>

#include "preferences.h"
#include "autostart.h"
#include "constants.h"
#include "global.h"
#include "helper.h"
#include "ipmsg.h"

Preferences::Preferences()
{
    reset();
    load();
}

Preferences::~Preferences()
{
    save();
}

void Preferences::reset()
{
    userName = "";
    groupName = "";
    isSealed = false;
    isNoAutoPopupMsg = false;
    isQuoteMsg = true;
    isReadCheck = true;
    isNoSoundAlarm = false;
    groupNameHistory = "";
    groupNameList.clear();

    userSearchHistory = "";
    userSearchList.clear();
    userSpecifiedBroadcastIpList.clear();

    displayLevel = "";

    // Detail settings
    isSingleClickSendWindow = false;
    noAutoCloseMsgWindowWhenReply = false;
    // End detail settings

    // Other settings
    noticeSound = Helper::soundPath() + "/info.wav";
    quoteMark = ">";

    // Log settings
    isLogMsg = true;
    noLogLockMsgBeforeOpen = true;
    isLogLoginName = false;
    isLogIP = false;
    logFilePath = "";
    openLogFile();
    // End log settings

    // Internel use
    lastSaveFilePath = QDir::homePath();
    lastSendFilePath = QDir::homePath();
    lastSendDirPath = QDir::homePath();
    // End internel use

    transferCodecName = "GBK";

    isAutoRefresh = true;
    autoRefreshSeconds = DEFAULT_AUTO_REFRESH_SECONDS;
    staleMissedRefreshCount = DEFAULT_STALE_MISSED_REFRESH;

    isAway = false;
    isStealth = false;
    isAutoReply = false;
    autoReplyText = "";
    autoReplyCooldownSecs = DEFAULT_AUTO_REPLY_COOLDOWN;
    isNoPopupWhenAway = true;

    isTrayNotify = true;
    isOfflineQueue = true;
    isDoNotDisturb = false;
    isPresenceSound = false;
    onlineSound = Helper::soundPath() + "/info.wav";
    offlineSound = Helper::soundPath() + "/warning.wav";
    isStartWithOs = false;

    isResumeTransFile = true;
    isAutoRecvFile = false;
    isAutoRecvRename = true;
    autoRecvPath = Helper::defaultRecvPath();

    udpPort = IPMSG_DEFAULT_PORT;
    bindAddress.clear();

    personSign = "";
    blacklistIps.clear();
}

void Preferences::load()
{
    qDebug("Preferences::load");

    QSettings *set = Global::settings;

    set->beginGroup("NickName");
    userName = set->value("userName", userName).toString();
    set->endGroup();

    set->beginGroup("GroupName");
    groupName = set->value("groupName", groupName).toString();
    groupNameHistory = set->value("groupNameHistory", groupNameHistory)
        .toString();
    groupNameList = groupNameHistory.split(QChar('\r'));
    set->endGroup();

    set->beginGroup("SendReceive");
    isSealed = set->value("isSealed", isSealed).toBool();
    isReadCheck = set->value("isReadCheck", isReadCheck).toBool();
    isNoSoundAlarm = set->value("isNoSoundAlarm", isNoSoundAlarm).toBool();
    isNoAutoPopupMsg = set->value("isNoAutoPopupMsg",
            isNoAutoPopupMsg).toBool();
    isQuoteMsg = set->value("isQuoteMsg", isQuoteMsg).toBool();
    isDoNotDisturb = set->value("isDoNotDisturb", isDoNotDisturb).toBool();
    set->endGroup();

    set->beginGroup("Other");
    quoteMark = set->value("quoteMark", quoteMark).toString();
    noticeSound = set->value("noticeSound", noticeSound).toString();
    onlineSound = set->value("onlineSound", onlineSound).toString();
    offlineSound = set->value("offlineSound", offlineSound).toString();
    set->endGroup();

    set->beginGroup("Detail");
    isSingleClickSendWindow = set->value("isSingleClickSendWindow", isSingleClickSendWindow).toBool();
    noAutoCloseMsgWindowWhenReply = set->value("noAutoCloseMsgWindowWhenReply", noAutoCloseMsgWindowWhenReply).toBool();
    set->endGroup();

    set->beginGroup("Log");
    isLogMsg = set->value("isLogMsg", isLogMsg).toBool();
    noLogLockMsgBeforeOpen
        = set->value("noLogLockMsgBeforeOpen",
                noLogLockMsgBeforeOpen).toBool();
    isLogLoginName = set->value("isLogLoginName",
            isLogLoginName).toBool();
    isLogIP = set->value("isLogIP",
            isLogIP).toBool();
    logFilePath = set->value("logFilePath", logFilePath).toString();
    openLogFile();
    set->endGroup();

    set->beginGroup("InternalUse");
    lastSaveFilePath = set->value("lastSaveFilePath", lastSaveFilePath).toString();
    lastSendFilePath = set->value("lastSendFilePath", lastSendFilePath).toString();
    lastSendDirPath = set->value("lastSendDirPath", lastSendDirPath).toString();
    set->endGroup();

    set->beginGroup("UserSearch");
    userSearchHistory = set->value("userSearchHistory", userSearchHistory)
        .toString();
    userSearchList = userSearchHistory.split(QChar('\r'));
    set->endGroup();

    set->beginGroup("BroadcastIp");
    userSpecifiedBroadcastIp =
        set->value("userSpecifiedBroadcastIp",
                   userSpecifiedBroadcastIp).toString();
    userSpecifiedBroadcastIpList =
        userSpecifiedBroadcastIp.split(QChar('\r'), Qt::SkipEmptyParts);
    set->endGroup();

    set->beginGroup("Transfer");
    transferCodecName
        = set->value("transferCodecName", transferCodecName).toString();
    if (transferCodecName.compare("GB2312", Qt::CaseInsensitive) == 0) {
        // GBK is a superset and matches classic FeiQ / 飞鸽 peers better.
        transferCodecName = "GBK";
    }
    set->endGroup();

    set->beginGroup("Presence");
    isAutoRefresh = set->value("isAutoRefresh", isAutoRefresh).toBool();
    autoRefreshSeconds = set->value("autoRefreshSeconds",
                                    autoRefreshSeconds).toInt();
    staleMissedRefreshCount = set->value("staleMissedRefreshCount",
                                         staleMissedRefreshCount).toInt();
    isTrayNotify = set->value("isTrayNotify", isTrayNotify).toBool();
    isOfflineQueue = set->value("isOfflineQueue", isOfflineQueue).toBool();
    isPresenceSound = set->value("isPresenceSound", isPresenceSound).toBool();
    set->endGroup();

    set->beginGroup("System");
    isStartWithOs = set->value("isStartWithOs", isStartWithOs).toBool();
    set->endGroup();
    if (!AutoStart::apply(isStartWithOs)) {
        qWarning("Preferences::load: autostart apply failed (enabled=%d)",
                 isStartWithOs ? 1 : 0);
    }

    set->beginGroup("Away");
    isAway = set->value("isAway", isAway).toBool();
    isStealth = set->value("isStealth", isStealth).toBool();
    isAutoReply = set->value("isAutoReply", isAutoReply).toBool();
    autoReplyText = set->value("autoReplyText", autoReplyText).toString();
    autoReplyCooldownSecs = set->value("autoReplyCooldownSecs",
                                       autoReplyCooldownSecs).toInt();
    isNoPopupWhenAway = set->value("isNoPopupWhenAway",
                                   isNoPopupWhenAway).toBool();
    set->endGroup();

    set->beginGroup("FileRecv");
    isResumeTransFile = set->value("isResumeTransFile",
                                   isResumeTransFile).toBool();
    isAutoRecvFile = set->value("isAutoRecvFile", isAutoRecvFile).toBool();
    isAutoRecvRename = set->value("isAutoRecvRename",
                                  isAutoRecvRename).toBool();
    autoRecvPath = set->value("autoRecvPath", autoRecvPath).toString();
    set->endGroup();

    set->beginGroup("Network");
    udpPort = set->value("udpPort", udpPort).toInt();
    if (udpPort < 1 || udpPort > 65535) {
        udpPort = IPMSG_DEFAULT_PORT;
    }
    bindAddress = set->value("bindAddress", bindAddress).toString();
    set->endGroup();

    set->beginGroup("Profile");
    personSign = set->value("personSign", personSign).toString();
    set->endGroup();

    set->beginGroup("Blacklist");
    blacklistIps = set->value("ips", QString()).toString()
            .split(QChar('\r'), Qt::SkipEmptyParts);
    set->endGroup();

}

void Preferences::save()
{
    qDebug("Preferences::save");

    QSettings *set = Global::settings;

    set->beginGroup("NickName");
    set->setValue("userName", userName);
    set->endGroup();

    set->beginGroup("GroupName");
    set->setValue("groupName", groupName);
    set->setValue("groupNameHistory", groupNameList.join("\r"));
    set->endGroup();

    set->beginGroup("SendReceive");
    set->setValue("isSealed", QString("%1").arg(isSealed));
    set->setValue("isNoAutoPopupMsg", QString("%1").arg(isNoAutoPopupMsg));
    set->setValue("isQuoteMsg", QString("%1").arg(isQuoteMsg));
    set->setValue("isReadCheck", QString("%1").arg(isReadCheck));
    set->setValue("isNoSoundAlarm", QString("%1").arg(isNoSoundAlarm));
    set->setValue("isDoNotDisturb", isDoNotDisturb);
    set->endGroup();

    set->beginGroup("Detail");
    set->setValue("isSingleClickSendWindow", isSingleClickSendWindow);
    set->setValue("noAutoCloseMsgWindowWhenReply", noAutoCloseMsgWindowWhenReply);
    set->endGroup();

    set->beginGroup("Other");
    set->setValue("quoteMark", quoteMark);
    set->setValue("noticeSound", noticeSound);
    set->setValue("onlineSound", onlineSound);
    set->setValue("offlineSound", offlineSound);
    set->endGroup();

    set->beginGroup("Log");
    set->setValue("isLogMsg", isLogMsg);
    set->setValue("noLogLockMsgBeforeOpen", noLogLockMsgBeforeOpen);
    set->setValue("isLogLoginName", isLogLoginName);
    set->setValue("isLogIP", isLogIP);
    set->setValue("logFilePath", logFilePath);
    set->endGroup();

    set->beginGroup("InternalUse");
    set->setValue("lastSaveFilePath", lastSaveFilePath);
    set->setValue("lastSendFilePath", lastSendFilePath);
    set->setValue("lastSendDirPath", lastSendDirPath);
    set->endGroup();

    set->beginGroup("UserSearch");
    set->setValue("userSearchHistory", userSearchList.join("\r"));
    set->endGroup();

    set->beginGroup("BroadcastIp");
    set->setValue("userSpecifiedBroadcastIp",
                  userSpecifiedBroadcastIpList.join("\r"));
    set->endGroup();

    set->beginGroup("Transfer");
    set->setValue("transferCodecName", transferCodecName);
    set->endGroup();

    set->beginGroup("Presence");
    set->setValue("isAutoRefresh", isAutoRefresh);
    set->setValue("autoRefreshSeconds", autoRefreshSeconds);
    set->setValue("staleMissedRefreshCount", staleMissedRefreshCount);
    set->setValue("isTrayNotify", isTrayNotify);
    set->setValue("isOfflineQueue", isOfflineQueue);
    set->setValue("isPresenceSound", isPresenceSound);
    set->endGroup();

    set->beginGroup("System");
    set->setValue("isStartWithOs", isStartWithOs);
    set->endGroup();
    if (!AutoStart::apply(isStartWithOs)) {
        qWarning("Preferences::save: autostart apply failed (enabled=%d)",
                 isStartWithOs ? 1 : 0);
    }

    set->beginGroup("Away");
    set->setValue("isAway", isAway);
    set->setValue("isStealth", isStealth);
    set->setValue("isAutoReply", isAutoReply);
    set->setValue("autoReplyText", autoReplyText);
    set->setValue("autoReplyCooldownSecs", autoReplyCooldownSecs);
    set->setValue("isNoPopupWhenAway", isNoPopupWhenAway);
    set->endGroup();

    set->beginGroup("FileRecv");
    set->setValue("isResumeTransFile", isResumeTransFile);
    set->setValue("isAutoRecvFile", isAutoRecvFile);
    set->setValue("isAutoRecvRename", isAutoRecvRename);
    set->setValue("autoRecvPath", autoRecvPath);
    set->endGroup();

    set->beginGroup("Network");
    set->setValue("udpPort", udpPort);
    set->setValue("bindAddress", bindAddress);
    set->endGroup();

    set->beginGroup("Profile");
    set->setValue("personSign", personSign);
    set->endGroup();

    set->beginGroup("Blacklist");
    set->setValue("ips", blacklistIps.join("\r"));
    set->endGroup();
}

quint16 Preferences::listenPort() const
{
    if (udpPort < 1 || udpPort > 65535) {
        return IPMSG_DEFAULT_PORT;
    }
    return static_cast<quint16>(udpPort);
}

QHostAddress Preferences::listenAddress() const
{
    const QString text = bindAddress.trimmed();
    if (text.isEmpty()
            || text == QLatin1String("0.0.0.0")
            || text.compare(QLatin1String("any"), Qt::CaseInsensitive) == 0) {
        return QHostAddress(QHostAddress::AnyIPv4);
    }

    const QHostAddress address(text);
    if (address.isNull()) {
        return QHostAddress(QHostAddress::AnyIPv4);
    }
    return address;
}

void Preferences::openLogFile()
{
    if (logFile.isOpen()) {
        logFile.close();
    }

    logFile.setFileName(logFilePath);

    if (!logFile.open(QIODevice::Append)) {
        return;
    }
}

