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

#include "global.h"
#include "preferences.h"
#include "setup_window.h"
#include "user_manager.h"
#include "transfer_codec.h"
#include "helper.h"
#include "protocolutil.h"
#include "autostart.h"
#include "systray.h"

#include <QtGui>
#include <QtCore>
#include <QtWidgets>
#include <QRegularExpression>
#include <QScreen>
#include <QSpinBox>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QAbstractSocket>

DetailTab::DetailTab(QWidget *parent)
    : QWidget(parent)
{
    autoMinimizeCheckBox
        = new QCheckBox(tr("Minimize message readed alert window"));
    autoMinimizeCheckBox->setEnabled(false);

    singleClickCheckBox
        = new QCheckBox(tr("Single click to open send window (default"
                    " double click)"));
    if (Global::preferences->isSingleClickSendWindow) {
        singleClickCheckBox->setCheckState(Qt::Checked);
    }
    enableHotKeyCheckBox = new QCheckBox(tr("Enable HotKey (Ctrl+Alt+s, R)"));
    enableHotKeyCheckBox->setEnabled(false);

    buttonAliasCheckBox
        = new QCheckBox(tr("Change [Send/Reply] button name to"
                    " [Fire/Retrun Fire]"));
    buttonAliasCheckBox->setEnabled(false);

    noAutoCloseReplyCheckBox
        = new QCheckBox(tr("No auto close receive message window when reply"));
    if (Global::preferences->noAutoCloseMsgWindowWhenReply) {
        noAutoCloseReplyCheckBox->setCheckState(Qt::Checked);
    }

    noPopupMessageWhenAway = new QCheckBox(tr("Not popup message when away"));
    if (Global::preferences->isNoPopupWhenAway) {
        noPopupMessageWhenAway->setCheckState(Qt::Checked);
    }

    autoRefreshCheckBox = new QCheckBox(tr("Auto refresh user list (FeiQ AutoRefresh)"));
    if (Global::preferences->isAutoRefresh) {
        autoRefreshCheckBox->setCheckState(Qt::Checked);
    }

    refreshIntervalSpin = new QSpinBox;
    refreshIntervalSpin->setRange(5, 600);
    refreshIntervalSpin->setSuffix(tr(" seconds"));
    refreshIntervalSpin->setValue(Global::preferences->autoRefreshSeconds);

    staleMissedSpin = new QSpinBox;
    staleMissedSpin->setRange(1, 20);
    staleMissedSpin->setValue(Global::preferences->staleMissedRefreshCount);

    trayNotifyCheckBox = new QCheckBox(tr("Show tray notification on new message"));
    if (Global::preferences->isTrayNotify) {
        trayNotifyCheckBox->setCheckState(Qt::Checked);
    }

    startWithOsCheck = new QCheckBox(tr("Start with the system"));
    startWithOsCheck->setToolTip(AutoStart::describe());
    if (!AutoStart::supported()) {
        startWithOsCheck->setEnabled(false);
    }
    if (Global::preferences->isStartWithOs) {
        startWithOsCheck->setCheckState(Qt::Checked);
    }

    QLabel *intervalLabel = new QLabel(tr("Refresh interval:"));
    QLabel *staleLabel = new QLabel(tr("Mark stale after missed refreshes:"));

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(autoMinimizeCheckBox, 0, 0, 1, 2);
    layout->addWidget(singleClickCheckBox, 1, 0, 1, 2);
    layout->addWidget(enableHotKeyCheckBox, 2, 0, 1, 2);
    layout->addWidget(buttonAliasCheckBox, 3, 0, 1, 2);
    layout->addWidget(noAutoCloseReplyCheckBox, 4, 0, 1, 2);
    layout->addWidget(noPopupMessageWhenAway, 5, 0, 1, 2);
    layout->addWidget(autoRefreshCheckBox, 6, 0, 1, 2);
    layout->addWidget(intervalLabel, 7, 0);
    layout->addWidget(refreshIntervalSpin, 7, 1);
    layout->addWidget(staleLabel, 8, 0);
    layout->addWidget(staleMissedSpin, 8, 1);
    layout->addWidget(trayNotifyCheckBox, 9, 0, 1, 2);
    layout->addWidget(startWithOsCheck, 10, 0, 1, 2);
    QLabel *startHint = new QLabel(AutoStart::describe());
    startHint->setWordWrap(true);
    layout->addWidget(startHint, 11, 0, 1, 2);

    setLayout(layout);
}

void DetailTab::save()
{
    Global::preferences->isSingleClickSendWindow
        = singleClickCheckBox->isChecked();
    Global::preferences->noAutoCloseMsgWindowWhenReply
        = noAutoCloseReplyCheckBox->isChecked();
    Global::preferences->isNoPopupWhenAway
        = noPopupMessageWhenAway->isChecked();
    Global::preferences->isAutoRefresh = autoRefreshCheckBox->isChecked();
    Global::preferences->autoRefreshSeconds = refreshIntervalSpin->value();
    Global::preferences->staleMissedRefreshCount = staleMissedSpin->value();
    Global::preferences->isTrayNotify = trayNotifyCheckBox->isChecked();
    Global::preferences->isStartWithOs = startWithOsCheck->isChecked();
    Global::userManager->applyRefreshTimer();
}

LogTab::LogTab(QWidget *parent)
    : QWidget(parent)
{
    isLogMsgCheckBox = new QCheckBox(tr("Enable log message"));
    if (Global::preferences->isLogMsg) {
        isLogMsgCheckBox->setCheckState(Qt::Checked);
    }
    noLogLockMsgBeforeOpenCheckBox
        = new QCheckBox(tr("Not log lock message before open"));
    noLogLockMsgBeforeOpenCheckBox->setEnabled(false);
    if (Global::preferences->noLogLockMsgBeforeOpen) {
        noLogLockMsgBeforeOpenCheckBox->setCheckState(Qt::Unchecked);
    }
    isLogLoginNameCheckBox = new QCheckBox(tr("Log nickname and login name"));
    if (Global::preferences->isLogLoginName) {
        isLogLoginNameCheckBox->setCheckState(Qt::Checked);
    }
    isLogIPCheckBox = new QCheckBox(tr("Log ip address"));
    if (Global::preferences->isLogIP) {
        isLogIPCheckBox->setCheckState(Qt::Checked);
    }

    logFileButton = new QPushButton(tr("Log File"));
    logFileLabel = new QLabel;
    int frameStyle = QFrame::Sunken | QFrame::Panel;
    logFileLabel->setFrameStyle(frameStyle);
    logFileLabel->setText(Global::preferences->logFilePath);

    connect(logFileButton, SIGNAL(clicked()),
            this, SLOT(getLogFilePath()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(isLogMsgCheckBox);
    mainLayout->addWidget(noLogLockMsgBeforeOpenCheckBox);
    mainLayout->addWidget(isLogLoginNameCheckBox);
    mainLayout->addWidget(isLogIPCheckBox);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(logFileButton, 0, 0);
    layout->addWidget(logFileLabel, 0, 1);
    layout->setColumnStretch(0, 10);
    layout->setColumnStretch(1, 30);

    mainLayout->addLayout(layout);

    setLayout(mainLayout);
}

TransferTab::TransferTab(QWidget *parent)
    : QWidget(parent)
{
    createCodecComboBox();

    QLabel *label = new QLabel(tr("Transfer Codec:"));
    QLabel *hint = new QLabel(tr(
        "GBK/GB2312 is recommended for FeiQ / 飞鸽 peers. "
        "UTF-8 is optional for modern clients."));
    hint->setWordWrap(true);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(label, 0, 0);
    mainLayout->addWidget(codecComboBox, 0, 1);
    mainLayout->addWidget(hint, 1, 0, 1, 2);

    mainLayout->setColumnStretch(0, 10);
    mainLayout->setColumnStretch(1, 30);

    setLayout(mainLayout);
}

void TransferTab::createCodecComboBox()
{
    QMap<QString, QString> codecMap;
    QRegularExpression iso8859RegExp("ISO[- ]8859-([0-9]+).*");

    foreach (QByteArray name, QTextCodec::availableCodecs()) {
        QTextCodec *codec = QTextCodec::codecForName(name);
        if (!codec) {
            continue;
        }

        QString sortKey = codec->name().toUpper();
        int rank;

        if (sortKey.startsWith("UTF-8")) {
            rank = 1;
        } else if (sortKey.startsWith("UTF-16")) {
            rank = 2;
        } else if (iso8859RegExp.match(sortKey).hasMatch()) {
            QRegularExpressionMatch match = iso8859RegExp.match(sortKey);
            if (match.captured(1).size() == 1)
                rank = 3;
            else
                rank = 4;
        } else {
            rank = 5;
        }
        sortKey.prepend(QChar('0' + rank));

        codecMap.insert(sortKey, QString(name));
    }
    QStringList codecNames = codecMap.values();

    codecComboBox = new QComboBox();
    codecComboBox->setDuplicatesEnabled(false);

    codecComboBox->addItem("GBK");
    codecComboBox->addItem("GB2312");
    codecComboBox->addItem("UTF-8");
    codecComboBox->addItems(codecNames);
    int i = codecComboBox->findText(Global::preferences->transferCodecName);
    codecComboBox->setCurrentIndex(i);
}

void TransferTab::save()
{
    Global::preferences->transferCodecName = codecComboBox->currentText();
    Global::transferCodec->setTransCodec(codecComboBox->currentText());
}

FileTab::FileTab(QWidget *parent)
    : QWidget(parent)
{
    resumeCheckBox = new QCheckBox(tr("Resume interrupted file transfers"));
    if (Global::preferences->isResumeTransFile) {
        resumeCheckBox->setCheckState(Qt::Checked);
    }
    autoRecvCheckBox = new QCheckBox(tr("Auto-accept incoming files"));
    if (Global::preferences->isAutoRecvFile) {
        autoRecvCheckBox->setCheckState(Qt::Checked);
    }
    autoRenameCheckBox = new QCheckBox(tr("Auto-rename if the file already exists"));
    if (Global::preferences->isAutoRecvRename) {
        autoRenameCheckBox->setCheckState(Qt::Checked);
    }

    recvPathButton = new QPushButton(tr("Receive folder"));
    recvPathLabel = new QLabel(Global::preferences->autoRecvPath);
    recvPathLabel->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    connect(recvPathButton, SIGNAL(clicked()), this, SLOT(getRecvPath()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(resumeCheckBox);
    mainLayout->addWidget(autoRecvCheckBox);
    mainLayout->addWidget(autoRenameCheckBox);
    QHBoxLayout *pathLayout = new QHBoxLayout;
    pathLayout->addWidget(recvPathButton);
    pathLayout->addWidget(recvPathLabel, 1);
    mainLayout->addLayout(pathLayout);
    mainLayout->addStretch();
    setLayout(mainLayout);
}

void FileTab::getRecvPath()
{
    const QString path = QFileDialog::getExistingDirectory(
                this, tr("Select receive folder"),
                recvPathLabel->text());
    if (!path.isEmpty()) {
        recvPathLabel->setText(path);
    }
}

void FileTab::save()
{
    Global::preferences->isResumeTransFile = resumeCheckBox->isChecked();
    Global::preferences->isAutoRecvFile = autoRecvCheckBox->isChecked();
    Global::preferences->isAutoRecvRename = autoRenameCheckBox->isChecked();
    if (!recvPathLabel->text().isEmpty()) {
        Global::preferences->autoRecvPath = recvPathLabel->text();
    }
}

NetworkTab::NetworkTab(QWidget *parent)
    : QWidget(parent)
{
    portSpin = new QSpinBox;
    portSpin->setRange(1, 65535);
    portSpin->setValue(Global::preferences->listenPort());

    bindCombo = new QComboBox;
    bindCombo->setEditable(true);
    fillBindCombo();

    QLabel *portLabel = new QLabel(tr("UDP listen port"));
    QLabel *bindLabel = new QLabel(tr("Bind address"));
    QLabel *hint = new QLabel(tr(
        "Changes take effect after restart. UDP discovery and TCP file "
        "transfer use the same port (IPMSG / FeiQ, default 2425). "
        "Empty / 0.0.0.0 binds any IPv4 NIC. Bind failure is logged and "
        "the app continues (soft-fail)."));
    hint->setWordWrap(true);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(portLabel, 0, 0);
    layout->addWidget(portSpin, 0, 1);
    layout->addWidget(bindLabel, 1, 0);
    layout->addWidget(bindCombo, 1, 1);
    layout->addWidget(hint, 2, 0, 1, 2);
    layout->setColumnStretch(0, 10);
    layout->setColumnStretch(1, 30);
    setLayout(layout);
}

void NetworkTab::fillBindCombo()
{
    bindCombo->addItem(tr("Any IPv4 (0.0.0.0)"), QString());

    foreach (const QNetworkInterface networkInterface,
             QNetworkInterface::allInterfaces()) {
        const QNetworkInterface::InterfaceFlags flags = networkInterface.flags();
        if (!(flags & QNetworkInterface::IsUp)
                || (flags & QNetworkInterface::IsLoopBack)) {
            continue;
        }
        foreach (const QNetworkAddressEntry entry,
                 networkInterface.addressEntries()) {
            const QHostAddress ip = entry.ip();
            if (ip.protocol() != QAbstractSocket::IPv4Protocol
                    || ip.isLoopback()
                    || ip.isNull()) {
                continue;
            }
            const QString text = QString("%1 (%2)")
                    .arg(ip.toString(), networkInterface.humanReadableName());
            bindCombo->addItem(text, ip.toString());
        }
    }

    const QString current = Global::preferences->bindAddress.trimmed();
    if (current.isEmpty()) {
        bindCombo->setCurrentIndex(0);
        return;
    }

    int index = bindCombo->findData(current);
    if (index < 0) {
        bindCombo->addItem(current, current);
        index = bindCombo->findData(current);
    }
    bindCombo->setCurrentIndex(index);
}

void NetworkTab::save()
{
    Global::preferences->udpPort = portSpin->value();

    const QString typed = bindCombo->currentText().trimmed();
    QString address = bindCombo->currentData().toString();
    if (bindCombo->currentIndex() == 0
            && (typed.isEmpty()
                || typed == bindCombo->itemText(0)
                || typed == QLatin1String("0.0.0.0"))) {
        address.clear();
    } else if (address.isEmpty()) {
        QHostAddress parsed(typed);
        address = parsed.isNull() ? QString() : parsed.toString();
    }
    Global::preferences->bindAddress = address;
}

void LogTab::getLogFilePath()
{
    QFileDialog::Options options = QFileDialog::DontConfirmOverwrite;
    QString selectedFilter;
    QString path = Global::preferences->logFilePath;
    QString fileName
        = QFileDialog::getSaveFileName(this,
                tr("Select Log File"),
                path,
                tr("Text Files (*.log);;All Files (*)"),
                &selectedFilter,
                options);

    if (!fileName.isEmpty()) {
        logFileLabel->setText(fileName);
    }
}

void LogTab::save()
{
    Global::preferences->isLogMsg = isLogMsgCheckBox->isChecked();
    Global::preferences->noLogLockMsgBeforeOpen
        = noLogLockMsgBeforeOpenCheckBox->isChecked();
    Global::preferences->isLogLoginName = isLogLoginNameCheckBox->isChecked();
    Global::preferences->isLogIP = isLogIPCheckBox->isChecked();
    Global::preferences->logFilePath = logFileLabel->text();
    Global::preferences->openLogFile();
}

OtherTab::OtherTab(QWidget *parent)
    : QWidget(parent)
{
    quoteMarkButton = new QPushButton(tr("Quote Mark"));
    quoteMarkButton->setFlat(true);
    quoteMarkLineEdit = new QLineEdit;
    quoteMarkLineEdit->setText(Global::preferences->quoteMark);

    int frameStyle = QFrame::Sunken | QFrame::Panel;

    noticeSoundButton = new QPushButton(tr("Notice Sound"));
    noticeSoundLabel = new QLabel(Global::preferences->noticeSound);
    noticeSoundLabel->setFrameStyle(frameStyle);
    connect(noticeSoundButton, SIGNAL(clicked()),
            this, SLOT(getNoticeSoundPath()));

    onlineIconButton = new QPushButton(tr("Online Icon"));
    onlineIconButton->setDisabled(true);
    onlineIconLabel = new QLabel;
    onlineIconLabel->setFrameStyle(frameStyle);

    awayIconButton = new QPushButton(tr("Away Icon"));
    awayIconButton->setDisabled(true);
    awayIconLabel = new QLabel;
    awayIconLabel->setFrameStyle(frameStyle);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(quoteMarkButton, 0, 0);
    layout->addWidget(quoteMarkLineEdit, 0, 1);
    layout->addWidget(noticeSoundButton, 1, 0);
    layout->addWidget(noticeSoundLabel, 1, 1);
    layout->addWidget(onlineIconButton, 2, 0);
    layout->addWidget(onlineIconLabel, 2, 1);
    layout->addWidget(awayIconButton, 3, 0);
    layout->addWidget(awayIconLabel, 3, 1);

    layout->setColumnStretch(0, 10);
    layout->setColumnStretch(1, 30);

    setLayout(layout);
}

void OtherTab::getNoticeSoundPath()
{
    QFileDialog::Options options;
    QString selectedFilter;
    QString path = Global::preferences->noticeSound;
    QString fileName
        = QFileDialog::getOpenFileName(this,
                tr("Select Sound File"),
                path,
                tr("Audio Files (*.wav)"),
                &selectedFilter,
                options);

    if (!fileName.isEmpty()) {
        noticeSoundLabel->setText(fileName);
    }
}

void OtherTab::save()
{
    if (!quoteMarkLineEdit->text().isEmpty()) {
        Global::preferences->quoteMark = quoteMarkLineEdit->text();
    }

    if (!noticeSoundLabel->text().isEmpty()) {
        Global::preferences->noticeSound = noticeSoundLabel->text();
    }
}

DetailSetupDialog::DetailSetupDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Detail/Log Settings"));
    setWindowModality(Qt::WindowModal);

    detailTab = new DetailTab;
    logTab = new LogTab;
    otherTab = new OtherTab;
    transferTab = new TransferTab;
    fileTab = new FileTab;
    networkTab = new NetworkTab;

    tabWidget = new QTabWidget;
    tabWidget->setUsesScrollButtons(true);
    tabWidget->addTab(detailTab, tr("Detail"));
    tabWidget->addTab(logTab, tr("Log"));
    tabWidget->addTab(otherTab, tr("Other"));
    tabWidget->addTab(transferTab, tr("Transfer"));
    tabWidget->addTab(fileTab, tr("File"));
    tabWidget->addTab(networkTab, tr("Network"));

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
            | QDialogButtonBox::Apply);

    connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(save()));
    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
            this, SLOT(apply()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    adjustSize();
}

QSize DetailSetupDialog::sizeHint() const
{
    return QSize(360, 240);
}

void DetailSetupDialog::save()
{
    apply();

    close();
}

void DetailSetupDialog::apply()
{
    detailTab->save();
    otherTab->save();
    logTab->save();
    transferTab->save();
    fileTab->save();
    networkTab->save();

    Global::preferences->save();
    if (Global::systray) {
        Global::systray->syncActions();
    }
}

SetupWindow::SetupWindow(QWidget *parent)
    : QWidget(parent)
{
    // setModal(true);
    setAttribute(Qt::WA_DeleteOnClose, true);

    createNicknameGroupBox();
    createGroupNameGroupBox();
    createSendRecvSettingsGroupBox();
    createBroadcastGroupBox();

    createButtonLayout();

    createMiscSettingGroupBox();

    QGridLayout *mainLayout = new QGridLayout(this);

    mainLayout->addWidget(nicknameGroupBox, 0, 0);
    mainLayout->addWidget(groupNameGroupBox, 0, 1);
    mainLayout->addWidget(sendReceiveSettingsGroupBox, 1, 0, 1, 1);
    mainLayout->addWidget(miscSettingGroupBox, 1, 1);
    mainLayout->addWidget(broadcastGroupBox, 2, 0, 1, 2);
    mainLayout->addLayout(buttonsLayout, 3, 0, 1, 2);

    createConnections();

    setWindowTitle(tr("Setup Preferences"));
    setWindowIcon(*Global::iconSet.value("normal"));

    setLayout(mainLayout);

    adjustSize();

    // Put the dialog in the middle of screen
    QRect rect = QGuiApplication::primaryScreen()->geometry();
    move((rect.width() - width()) / 2, (rect.height() - height()) / 2);
}

void SetupWindow::createNicknameGroupBox()
{
    nicknameGroupBox = new QGroupBox(tr("Nickname"));
    nicknameGroupBox->setAlignment(Qt::AlignHCenter);

    nickNameEdit = new QLineEdit();
    nickNameEdit->setText(Global::preferences->userName);
    personSignEdit = new QLineEdit();
    personSignEdit->setPlaceholderText(tr("Personal signature"));
    personSignEdit->setText(Global::preferences->personSign);

    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->addWidget(nickNameEdit);
    vbox->addWidget(personSignEdit);

    nicknameGroupBox->setLayout(vbox);
}

void SetupWindow::createGroupNameGroupBox()
{
    groupNameGroupBox = new QGroupBox(tr("Group"));
    groupNameGroupBox->setAlignment(Qt::AlignHCenter);

    groupNameComboBox = new QComboBox();
    groupNameComboBox->setEditable(true);
    groupNameComboBox->setDuplicatesEnabled(false);
    QCompleter *completer
        = new QCompleter(Global::preferences->groupNameList, this);
    groupNameComboBox->setCompleter(completer);

    // groupNameComboBox->addItem("");
    groupNameComboBox->addItems(Global::preferences->groupNameList);
    int i = groupNameComboBox->findText(Global::preferences->groupName);
    groupNameComboBox->setCurrentIndex(i);

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->addWidget(groupNameComboBox);

    groupNameGroupBox->setLayout(hbox);
}

void SetupWindow::createSendRecvSettingsGroupBox()
{
    sendReceiveSettingsGroupBox = new QGroupBox(tr("Send/Receive Settings"));

    readCheck = new QCheckBox(tr("Envelope open check"));
    if (Global::preferences->isReadCheck) {
        readCheck->setCheckState(Qt::Checked);
    }
    noAutoPopupMsg = new QCheckBox(tr("Not popup message"));
    if (Global::preferences->isNoAutoPopupMsg) {
        noAutoPopupMsg->setCheckState(Qt::Checked);
    }
    noSoundAlarm = new QCheckBox(tr("No sound alarm"));
    if (Global::preferences->isNoSoundAlarm) {
        noSoundAlarm->setCheckState(Qt::Checked);
    }
    presenceSoundCheck = new QCheckBox(tr("Sound when peers come online or go offline"));
    presenceSoundCheck->setToolTip(tr(
            "Off by default. After startup, the first few seconds stay quiet. "
            "The same peer will not beep again for 20 seconds, and bursts share one beep."));
    if (Global::preferences->isPresenceSound) {
        presenceSoundCheck->setCheckState(Qt::Checked);
    }
    doNotDisturbCheck = new QCheckBox(tr("Do not disturb (don't pop message windows)"));
    doNotDisturbCheck->setToolTip(tr(
            "Independent of Away / auto-reply. Hides tray balloons and does not "
            "raise chat windows. History and the unread count still update."));
    if (Global::preferences->isDoNotDisturb) {
        doNotDisturbCheck->setCheckState(Qt::Checked);
    }
    sealedSend = new QCheckBox(tr("Send sealed message by default"));
    if (Global::preferences->isSealed) {
        sealedSend->setCheckState(Qt::Checked);
    }
    quoteMsg = new QCheckBox(tr("Quote message by default"));
    if (Global::preferences->isQuoteMsg) {
        quoteMsg->setCheckState(Qt::Checked);
    }
    offlineQueueCheck = new QCheckBox(tr("Queue messages when peer is offline"));
    if (Global::preferences->isOfflineQueue) {
        offlineQueueCheck->setCheckState(Qt::Checked);
    }

    awayCheck = new QCheckBox(tr("Away"));
    if (Global::preferences->isAway) {
        awayCheck->setCheckState(Qt::Checked);
    }
    stealthCheck = new QCheckBox(tr("Stealth / Hide mode (do not answer discovery)"));
    if (Global::preferences->isStealth) {
        stealthCheck->setCheckState(Qt::Checked);
    }
    autoReplyCheck = new QCheckBox(tr("Enable auto-reply"));
    if (Global::preferences->isAutoReply) {
        autoReplyCheck->setCheckState(Qt::Checked);
    }
    autoReplyEdit = new QLineEdit;
    autoReplyEdit->setPlaceholderText(tr("I am away from my computer."));
    autoReplyEdit->setText(Global::preferences->autoReplyText);
    autoReplyCooldownSpin = new QSpinBox;
    autoReplyCooldownSpin->setRange(5, 3600);
    autoReplyCooldownSpin->setSuffix(tr(" seconds"));
    autoReplyCooldownSpin->setValue(Global::preferences->autoReplyCooldownSecs);

    QHBoxLayout *replyLayout = new QHBoxLayout;
    replyLayout->addWidget(new QLabel(tr("Reply text:")));
    replyLayout->addWidget(autoReplyEdit);
    QHBoxLayout *cooldownLayout = new QHBoxLayout;
    cooldownLayout->addWidget(new QLabel(tr("Reply cooldown:")));
    cooldownLayout->addWidget(autoReplyCooldownSpin);

    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->addWidget(readCheck);
    vbox->addWidget(noAutoPopupMsg);
    vbox->addWidget(doNotDisturbCheck);
    vbox->addWidget(noSoundAlarm);
    vbox->addWidget(presenceSoundCheck);
    vbox->addWidget(sealedSend);
    vbox->addWidget(quoteMsg);
    vbox->addWidget(offlineQueueCheck);
    vbox->addWidget(awayCheck);
    vbox->addWidget(stealthCheck);
    vbox->addWidget(autoReplyCheck);
    vbox->addLayout(replyLayout);
    vbox->addLayout(cooldownLayout);

    sendReceiveSettingsGroupBox->setLayout(vbox);
}

void SetupWindow::createBroadcastGroupBox()
{
    broadcastGroupBox = new QGroupBox(tr("None Local Broadcast Address Settings"));

    broadcast_list_widget_ = new QListWidget;
    broadcast_list_widget_->setSelectionMode(
            QAbstractItemView::ExtendedSelection);
    foreach(QString s, Global::preferences->userSpecifiedBroadcastIpList) {
        QListWidgetItem* item = new QListWidgetItem(s, broadcast_list_widget_);
        broadcast_list_widget_->insertItem(0, item);
    }

    QLabel* label = new QLabel(tr("Add Broadcast Address"));
    line_edit_ = new QLineEdit;
    line_edit_->setPlaceholderText(tr("Broadcast IP, start-end, or CIDR"));
    QCheckBox* dial_up = new QCheckBox(tr("dial up connection"));
    dial_up->setEnabled(false);

    QPushButton* add_button = new QPushButton(">>");
    QPushButton* del_button = new QPushButton("<<");
    QFontMetrics fm = fontMetrics();
    add_button->setFixedSize(Helper::fontTextWidth(fm, ">>") + 6,
                             Helper::fontTextWidth(fm, ">>") + 2);
    del_button->setFixedSize(Helper::fontTextWidth(fm, ">>") + 6,
                             Helper::fontTextWidth(fm, ">>") + 2);
    connect(add_button, SIGNAL(clicked()), this, SLOT(AddBroadcast()));
    connect(del_button, SIGNAL(clicked()), this, SLOT(DelBroadcast()));
    QVBoxLayout* button_layout = new QVBoxLayout;
    button_layout->addSpacing(20);
    button_layout->addWidget(add_button);
    button_layout->addWidget(del_button);
    button_layout->addSpacing(20);

    QGridLayout *grid_layout = new QGridLayout();
    grid_layout->addWidget(label, 0, 0, 1, 1);
    grid_layout->addWidget(line_edit_, 1, 0, 1, 1);
    grid_layout->addWidget(dial_up, 2, 0, 1, 1);
    grid_layout->addLayout(button_layout, 0, 1, 3, 1);
    grid_layout->addWidget(broadcast_list_widget_, 0, 2, 3, 1);

    QLabel *rangeHint = new QLabel(tr(
            "Cross-subnet peers: add a start-end range or CIDR. "
            "Discovery unicasts BR_ENTRY to each address (no server). "
            "One entry is limited to %1 hosts and is saved with the broadcast list.")
            .arg(ProtocolUtil::maxIpv4RangeHosts()));
    rangeHint->setWordWrap(true);

    rangeStartEdit = new QLineEdit;
    rangeStartEdit->setPlaceholderText(tr("Start IP"));
    rangeEndEdit = new QLineEdit;
    rangeEndEdit->setPlaceholderText(tr("End IP"));
    cidrEdit = new QLineEdit;
    cidrEdit->setPlaceholderText(tr("CIDR, e.g. 192.168.2.0/24"));
    QPushButton *addRangeButton = new QPushButton(tr("Add range"));
    connect(addRangeButton, SIGNAL(clicked()), this, SLOT(AddIpRange()));

    QHBoxLayout *rangeLayout = new QHBoxLayout;
    rangeLayout->addWidget(rangeStartEdit);
    rangeLayout->addWidget(rangeEndEdit);
    rangeLayout->addWidget(cidrEdit, 1);
    rangeLayout->addWidget(addRangeButton);

    QVBoxLayout *boxLayout = new QVBoxLayout;
    boxLayout->addLayout(grid_layout);
    boxLayout->addWidget(rangeHint);
    boxLayout->addLayout(rangeLayout);
    broadcastGroupBox->setLayout(boxLayout);
}

void SetupWindow::createButtonLayout()
{
    okButton = new QPushButton(tr("Ok"));
    applyButton = new QPushButton(tr("Apply"));

    buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(okButton);
    buttonsLayout->addWidget(applyButton);
}

void SetupWindow::createMiscSettingGroupBox()
{
    miscSettingGroupBox = new QGroupBox(tr("Other Settings"));

    int frameStyle = QFrame::Sunken | QFrame::Panel;
    QLabel *label = new QLabel(tr("Click right button on send\nwindow to set display level"));
    label->setFrameStyle(frameStyle);

    detailSetupButton = new QPushButton(tr("Detail/Log settings"));
    urlLinkSetupButton = new QPushButton(tr("URL link settings"));
    urlLinkSetupButton->setDisabled(true);
    passwordSetupButton = new QPushButton(tr("Password settings"));
    passwordSetupButton->setDisabled(true);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(label);
    vbox->addWidget(detailSetupButton);
    vbox->addWidget(urlLinkSetupButton);
    vbox->addWidget(passwordSetupButton);

    miscSettingGroupBox->setLayout(vbox);
}

void SetupWindow::createConnections()
{
    connect(okButton, SIGNAL(clicked()),
            this, SLOT(savePreferences()));
    connect(applyButton, SIGNAL(clicked()),
            this, SLOT(applyPreferences()));

    connect(detailSetupButton, SIGNAL(clicked()),
            this, SLOT(detailSetup()));
}

void SetupWindow::savePreferences()
{
    applyPreferences();

    close();
}

void SetupWindow::applyPreferences()
{
    Global::preferences->userName = nickNameEdit->text();
    Global::preferences->personSign = personSignEdit->text();

    if (!groupNameComboBox->currentText().isEmpty() &&
        groupNameComboBox->findText(groupNameComboBox->currentText())
        == -1) { // Group not exist
        groupNameComboBox->insertItem(0, groupNameComboBox->currentText());

        for (int i = 0; i < groupNameComboBox->count(); ++i) {
            QString s = groupNameComboBox->itemText(i);
            if (!Global::preferences->groupNameList.contains(s)) {
                Global::preferences->groupNameList.prepend(s);
            }
        }

        QCompleter *completer
            = new QCompleter(Global::preferences->groupNameList, this);
        groupNameComboBox->setCompleter(completer);
    }
    Global::preferences->groupName = groupNameComboBox->currentText();

    Global::preferences->isReadCheck = readCheck->isChecked();
    Global::preferences->isNoSoundAlarm = noSoundAlarm->isChecked();
    Global::preferences->isPresenceSound = presenceSoundCheck->isChecked();
    Global::preferences->isDoNotDisturb = doNotDisturbCheck->isChecked();
    Global::preferences->isSealed = sealedSend->isChecked();
    Global::preferences->isQuoteMsg = quoteMsg->isChecked();
    Global::preferences->isOfflineQueue = offlineQueueCheck->isChecked();
    Global::preferences->isNoAutoPopupMsg = noAutoPopupMsg->isChecked();
    Global::preferences->isAway = awayCheck->isChecked();
    Global::preferences->isAutoReply = autoReplyCheck->isChecked();
    Global::preferences->autoReplyText = autoReplyEdit->text();
    Global::preferences->autoReplyCooldownSecs = autoReplyCooldownSpin->value();
    const bool stealth = stealthCheck->isChecked();
    const bool stealthChanged = (stealth != Global::preferences->isStealth);

    Global::userManager->updateOurself();
    if (stealthChanged) {
        Global::userManager->applyStealth(stealth);
    } else if (!stealth) {
        Global::userManager->announcePresence();
    }

    Global::preferences->save();
    if (Global::systray) {
        Global::systray->syncActions();
    }
}

void SetupWindow::detailSetup()
{
    DetailSetupDialog *dialog = new DetailSetupDialog(this);
    dialog->show();
}

QString SetupWindow::tokenStatusText(int status) const
{
    switch (status) {
    case ProtocolUtil::Ipv4TokenEmpty:
        return tr("Enter an IPv4 address, a start-end range, or a CIDR prefix.");
    case ProtocolUtil::Ipv4TokenBadPrefix:
        return tr("CIDR prefix must be between 0 and 32.");
    case ProtocolUtil::Ipv4TokenTooLarge:
        return tr("That range has more than %1 addresses. Split it into smaller ranges.")
                .arg(ProtocolUtil::maxIpv4RangeHosts());
    case ProtocolUtil::Ipv4TokenUnusable:
        return tr("Loopback and unspecified addresses are not probed.");
    default:
        return tr("That is not a valid IPv4 address or range.");
    }
}

bool SetupWindow::appendBroadcastToken(const QString &raw)
{
    QString canonical;
    const int status = ProtocolUtil::inspectIpv4Token(raw, &canonical, 0, 0);
    if (status != ProtocolUtil::Ipv4TokenOk) {
        QMessageBox::warning(this, tr("Broadcast address"), tokenStatusText(status));
        return false;
    }
    if (Global::preferences->userSpecifiedBroadcastIpList.contains(canonical)) {
        return false;
    }
    Global::preferences->userSpecifiedBroadcastIpList.prepend(canonical);
    QListWidgetItem *item = new QListWidgetItem(canonical, broadcast_list_widget_);
    broadcast_list_widget_->insertItem(0, item);
    return true;
}

void SetupWindow::AddBroadcast()
{
    if (appendBroadcastToken(line_edit_->text())) {
        line_edit_->clear();
    }
}

void SetupWindow::AddIpRange()
{
    const QString cidr = cidrEdit->text().trimmed();
    const QString start = rangeStartEdit->text().trimmed();
    const QString end = rangeEndEdit->text().trimmed();
    QString raw;
    if (!cidr.isEmpty()) {
        raw = cidr;
    } else if (!start.isEmpty() || !end.isEmpty()) {
        if (start.isEmpty() || end.isEmpty()) {
            QMessageBox::warning(this, tr("Broadcast address"),
                    tr("Enter both a start and an end address, or a CIDR prefix."));
            return;
        }
        raw = start + QLatin1Char('-') + end;
    } else {
        QMessageBox::warning(this, tr("Broadcast address"),
                tokenStatusText(ProtocolUtil::Ipv4TokenEmpty));
        return;
    }
    if (appendBroadcastToken(raw)) {
        rangeStartEdit->clear();
        rangeEndEdit->clear();
        cidrEdit->clear();
    }
}

void SetupWindow::DelBroadcast()
{
    foreach(QListWidgetItem* item, broadcast_list_widget_->selectedItems()) {
        int row = broadcast_list_widget_->row(item);
        broadcast_list_widget_->takeItem(row);
        line_edit_->setText(item->text());
        Global::preferences->userSpecifiedBroadcastIpList.removeOne(
                item->text());
    }
}

QSize SetupWindow::sizeHint() const
{
    return QSize(360, 240);
}

