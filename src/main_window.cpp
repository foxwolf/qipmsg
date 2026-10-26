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

#include <QtGui>
#include <QtCore>
#include <QtWidgets>
#include <QMessageBox>
#include <algorithm>
#include <QHostAddress>
#include <QInputDialog>
#include <QStringList>
#include <QSet>

#include "main_window.h"
#include "msgCrypto.h"
#include "helper.h"
#include "user_manager.h"
#include "msg_thread.h"
#include "send_msg.h"
#include "recv_msg.h"
#include "constants.h"
#include "ipmsg.h"
#include "send_file_window.h"
#include "systray.h"
#include "global.h"
#include "preferences.h"
#include "dir_dialog.h"
#include "send_file_manager.h"
#include "window_manager.h"
#include "history_search_window.h"
#include "contactDelegate.h"
#include "discussion_store.h"
#include "pinyin.h"
#include "groupPolicy.h"
#include "groupPolicyDialog.h"
#include "recentStore.h"
#include "avatarStore.h"
#include "avatarFetch.h"
#include "scheduleDialog.h"
#include "winShare.h"

quint32 MainWindow::m_levelOneCount = 0;
quint32 MainWindow::m_levelTwoCount = 0;

MainWindow::MainWindow(QString text, QString ip,
        QWidget *parent)
    : QMainWindow(parent), m_groupSendHint(0), m_scheduleButton(0), sendToGroupAct(0),
    setMyPhotoAct(0), setPeerPhotoAct(0), clearPeerPhotoAct(0), openShareAct(0),
    m_initText(text), m_selectedIp(ip), m_lastSearchDialog(0)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    setAcceptDrops(true);

    createSendLayout();
    createPeerWidget();
    createInputWidget();
    createSplitterWidget();
    createActions();

    setCentralWidget(splitter);
    createConnections();

    inputEdit->setFocus(Qt::TabFocusReason);
    inputEdit->moveCursor(QTextCursor::End);

    setWindowTitle(tr("IP Messenger"));
    setWindowIcon(*Global::iconSet.value("normal"));

    rebuildContactTree();
    selectUser();

    adjustSize();
    move(Global::randomNearMiddlePoint());
}

MainWindow::~MainWindow()
{
}

QSize MainWindow::sizeHint() const
{
    return QSize(360, 640);
}

void MainWindow::createConnections()
{
    connect(sendButton, SIGNAL(clicked()),
            this, SLOT(sendMessage()));
    connect(broadcastButton, SIGNAL(clicked()),
            this, SLOT(broadcastToOnline()));

    connect(Global::userManager, SIGNAL(userCountUpdated(int)),
            this, SLOT(updateUserCount(int)));
    connect(Global::userManager, SIGNAL(usersChanged()),
            this, SLOT(rebuildContactTree()));
    if (Global::discussionStore) {
        connect(Global::discussionStore, SIGNAL(groupsChanged()),
                this, SLOT(rebuildContactTree()));
    }
    if (Global::groupPolicy) {
        connect(Global::groupPolicy, SIGNAL(policyChanged()),
                this, SLOT(rebuildContactTree()));
    }
    if (Global::recentStore) {
        connect(Global::recentStore, SIGNAL(contactsChanged()),
                this, SLOT(rebuildContactTree()));
    }
    if (Global::avatarStore) {
        connect(Global::avatarStore, SIGNAL(changed()),
                this, SLOT(rebuildContactTree()));
        connect(Global::avatarStore, SIGNAL(changed()),
                this, SLOT(updateHeader()));
    }
    if (Global::windowManager) {
        connect(Global::windowManager, SIGNAL(peerTypingChanged(QString,bool)),
                this, SLOT(onPeerTyping(QString,bool)));
    }
    if (Global::offlineQueue) {
        connect(Global::offlineQueue, SIGNAL(queueChanged()),
                this, SLOT(updateHeader()));
    }
    connect(refreshButton, SIGNAL(clicked()),
            this, SLOT(refreshUserList()));
    connect(m_scheduleButton, SIGNAL(clicked()),
            this, SLOT(openSchedule()));

    connect(sendFileAct, SIGNAL(triggered()),
            this, SLOT(addSendFile()));
    connect(sendDirAct, SIGNAL(triggered()),
            this, SLOT(addSendDir()));
    connect(searchUserAct, SIGNAL(triggered()),
            this, SLOT(showSearchDialog()));
    connect(blacklistAct, SIGNAL(triggered()),
            this, SLOT(toggleBlacklist()));
    connect(createDiscussionAct, SIGNAL(triggered()),
            this, SLOT(createDiscussion()));
    connect(renameDiscussionAct, SIGNAL(triggered()),
            this, SLOT(renameDiscussion()));
    connect(deleteDiscussionAct, SIGNAL(triggered()),
            this, SLOT(deleteDiscussion()));
    connect(addToDiscussionAct, SIGNAL(triggered()),
            this, SLOT(addSelectedToDiscussion()));

    connect(new QShortcut(QKeySequence("Ctrl+F"), this), SIGNAL(activated()),
            searchEdit, SLOT(setFocus()));
    connect(new QShortcut(QKeySequence("Ctrl+O"), this), SIGNAL(activated()),
            this, SLOT(openSelectedChat()));

    connect(contactTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(onContactDoubleClicked(QTreeWidgetItem*,int)));
    connect(contactTree, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(onRecentClicked(QTreeWidgetItem*,int)));
    connect(searchEdit, SIGNAL(textChanged(QString)),
            this, SLOT(applyContactFilter(QString)));
    connect(signatureEdit, SIGNAL(editingFinished()),
            this, SLOT(saveSignature()));
}

void MainWindow::createPeerWidget()
{
    peerWidget = new QWidget;
    peerWidget->setObjectName("contactPane");

    avatarLabel = new QLabel;
    avatarLabel->setFixedSize(40, 40);
    avatarLabel->setCursor(Qt::PointingHandCursor);
    avatarLabel->setToolTip(tr(
            "Click to choose a PNG or JPEG. Other qipmsg peers can fetch a "
            "small copy. FeiQ has no photo packet, so those peers still see "
            "a color block."));
    avatarLabel->installEventFilter(this);

    nickStatusLabel = new QLabel;
    nickStatusLabel->setObjectName("headerName");

    signatureEdit = new QLineEdit;
    signatureEdit->setObjectName("signatureEdit");
    signatureEdit->setPlaceholderText(tr("Enter a personal signature"));
    signatureEdit->setText(Global::preferences->personSign);

    onlineCountLabel = new QLabel;
    onlineCountLabel->setObjectName("headerMeta");
    onlineCountLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QVBoxLayout *headText = new QVBoxLayout;
    headText->setContentsMargins(0, 0, 0, 0);
    headText->setSpacing(2);
    headText->addWidget(nickStatusLabel);
    headText->addWidget(onlineCountLabel);
    headText->addWidget(signatureEdit);

    QHBoxLayout *headRow = new QHBoxLayout;
    headRow->setContentsMargins(0, 0, 0, 0);
    headRow->setSpacing(12);
    headRow->addWidget(avatarLabel, 0, Qt::AlignTop);
    headRow->addLayout(headText, 1);

    QWidget *header = new QWidget;
    header->setObjectName("appHeader");
    QHBoxLayout *headerPad = new QHBoxLayout(header);
    headerPad->setContentsMargins(14, 12, 14, 12);
    headerPad->addLayout(headRow);

    searchEdit = new QLineEdit;
    searchEdit->setObjectName("contactSearch");
    searchEdit->setPlaceholderText(tr("Search friends / pinyin..."));
    searchEdit->setClearButtonEnabled(true);

    refreshButton = new QPushButton(tr("&Refresh"));
    refreshButton->setToolTip(tr("Refresh contacts"));
    m_scheduleButton = new QPushButton(tr("Schedule"));
    m_scheduleButton->setToolTip(tr(
            "Local reminders. They fire only while QIpMsg is running."));

    QWidget *searchBar = new QWidget;
    searchBar->setObjectName("searchBar");
    QHBoxLayout *searchRow = new QHBoxLayout(searchBar);
    searchRow->setContentsMargins(12, 8, 12, 8);
    searchRow->addWidget(searchEdit, 1);
    searchRow->addWidget(m_scheduleButton);
    searchRow->addWidget(refreshButton);

    contactTree = new QTreeWidget;
    contactTree->setObjectName("contactTree");
    contactTree->setHeaderHidden(true);
    contactTree->setRootIsDecorated(true);
    contactTree->setIndentation(14);
    contactTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    contactTree->setEditTriggers(QAbstractItemView::NoEditTriggers);
    contactTree->setUniformRowHeights(false);
    contactTree->setIconSize(QSize(36, 36));
    contactTree->setAnimated(true);
    contactTree->setItemDelegate(new ContactDelegate(contactTree));
    contactTree->setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(header);
    layout->addWidget(searchBar);
    layout->addWidget(contactTree, 1);
    peerWidget->setLayout(layout);

    updateHeader();
}

void MainWindow::createInputWidget()
{
    inputWidget = new QWidget;

    fileListButton = new QPushButton;
    connect(fileListButton, SIGNAL(clicked()),
            this, SLOT(showSendFileList()));

    inputEdit = new QTextEdit;
    inputEdit->setAcceptDrops(false);
    inputEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    inputEdit->setAcceptRichText(false);
    inputEdit->setText(m_initText);
    inputEdit->setTabChangesFocus(true);
    inputEdit->setMaximumHeight(90);

    m_groupSendHint = new QLabel;
    m_groupSendHint->setWordWrap(true);
    m_groupSendHint->hide();

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(fileListButton);
    vbox->addWidget(m_groupSendHint);
    vbox->addWidget(inputEdit);
    vbox->addLayout(sendLayout);

    fileListButton->hide();
    inputWidget->setLayout(vbox);
}

void MainWindow::createSplitterWidget()
{
    splitter = new QSplitter(Qt::Vertical);
    splitter->setChildrenCollapsible(false);
    splitter->addWidget(peerWidget);
    splitter->addWidget(inputWidget);

    QList<int> l;
    l << 420 << 140;
    splitter->setSizes(l);
}

void MainWindow::createSendLayout()
{
    sendButton = new QPushButton(tr("&Send"));
    sendButton->setObjectName("primaryButton");
    broadcastButton = new QPushButton(tr("Broadcast"));
    broadcastButton->setToolTip(tr("Send to every online peer with the broadcast flag"));
    encapCheckBox = new QCheckBox(tr("&Encap"));
    if (Global::preferences->isSealed) {
        encapCheckBox->setCheckState(Qt::Checked);
    } else {
        encapCheckBox->setCheckState(Qt::Unchecked);
    }
    lockCheckBox = new QCheckBox(tr("Loc&k"));
    lockCheckBox->setEnabled(false);

    sendLayout = new QHBoxLayout;
    sendLayout->addStretch();
    sendLayout->addWidget(broadcastButton);
    sendLayout->addWidget(sendButton);
    sendLayout->addSpacing(20);
    sendLayout->addWidget(encapCheckBox);
    sendLayout->addSpacing(20);
    sendLayout->addWidget(lockCheckBox);
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(openChatAct);
    menu.addAction(broadcastAllAct);
    menu.addAction(sendToGroupAct);
    menu.addMenu(selectGroupMenu);
    menu.addAction(sendFileAct);
    menu.addAction(sendDirAct);
    menu.addAction(openShareAct);
    menu.addAction(searchUserAct);
    menu.addAction(findHistoryAct);
    menu.addSeparator();
    menu.addAction(createDiscussionAct);
    menu.addAction(renameDiscussionAct);
    menu.addAction(deleteDiscussionAct);
    menu.addAction(addToDiscussionAct);
    menu.addAction(setMemoAct);
    menu.addAction(setMyPhotoAct);
    menu.addAction(setPeerPhotoAct);
    menu.addAction(clearPeerPhotoAct);
    menu.addSeparator();
    menu.addAction(hideFromGroupAct);
    menu.addAction(muteGroupAct);
    menu.addAction(groupPolicyAct);
    menu.addSeparator();
    menu.addAction(blacklistAct);

    const QStringList ips = selectedPeerIps();
    const QStringList discIds = selectedDiscussionIds();
    createDiscussionAct->setEnabled(ips.size() >= 2);
    renameDiscussionAct->setEnabled(discIds.size() == 1);
    deleteDiscussionAct->setEnabled(!discIds.isEmpty());
    addToDiscussionAct->setEnabled(discIds.size() == 1 && !ips.isEmpty());
    if (ips.size() == 1 && isBlacklisted(ips.first())) {
        blacklistAct->setText(tr("Remove from blacklist"));
    } else {
        blacklistAct->setText(tr("Add to blacklist"));
    }
    blacklistAct->setEnabled(!ips.isEmpty());
    setMemoAct->setEnabled(ips.size() == 1);
    updateShareAction(ips);
    setPeerPhotoAct->setEnabled(ips.size() == 1);
    clearPeerPhotoAct->setEnabled(ips.size() == 1);

    const QString groupName = groupAtCursor();
    const bool hasGroup = !groupName.isEmpty() && Global::groupPolicy;
    hideFromGroupAct->setVisible(hasGroup);
    muteGroupAct->setVisible(hasGroup);
    if (hasGroup) {
        hideFromGroupAct->setText(tr("Hide from group \"%1\"").arg(groupName));
        muteGroupAct->setText(tr("Mute messages from group \"%1\"").arg(groupName));
        hideFromGroupAct->setData(groupName);
        muteGroupAct->setData(groupName);
        hideFromGroupAct->blockSignals(true);
        muteGroupAct->blockSignals(true);
        hideFromGroupAct->setChecked(Global::groupPolicy->isHidden(groupName));
        muteGroupAct->setChecked(Global::groupPolicy->isMuted(groupName));
        hideFromGroupAct->blockSignals(false);
        muteGroupAct->blockSignals(false);
    }
    sendToGroupAct->setVisible(hasGroup);
    if (hasGroup && Global::userManager) {
        const int count = reachableSendCount(groupName);
        sendToGroupAct->setText(tr("Send to \"%1\" (%n online/away)", "", count)
                                .arg(groupName));
        sendToGroupAct->setData(groupName);
    }

    updateSelectGroupMenu();
    menu.exec(event->globalPos());
}

void MainWindow::createActions()
{
    openChatAct = new QAction(tr("Open chat"), this);
    connect(openChatAct, SIGNAL(triggered()),
            this, SLOT(openSelectedChat()));
    broadcastAllAct = new QAction(tr("Broadcast to all online"), this);
    connect(broadcastAllAct, SIGNAL(triggered()),
            this, SLOT(broadcastToOnline()));
    createDiscussionAct = new QAction(tr("Create discussion group"), this);
    renameDiscussionAct = new QAction(tr("Rename discussion group"), this);
    deleteDiscussionAct = new QAction(tr("Delete discussion group"), this);
    addToDiscussionAct = new QAction(tr("Add selected contacts to discussion"), this);
    sendFileAct = new QAction(tr("Send file"), this);
    sendDirAct = new QAction(tr("Send folder"), this);
    openShareAct = new QAction(tr("Open network neighborhood"), this);
    openShareAct->setToolTip(tr(
            "Open \\\\IP\\ in Explorer on Windows. Windows asks for a password if one is required."));
    connect(openShareAct, SIGNAL(triggered()),
            this, SLOT(openNetworkNeighborhood()));
    searchUserAct = new QAction(tr("Search user"), this);
    findHistoryAct = new QAction(tr("Find in history..."), this);
    connect(findHistoryAct, SIGNAL(triggered()),
            this, SLOT(findInHistory()));
    blacklistAct = new QAction(tr("Add to blacklist"), this);
    setMemoAct = new QAction(tr("Set memo..."), this);
    connect(setMemoAct, SIGNAL(triggered()),
            this, SLOT(editPeerMemo()));
    setMyPhotoAct = new QAction(tr("Set my photo..."), this);
    setMyPhotoAct->setToolTip(tr(
            "Photo for the contact list and header. A small PNG is offered "
            "to qipmsg peers. FeiQ is not offered a photo packet."));
    connect(setMyPhotoAct, SIGNAL(triggered()),
            this, SLOT(setMyPhoto()));
    setPeerPhotoAct = new QAction(tr("Set local photo..."), this);
    setPeerPhotoAct->setToolTip(tr(
            "Show a PNG or JPEG for this IP on this computer. It overrides "
            "a photo fetched from qipmsg. FeiQ still has no photo packet."));
    connect(setPeerPhotoAct, SIGNAL(triggered()),
            this, SLOT(setPeerPhoto()));
    clearPeerPhotoAct = new QAction(tr("Clear local photo"), this);
    connect(clearPeerPhotoAct, SIGNAL(triggered()),
            this, SLOT(clearPeerPhoto()));
    hideFromGroupAct = new QAction(tr("Hide from this group"), this);
    hideFromGroupAct->setCheckable(true);
    connect(hideFromGroupAct, SIGNAL(triggered()),
            this, SLOT(toggleHideFromGroup()));
    muteGroupAct = new QAction(tr("Mute messages from this group"), this);
    muteGroupAct->setCheckable(true);
    connect(muteGroupAct, SIGNAL(triggered()),
            this, SLOT(toggleMuteGroup()));
    groupPolicyAct = new QAction(tr("Group permissions..."), this);
    connect(groupPolicyAct, SIGNAL(triggered()),
            this, SLOT(openGroupPolicyDialog()));
    sendToGroupAct = new QAction(tr("Send to this group (online and away)"), this);
    sendToGroupAct->setToolTip(tr(
            "Multicast the current text to online and away members of this "
            "department. Offline peers are not queued."));
    connect(sendToGroupAct, SIGNAL(triggered()),
            this, SLOT(sendToDepartment()));

    setLevelOneAct = new QAction(tr("Set level 1 (user %1 / %2)")
            .arg(m_levelOneCount)
            .arg(Global::userManager->userCount()),
            this);
    setLevelTwoAct = new QAction(tr("Set level 2 (user %1 / %2)")
            .arg(m_levelTwoCount)
            .arg(Global::userManager->userCount()),
            this);
    setDisplayLevelMenu = new QMenu(tr("Set display level"), this);
    setDisplayLevelMenu->addAction(setLevelOneAct);
    setDisplayLevelMenu->addAction(setLevelTwoAct);

    selectGroupMenu = new QMenu(tr("Select group"), this);
    updateSelectGroupMenu();
}

void MainWindow::updateShareAction(const QStringList &ips)
{
    if (!openShareAct) {
        return;
    }
    if (!WinShare::platformSupports()) {
        openShareAct->setText(tr("Open network neighborhood (Windows only)"));
        openShareAct->setEnabled(false);
        openShareAct->setToolTip(tr(
                "Windows network neighborhood is only available on Windows."));
        return;
    }
    openShareAct->setText(tr("Open network neighborhood"));
    const bool one = ips.size() == 1 && WinShare::isSafeIpv4(ips.first(), 0);
    openShareAct->setEnabled(one);
    if (one) {
        openShareAct->setToolTip(tr(
                "Open \\\\%1\\ in Explorer. Windows asks for a password if one is required. This program does not ask for share credentials.")
                                 .arg(ips.first()));
    } else {
        openShareAct->setToolTip(tr("Select one peer with an IPv4 address."));
    }
}

void MainWindow::openNetworkNeighborhood()
{
    const QStringList ips = selectedPeerIps();
    if (ips.size() != 1) {
        return;
    }
    const WinShareOpen opened = WinShare::openNeighborhood(ips.first());
    if (opened.result == winShareOk) {
        return;
    }
    QMessageBox::warning(this, tr("Network neighborhood"), WinShare::describe(opened));
}

void MainWindow::updateSelectGroupMenu()
{
    selectGroupMenu->clear();
    const QStringList names = Global::userManager
            ? Global::userManager->departmentNames() : QStringList();
    if (names.isEmpty()) {
        QAction *empty = selectGroupMenu->addAction(tr("No department groups"));
        empty->setEnabled(false);
        return;
    }
    foreach (const QString &group, names) {
        const int count = reachableSendCount(group);
        QAction *action = new QAction(
                tr("%1 (%n online/away)", "", count).arg(group), selectGroupMenu);
        action->setData(group);
        action->setToolTip(tr(
                "Select online and away members. Grey offline peers stay out."));
        connect(action, SIGNAL(triggered()), this, SLOT(selectGroup()));
        selectGroupMenu->addAction(action);
    }
}

QString MainWindow::groupAtCursor() const
{
    if (!contactTree) {
        return QString();
    }
    const QPoint pos = contactTree->viewport()->mapFromGlobal(QCursor::pos());
    QTreeWidgetItem *item = contactTree->itemAt(pos);
    if (!item) {
        return QString();
    }
    const QString key = item->data(0, Qt::UserRole + 1).toString();
    const QString prefix = QStringLiteral("dept:");
    if (key.startsWith(prefix)) {
        return GroupPolicy::normalizeName(key.mid(prefix.size()));
    }
    if (!item->data(0, Qt::UserRole).toString().isEmpty()) {
        return GroupPolicy::normalizeName(item->data(0, Qt::UserRole + 3).toString());
    }
    return QString();
}

void MainWindow::toggleHideFromGroup()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action || !Global::groupPolicy || !Global::userManager) {
        return;
    }
    const QString groupName = action->data().toString();
    if (Global::groupPolicy->setHidden(groupName, action->isChecked())) {
        Global::userManager->refreshGroupVisibility(groupName);
    }
}

void MainWindow::toggleMuteGroup()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action || !Global::groupPolicy) {
        return;
    }
    Global::groupPolicy->setMuted(action->data().toString(), action->isChecked());
}

void MainWindow::openGroupPolicyDialog()
{
    GroupPolicyDialog dialog(this);
    dialog.exec();
}

void MainWindow::selectGroup()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action) {
        return;
    }
    const QString group = action->data().toString();
    const int count = selectReachableInGroup(group);
    if (count <= 0) {
        m_groupSendHint->setText(tr("No online or away members in \"%1\".")
                                 .arg(group));
        m_groupSendHint->show();
        return;
    }
    showGroupSendHint(group, count);
}

int MainWindow::reachableSendCount(const QString &groupName) const
{
    if (!Global::userManager) {
        return 0;
    }
    int count = 0;
    foreach (const QString &ip, Global::userManager->reachableIpsInGroup(groupName)) {
        if (!isBlacklisted(ip)) {
            ++count;
        }
    }
    return count;
}

int MainWindow::selectReachableInGroup(const QString &groupName)
{
    const QString key = GroupPolicy::normalizeName(groupName);
    contactTree->clearSelection();
    int count = 0;
    QStringList seen;
    foreach (QTreeWidgetItem *item, allPeerItems()) {
        if (item->data(0, Qt::UserRole + 8).toString() == QLatin1String("recent")) {
            continue;
        }
        const QString ip = item->data(0, Qt::UserRole).toString();
        if (ip.isEmpty() || seen.contains(ip)) {
            continue;
        }
        if (GroupPolicy::normalizeName(item->data(0, Qt::UserRole + 3).toString())
                != key) {
            continue;
        }
        if (isBlacklisted(ip) || !Global::userManager->isReachable(ip)) {
            continue;
        }
        QTreeWidgetItem *parent = item->parent();
        while (parent) {
            parent->setExpanded(true);
            parent = parent->parent();
        }
        item->setSelected(true);
        contactTree->scrollToItem(item);
        seen << ip;
        ++count;
    }
    return count;
}

void MainWindow::showGroupSendHint(const QString &groupName, int count)
{
    m_groupSendHint->setText(tr(
            "Group send: \"%1\" · %n online or away member(s). "
            "Offline peers are not included. Press Send.",
            "", count).arg(groupName));
    m_groupSendHint->show();
    inputEdit->setFocus();
}

void MainWindow::sendToDepartment()
{
    QAction *action = qobject_cast<QAction *>(sender());
    const QString group = action ? action->data().toString() : groupAtCursor();
    if (group.isEmpty() || !Global::userManager) {
        return;
    }
    const int count = selectReachableInGroup(group);
    if (count <= 0) {
        QMessageBox::information(this, tr("Group send"),
                tr("No online or away members in \"%1\". "
                   "Offline peers are not queued.").arg(group));
        m_groupSendHint->setText(tr("No online or away members in \"%1\".")
                                 .arg(group));
        m_groupSendHint->show();
        return;
    }
    showGroupSendHint(group, count);
    const bool hasBody = !inputEdit->toPlainText().trimmed().isEmpty()
            || hasSendFile();
    if (!hasBody) {
        return;
    }
    if (QMessageBox::question(this, tr("Group send"),
            tr("Send the current message to %n member(s) of \"%1\"?",
               "", count).arg(group),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
            != QMessageBox::Yes) {
        return;
    }
    sendMessage();
}

void MainWindow::onPeerTyping(QString ip, bool typing)
{
    if (typing) {
        m_typingIps.insert(ip);
    } else {
        m_typingIps.remove(ip);
    }
    foreach (QTreeWidgetItem *item, allPeerItems()) {
        if (item->data(0, Qt::UserRole).toString() == ip) {
            item->setData(0, Qt::UserRole + 9, typing);
        }
    }
    if (contactTree && contactTree->viewport()) {
        contactTree->viewport()->update();
    }
}

void MainWindow::onRecentClicked(QTreeWidgetItem *item, int)
{
    if (!item || !Global::windowManager) {
        return;
    }
    if (item->data(0, Qt::UserRole + 8).toString() != QLatin1String("recent")) {
        return;
    }
    const QString ip = item->data(0, Qt::UserRole).toString();
    if (!ip.isEmpty()) {
        Global::windowManager->openChat(ip, true);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Global::systray->mainWindowList.removeAll(this);
    event->accept();
}

void MainWindow::updateUserCount(int i)
{
    setLevelOneAct->setText(tr("set level 1 (user %1 / %2)")
            .arg(m_levelOneCount).arg(i));
    setLevelTwoAct->setText(tr("set level 2 (user %1 / %2)")
            .arg(m_levelTwoCount).arg(i));
    updateHeader();
}

void MainWindow::updateHeader()
{
    const Owner me = Global::userManager->ourself();
    QString status = tr("Online");
    int presence = PRESENCE_ONLINE;
    if (Global::preferences->isStealth) {
        status = tr("Stealth");
        presence = PRESENCE_STALE;
    } else if (Global::preferences->isAway) {
        status = tr("Away");
        presence = PRESENCE_AWAY;
    }
    if (!Global::preferences->isStealth && Global::groupPolicy
            && Global::groupPolicy->hasHidden()) {
        status = tr("%1, hidden from some groups").arg(status);
    }
    nickStatusLabel->setText(me.name());
    QString presenceLine = tr("%1 · %n online", "",
                              Global::userManager->onlineCount()).arg(status);
    if (Global::offlineQueue && Global::offlineQueue->count() > 0) {
        presenceLine += tr(" · %n offline queued", "",
                           Global::offlineQueue->count());
    }
    onlineCountLabel->setText(presenceLine);
    avatarLabel->setPixmap(peerAvatar(me.name(), presence, 40,
                                      AvatarStore::selfKey()));
}

QPixmap MainWindow::peerAvatar(const QString &name, int presence, int size,
                               const QString &photoKey) const
{
    if (!photoKey.isEmpty() && Global::avatarStore) {
        const QPixmap photo = Global::avatarStore->rounded(
                    photoKey, size, Helper::presenceColor(presence));
        if (!photo.isNull()) {
            return photo;
        }
    }
    return Helper::avatarPixmap(name, size, Helper::avatarColor(name),
                                Helper::presenceColor(presence));
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == avatarLabel && event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouse = static_cast<QMouseEvent *>(event);
        if (mouse->button() == Qt::LeftButton) {
            setMyPhoto();
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::setMyPhoto()
{
    if (Global::avatarStore) {
        Global::avatarStore->chooseAndSet(AvatarStore::selfKey(), this);
    }
}

void MainWindow::setPeerPhoto()
{
    const QStringList ips = selectedPeerIps();
    if (ips.size() != 1 || !Global::avatarStore) {
        return;
    }
    Global::avatarStore->chooseAndSet(ips.first(), this);
}

void MainWindow::clearPeerPhoto()
{
    const QStringList ips = selectedPeerIps();
    if (ips.size() != 1) {
        return;
    }
    if (Global::avatarFetch) {
        Global::avatarFetch->clearLocal(ips.first());
        return;
    }
    if (Global::avatarStore) {
        Global::avatarStore->clearKey(ips.first());
    }
}

QTreeWidgetItem *MainWindow::makeGroupItem(const QString &title,
                                           const QString &key)
{
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, title);
    item->setData(0, Qt::UserRole + 1, key);
    item->setFlags(Qt::ItemIsEnabled);
    QFont font = item->font(0);
    font.setBold(true);
    item->setFont(0, font);
    return item;
}

void MainWindow::updateGroupTitle(QTreeWidgetItem *group, const QString &title)
{
    if (!group) {
        return;
    }
    group->setText(0, QString("%1 (%2)").arg(title).arg(group->childCount()));
}

void MainWindow::addPeerItem(QTreeWidgetItem *parent, int row)
{
    UserManager *um = Global::userManager;
    // `row` is the preferred address of one logical peer. The other
    // NICs stay in the model for file retry, but they are not extra
    // top-level contacts. Search and the typing flag still see them.
    QList<int> foldRows;
    foldRows << row;
    const QList<PeerFoldGroup> folds = um->contactFolds();
    for (int g = 0; g < folds.size(); ++g) {
        if (folds.at(g).indexes.contains(row)) {
            foldRows = folds.at(g).indexes;
            break;
        }
    }

    QStringList allIps;
    for (int i = 0; i < foldRows.size(); ++i) {
        const QString one = um->ip(foldRows.at(i));
        if (!one.isEmpty() && !allIps.contains(one)) {
            allIps << one;
        }
    }

    const QString name = um->name(row);
    const QString host = um->host(row);
    const QString ip = um->ip(row);
    // A memo typed against either NIC should label the folded row.
    // The preferred address wins when both have one.
    QString memo = um->memoForIp(ip);
    if (memo.isEmpty()) {
        for (int i = 0; i < allIps.size(); ++i) {
            memo = um->memoForIp(allIps.at(i));
            if (!memo.isEmpty()) {
                break;
            }
        }
    }
    const int presence = um->presence(row);
    const QString display = memo.isEmpty() ? name : memo;

    QStringList extras;
    for (int i = 0; i < allIps.size(); ++i) {
        if (allIps.at(i) != ip) {
            extras << allIps.at(i);
        }
    }
    // Show the send address, and that more exist, without a second row
    // the user could multi-select and message twice.
    QString ipShown = ip;
    if (extras.size() == 1) {
        ipShown = QString("%1 · +%2").arg(ip, extras.first());
    } else if (extras.size() > 1) {
        ipShown = QString("%1 · +%2").arg(ip).arg(extras.size());
    }

    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    QString subtitle = memo.isEmpty()
            ? (host.isEmpty() ? ipShown : QString("%1 · %2").arg(host, ipShown))
            : (host.isEmpty()
               ? QString("%1 · %2").arg(name, ipShown)
               : QString("%1 · %2 · %3").arg(name, host, ipShown));
    bool dialup = false;
    bool share = false;
    bool typing = false;
    for (int i = 0; i < allIps.size(); ++i) {
        if (um->peerIsDialup(allIps.at(i))) {
            dialup = true;
        }
        if (um->peerHasShare(allIps.at(i))) {
            share = true;
        }
        if (m_typingIps.contains(allIps.at(i))) {
            typing = true;
        }
    }
    if (dialup) {
        subtitle = QString("%1 · %2").arg(tr("Dial-up"), subtitle);
    }
    item->setText(0, QString("%1 %2 %3 %4").arg(display, name, host, ipShown));
    item->setData(0, Qt::UserRole, ip);
    // Same department the tree parent used: the preferred row, or
    // another NIC when that row's group was empty. Group send matches
    // this field, so it must not stay blank in that case.
    QString groupName = um->group(row);
    if (groupName.isEmpty()) {
        for (int i = 0; i < foldRows.size(); ++i) {
            const QString other = um->group(foldRows.at(i));
            if (!other.isEmpty()) {
                groupName = other;
                break;
            }
        }
    }
    item->setData(0, Qt::UserRole + 3, groupName);
    item->setData(0, Qt::UserRole + 4, subtitle);
    item->setData(0, Qt::UserRole + 6, display);
    item->setData(0, Qt::UserRole + 9, typing);
    item->setData(0, Qt::UserRole + 10, share);
    item->setData(0, Qt::UserRole + 11, dialup);
    item->setData(0, Qt::UserRole + 12, allIps);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    item->setSizeHint(0, QSize(180, 52));

    if (presence == PRESENCE_STALE) {
        item->setForeground(0, QBrush(QColor(120, 120, 120)));
    }
    item->setIcon(0, QIcon(peerAvatar(display, presence, 36, ip)));
    item->setToolTip(0, um->peerCaption(ip));
}

void MainWindow::addSelfItem(QTreeWidgetItem *parent)
{
    const Owner me = Global::userManager->ourself();
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    const QString host = me.host();
    const QString caption = me.name();
    const bool dialup = Global::preferences && Global::preferences->isDialup;
    item->setText(0, caption);
    QString selfSub = host.isEmpty() ? tr("This is you") : host;
    if (dialup) {
        selfSub = QString("%1 · %2").arg(selfSub, tr("Dial-up"));
    }
    item->setData(0, Qt::UserRole + 4, selfSub);
    item->setData(0, Qt::UserRole + 11, dialup);
    item->setData(0, Qt::UserRole + 6, caption);
    item->setFlags(Qt::ItemIsEnabled);
    item->setIcon(0, QIcon(peerAvatar(me.name(), PRESENCE_ONLINE, 36,
                                      AvatarStore::selfKey())));
    const bool sharing = Global::preferences
            && Global::preferences->shareEnabled
            && !Global::preferences->sharePath.trimmed().isEmpty();
    item->setData(0, Qt::UserRole + 10, sharing);
    QString selfTip = sharing
            ? tr("This is you\nSharing %1").arg(Global::preferences->sharePath)
            : tr("This is you");
    if (Global::avatarStore && Global::avatarStore->selfWireId().isEmpty()) {
        selfTip += QLatin1Char('\n');
        selfTip += tr("No photo is published to qipmsg peers.");
    } else {
        selfTip += QLatin1Char('\n');
        selfTip += tr("A small photo is published to qipmsg peers. FeiQ cannot fetch it.");
    }
    if (dialup) {
        selfTip += QLatin1Char('\n');
        selfTip += tr("Dial-up host. Presence packets set IPMSG_DIALUPOPT.");
    }
    item->setToolTip(0, selfTip);
}

void MainWindow::addDiscussionItem(QTreeWidgetItem *parent, const QString &id)
{
    if (!Global::discussionStore) {
        return;
    }
    const DiscussionGroup group = Global::discussionStore->groupById(id);
    if (group.id.isEmpty()) {
        return;
    }
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    const QString summary = group.memberSummary();
    item->setText(0, QString("%1 %2").arg(group.name, summary));
    item->setData(0, Qt::UserRole, QString());
    item->setData(0, Qt::UserRole + 1, QString());
    item->setData(0, Qt::UserRole + 4, summary);
    item->setData(0, Qt::UserRole + 5, QLatin1String("discussion"));
    item->setData(0, Qt::UserRole + 6, group.name);
    item->setData(0, Qt::UserRole + 7, group.id);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    item->setSizeHint(0, QSize(180, 52));
    item->setIcon(0, QIcon(peerAvatar(group.name, PRESENCE_ONLINE, 36)));
    item->setToolTip(0, tr("%1\n%2\nLocal multi-send (not a FeiQ multicast room)")
                     .arg(group.name, summary));
}

void MainWindow::addRecentItems(QTreeWidgetItem *parent)
{
    if (!parent || !Global::recentStore || !Global::userManager) {
        return;
    }
    const QList<RecentContact> rows = Global::recentStore->items();
    // Recent chats are stored per IP. Two NICs of one person would
    // otherwise fill this section the same way the friend list did.
    // Keep the newest snippet. Clicking still opens that IP's history
    // rather than jumping to the preferred NIC and an empty transcript.
    // Walk past folded duplicates so the section can still show 20 people.
    QSet<int> seenFolds;
    int shown = 0;
    for (int i = 0; i < rows.size() && shown < 20; ++i) {
        const RecentContact &row = rows.at(i);
        const int userRow = Global::userManager->ipToRow(row.ip);
        if (userRow >= 0) {
            const int preferred = Global::userManager->preferredRowForIp(row.ip);
            const int foldId = preferred >= 0 ? preferred : userRow;
            if (seenFolds.contains(foldId)) {
                continue;
            }
            seenFolds.insert(foldId);
        }
        const int presence = userRow == -1
                ? PRESENCE_STALE : Global::userManager->presence(userRow);
        QString name = row.name;
        const QString memo = Global::userManager->memoForIp(row.ip);
        if (!memo.isEmpty()) {
            name = memo;
        } else if (name.isEmpty()) {
            name = Global::userManager->displayNameForIp(row.ip);
        }
        QString whenText = row.when.toString("MM-dd hh:mm");
        if (row.when.date() == QDate::currentDate()) {
            whenText = row.when.toString("hh:mm");
        }
        const QString snippet = row.text.isEmpty() ? tr("(no text)") : row.text;
        const QString subtitle = QString("%1 · %2").arg(whenText, snippet);

        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, QString("%1 %2 %3").arg(name, snippet, row.ip));
        item->setData(0, Qt::UserRole, row.ip);
        item->setData(0, Qt::UserRole + 3, userRow == -1
                ? QString() : Global::userManager->group(userRow));
        item->setData(0, Qt::UserRole + 4, subtitle);
        item->setData(0, Qt::UserRole + 6, name);
        item->setData(0, Qt::UserRole + 8, QStringLiteral("recent"));
        item->setData(0, Qt::UserRole + 9, m_typingIps.contains(row.ip));
        item->setData(0, Qt::UserRole + 10,
                      Global::userManager
                      && Global::userManager->peerHasShare(row.ip));
        const bool dialup = Global::userManager->peerIsDialup(row.ip);
        item->setData(0, Qt::UserRole + 11, dialup);
        const QString shownSubtitle = dialup
                ? QString("%1 · %2").arg(tr("Dial-up"), subtitle)
                : subtitle;
        item->setData(0, Qt::UserRole + 4, shownSubtitle);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        item->setSizeHint(0, QSize(180, 52));
        if (presence == PRESENCE_STALE) {
            item->setForeground(0, QBrush(QColor(120, 120, 120)));
        }
        item->setIcon(0, QIcon(peerAvatar(name, presence, 36, row.ip)));
        item->setToolTip(0, tr("Click to open chat\n%1").arg(subtitle));
        ++shown;
    }
    updateGroupTitle(parent, tr("Recent contacts"));
}

bool MainWindow::isDiscussionItem(const QTreeWidgetItem *item) const
{
    return item && !item->data(0, Qt::UserRole + 7).toString().isEmpty();
}

QStringList MainWindow::selectedDiscussionIds() const
{
    QStringList ids;
    if (!contactTree) {
        return ids;
    }
    foreach (QTreeWidgetItem *item, contactTree->selectedItems()) {
        const QString id = item->data(0, Qt::UserRole + 7).toString();
        if (!id.isEmpty() && !ids.contains(id)) {
            ids << id;
        }
    }
    return ids;
}

bool MainWindow::isBlacklisted(const QString &ip) const
{
    return Global::preferences->blacklistIps.contains(ip);
}

void MainWindow::rebuildContactTree()
{
    const QStringList selected = selectedPeerIps();
    const QStringList selectedDisc = selectedDiscussionIds();
    QSet<QString> expanded;
    for (int i = 0; i < contactTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *g = contactTree->topLevelItem(i);
        if (g->isExpanded()) {
            expanded.insert(g->data(0, Qt::UserRole + 1).toString());
        }
        for (int c = 0; c < g->childCount(); ++c) {
            QTreeWidgetItem *sub = g->child(c);
            if (sub->isExpanded() && sub->data(0, Qt::UserRole).toString().isEmpty()) {
                expanded.insert(sub->data(0, Qt::UserRole + 1).toString());
            }
        }
    }
    if (expanded.isEmpty()) {
        expanded << "friends" << "ungrouped" << "self" << "blacklist"
                 << "discussions" << "recent";
    }

    contactTree->clear();

    QTreeWidgetItem *recent = makeGroupItem(tr("Recent contacts"), "recent");
    addRecentItems(recent);

    QTreeWidgetItem *friends = makeGroupItem(tr("My Friends"), "friends");
    QTreeWidgetItem *ungrouped = makeGroupItem(tr("Ungrouped friends"),
                                               "ungrouped");
    friends->addChild(ungrouped);

    QMap<QString, QTreeWidgetItem *> depts;
    QTreeWidgetItem *discGroup = makeGroupItem(tr("Discussions"), "discussions");
    QTreeWidgetItem *selfGroup = makeGroupItem(tr("Myself"), "self");
    QTreeWidgetItem *blackGroup = makeGroupItem(tr("Blacklist"), "blacklist");

    UserManager *um = Global::userManager;
    // One tree row per login+host. The model underneath is still one
    // record per IP so file receive can try every NIC, and so a chat
    // that arrived on ZeroTier can stay on that address.
    const QList<PeerFoldGroup> folds = um->contactFolds();
    for (int g = 0; g < folds.size(); ++g) {
        const PeerFoldGroup &fold = folds.at(g);
        int row = fold.preferred;
        if (row < 0 || row >= um->userCount()) {
            continue;
        }
        const QString ip = um->ip(row);
        if (ip.isEmpty()) {
            continue;
        }
        // Blacklist follows the address we would send to. An old
        // blacklist entry for the other NIC does not hide the person;
        // blacklisting the folded row records the preferred address.
        if (isBlacklisted(ip)) {
            addPeerItem(blackGroup, row);
            continue;
        }
        // Prefer the department on the send address. If that packet
        // had an empty group, keep a department announced by the other
        // NIC so FeiQ does not fall into Ungrouped.
        QString group = um->group(row);
        if (group.isEmpty()) {
            for (int n = 0; n < fold.indexes.size(); ++n) {
                const QString other = um->group(fold.indexes.at(n));
                if (!other.isEmpty()) {
                    group = other;
                    break;
                }
            }
        }
        if (group.isEmpty()) {
            addPeerItem(ungrouped, row);
        } else {
            if (!depts.contains(group)) {
                depts.insert(group, makeGroupItem(group, "dept:" + group));
            }
            addPeerItem(depts.value(group), row);
        }
    }
    addSelfItem(selfGroup);

    if (Global::discussionStore) {
        foreach (const DiscussionGroup &group, Global::discussionStore->groups()) {
            addDiscussionItem(discGroup, group.id);
        }
    }

    updateGroupTitle(ungrouped, tr("Ungrouped friends"));
    friends->setText(0, QString("%1 (%2)")
                     .arg(tr("My Friends")).arg(ungrouped->childCount()));
    updateGroupTitle(discGroup, tr("Discussions"));
    updateGroupTitle(selfGroup, tr("Myself"));
    updateGroupTitle(blackGroup, tr("Blacklist"));

    contactTree->addTopLevelItem(recent);
    contactTree->addTopLevelItem(friends);
    contactTree->addTopLevelItem(discGroup);
    QMapIterator<QString, QTreeWidgetItem *> it(depts);
    while (it.hasNext()) {
        it.next();
        QString deptTitle = it.key();
        if (Global::groupPolicy) {
            QStringList marks;
            if (Global::groupPolicy->isHidden(it.key())) {
                marks << tr("hidden");
            }
            if (Global::groupPolicy->isMuted(it.key())) {
                marks << tr("muted");
            }
            if (!marks.isEmpty()) {
                deptTitle += QStringLiteral(" · ") + marks.join(QStringLiteral(" · "));
            }
        }
        updateGroupTitle(it.value(), deptTitle);
        contactTree->addTopLevelItem(it.value());
    }
    contactTree->addTopLevelItem(selfGroup);
    contactTree->addTopLevelItem(blackGroup);

    for (int i = 0; i < contactTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *g = contactTree->topLevelItem(i);
        const QString key = g->data(0, Qt::UserRole + 1).toString();
        g->setExpanded(expanded.contains(key) || key.startsWith("dept:"));
        for (int c = 0; c < g->childCount(); ++c) {
            QTreeWidgetItem *sub = g->child(c);
            const QString subKey = sub->data(0, Qt::UserRole + 1).toString();
            if (!subKey.isEmpty()) {
                sub->setExpanded(expanded.contains(subKey));
            }
        }
    }

    applyContactFilter(searchEdit->text());

    foreach (QTreeWidgetItem *item, allPeerItems()) {
        if (selected.contains(item->data(0, Qt::UserRole).toString())) {
            item->setSelected(true);
        }
    }
    QList<QTreeWidgetItem *> stack;
    stack << discGroup;
    while (!stack.isEmpty()) {
        QTreeWidgetItem *item = stack.takeFirst();
        if (selectedDisc.contains(item->data(0, Qt::UserRole + 7).toString())) {
            item->setSelected(true);
        }
        for (int i = 0; i < item->childCount(); ++i) {
            stack << item->child(i);
        }
    }

    updateHeader();
}

QStringList MainWindow::selectedPeerIps() const
{
    QStringList ips;
    if (!contactTree) {
        return ips;
    }
    foreach (QTreeWidgetItem *item, contactTree->selectedItems()) {
        const QString ip = item->data(0, Qt::UserRole).toString();
        if (!ip.isEmpty() && !ips.contains(ip)) {
            ips << ip;
        }
    }
    return ips;
}

QList<QTreeWidgetItem *> MainWindow::allPeerItems() const
{
    QList<QTreeWidgetItem *> items;
    QList<QTreeWidgetItem *> stack;
    for (int i = 0; i < contactTree->topLevelItemCount(); ++i) {
        stack << contactTree->topLevelItem(i);
    }
    while (!stack.isEmpty()) {
        QTreeWidgetItem *item = stack.takeFirst();
        if (!item->data(0, Qt::UserRole).toString().isEmpty()) {
            items << item;
        }
        for (int i = 0; i < item->childCount(); ++i) {
            stack << item->child(i);
        }
    }
    return items;
}

void MainWindow::applyContactFilter(const QString &text)
{
    applyFilterToItem(contactTree->invisibleRootItem(), text.trimmed());
}

void MainWindow::applyFilterToItem(QTreeWidgetItem *item, const QString &needle)
{
    for (int i = 0; i < item->childCount(); ++i) {
        applyFilterToItem(item->child(i), needle);
    }

    if (item == contactTree->invisibleRootItem()) {
        return;
    }

    const bool isPeer = !item->data(0, Qt::UserRole).toString().isEmpty()
            || isDiscussionItem(item);
    if (needle.isEmpty()) {
        item->setHidden(false);
        return;
    }

    if (isPeer) {
        const QString ip = item->data(0, Qt::UserRole).toString();
        UserManager *um = Global::userManager;
        const int row = um->ipToRow(ip);
        QStringList fields;
        fields << item->text(0) << item->toolTip(0) << ip;
        // UserRole+12 is every NIC of this folded contact, so a search
        // for the ZeroTier address still finds the LAN row.
        fields << item->data(0, Qt::UserRole + 12).toStringList();
        if (row != -1) {
            fields << um->name(row) << um->host(row) << um->loginName(row)
                   << um->group(row);
        }
        fields << um->memoForIp(ip);
        item->setHidden(!Pinyin::matchesAny(fields, needle));
        return;
    }

    bool anyVisible = false;
    for (int i = 0; i < item->childCount(); ++i) {
        if (!item->child(i)->isHidden()) {
            anyVisible = true;
            break;
        }
    }
    item->setHidden(!anyVisible && !Pinyin::matches(item->text(0), needle));
    if (!item->isHidden()) {
        item->setExpanded(true);
    }
}

void MainWindow::saveSignature()
{
    const QString sign = signatureEdit->text();
    if (sign == Global::preferences->personSign) {
        return;
    }
    Global::preferences->personSign = sign;
    Global::preferences->save();
    Global::userManager->updateOurself();
    Global::userManager->announcePresence();
}

void MainWindow::toggleBlacklist()
{
    const QStringList ips = selectedPeerIps();
    if (ips.isEmpty()) {
        return;
    }
    QStringList &list = Global::preferences->blacklistIps;
    const bool remove = ips.size() == 1 && list.contains(ips.first());
    foreach (const QString &ip, ips) {
        if (remove) {
            list.removeAll(ip);
        } else if (!list.contains(ip)) {
            list.append(ip);
        }
    }
    Global::preferences->save();
    rebuildContactTree();
}

void MainWindow::createDiscussion()
{
    if (!Global::discussionStore) {
        return;
    }
    const QStringList ips = selectedPeerIps();
    if (ips.size() < 2) {
        QMessageBox::information(this, tr("Discussion group"),
                tr("Select at least two contacts to create a discussion group."));
        return;
    }
    bool ok = false;
    const QString name = QInputDialog::getText(this,
            tr("New discussion group"), tr("Name:"),
            QLineEdit::Normal,
            tr("Discussion (%1)").arg(ips.size()), &ok);
    if (!ok || name.trimmed().isEmpty()) {
        return;
    }
    QList<DiscussionMember> members;
    foreach (const QString &ip, ips) {
        members << DiscussionStore::memberFromIp(ip);
    }
    const QString id = Global::discussionStore->addGroup(name, members);
    if (id.isEmpty()) {
        QMessageBox::warning(this, tr("Discussion group"),
                tr("Could not create the discussion group."));
        return;
    }
    Global::windowManager->openDiscussion(id, true);
}

void MainWindow::renameDiscussion()
{
    if (!Global::discussionStore) {
        return;
    }
    const QStringList ids = selectedDiscussionIds();
    if (ids.size() != 1) {
        return;
    }
    const DiscussionGroup group = Global::discussionStore->groupById(ids.first());
    bool ok = false;
    const QString name = QInputDialog::getText(this,
            tr("Rename discussion group"), tr("Name:"),
            QLineEdit::Normal, group.name, &ok);
    if (!ok || name.trimmed().isEmpty()) {
        return;
    }
    Global::discussionStore->renameGroup(ids.first(), name);
}

void MainWindow::deleteDiscussion()
{
    if (!Global::discussionStore) {
        return;
    }
    const QStringList ids = selectedDiscussionIds();
    if (ids.isEmpty()) {
        return;
    }
    const QString prompt = (ids.size() == 1)
            ? tr("Delete this discussion group? Local history stays on disk.")
            : tr("Delete the selected discussion groups? Local history stays on disk.");
    if (QMessageBox::question(this, tr("Delete discussion group"), prompt,
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            != QMessageBox::Yes) {
        return;
    }
    foreach (const QString &id, ids) {
        Global::discussionStore->removeGroup(id);
    }
}

void MainWindow::addSelectedToDiscussion()
{
    if (!Global::discussionStore) {
        return;
    }
    const QStringList ids = selectedDiscussionIds();
    const QStringList ips = selectedPeerIps();
    if (ids.size() != 1 || ips.isEmpty()) {
        return;
    }
    QList<DiscussionMember> members;
    foreach (const QString &ip, ips) {
        members << DiscussionStore::memberFromIp(ip);
    }
    Global::discussionStore->addMembers(ids.first(), members);
}

void MainWindow::editPeerMemo()
{
    const QStringList ips = selectedPeerIps();
    if (ips.size() != 1) {
        return;
    }
    const QString ip = ips.first();
    const QString real = Global::userManager->realNameForIp(ip);
    bool ok = false;
    const QString memo = QInputDialog::getText(
                this,
                tr("Set memo"),
                tr("Local memo for %1:").arg(real),
                QLineEdit::Normal,
                Global::userManager->memoForIp(ip),
                &ok);
    if (ok) {
        Global::userManager->setMemoForIp(ip, memo);
    }
}

void MainWindow::sendMessage()
{
    quint32 baseFlags = IPMSG_SENDMSG | IPMSG_SENDCHECKOPT;
    if (encapCheckBox->isChecked()) {
        baseFlags |= IPMSG_SECRETOPT | IPMSG_READCHECKOPT;
    }

    const QStringList ips = selectedPeerIps();
    if (ips.isEmpty()) {
        return;
    }
    if (ips.size() > 1) {
        baseFlags |= IPMSG_MULTICASTOPT;
    }

    int queuedCount = 0;
    bool warnedFiles = false;
    foreach (const QString &ip, ips) {
        if (!Global::userManager->isReachable(ip)) {
            if (hasSendFile() && !warnedFiles) {
                const bool hasText = !inputEdit->toPlainText().trimmed().isEmpty();
                QMessageBox::information(this, tr("Offline queue"),
                        hasText
                        ? tr("Files stay out of the offline queue because there is no "
                             "transfer server to resume them. Only the text can be queued.")
                        : tr("Files stay out of the offline queue because there is no "
                             "transfer server to resume them. Nothing was queued."));
                warnedFiles = true;
            }
            const QString body = inputEdit->toPlainText();
            if (!body.trimmed().isEmpty()
                    && Global::windowManager->queueOrDeliverText(ip, body, this)) {
                ++queuedCount;
            }
            continue;
        }

        const int row = Global::userManager->ipToRow(ip);
        QString recvUser = Global::userManager->displayNameForIp(ip);
        QString recvHostname = row != -1 ? Global::userManager->host(row)
                                         : QString();

        quint32 flags = Global::userManager->sendFlagsForIp(ip, baseFlags);
        QString additionalInfo = inputEdit->toPlainText();
        additionalInfo.append(QChar('\0'));

        if (hasSendFile()) {
            m_sendFileMap = new SendFileMap;
            flags |= IPMSG_FILEATTACHOPT;
            QStringList pathList = m_sendFileModel.pathList();
            m_sendFileMap->addFile(pathList);
            const bool withHash = Global::userManager
                    && Global::userManager->peerIsQipmsg(ip);
            additionalInfo.append(m_sendFileMap->packetString(withHash));
            additionalInfo.append(QChar('\0'));
        }

        const bool encrypted = MsgCrypto::prepareOutgoing(ip, &additionalInfo, &flags);

        if (Global::preferences->isLogMsg) {
            logSendMsg(inputEdit->toPlainText(), ip);
        }

        SendMsg sendMsg = SendMsg(QHostAddress(ip),
                                  Global::userManager->sendPortForIp(ip),
                                  additionalInfo, ""/* extendedInfo */,
                                  flags);

        Global::windowManager->recordOutgoing(
                    ip, recvUser, inputEdit->toPlainText(), QStringList(),
                    sendMsg.packetNoString(),
                    (baseFlags & IPMSG_READCHECKOPT) != 0, encrypted);

        if (hasSendFile()) {
            m_sendFileMap->setRecvUser(recvUser);
            m_sendFileMap->setRecvHostname(recvHostname);
            m_sendFileMap->setPacketNoString(sendMsg.packetNoString());
            Global::sendFileManager
                ->addTransferLocked(m_sendFileMap->packetNoString(),
                         m_sendFileMap);
        }

        Global::msgThread->addSendMsg(Msg(sendMsg));
    }

    if (queuedCount > 0) {
        QMessageBox::information(this, tr("Offline queue"),
                tr("%n message(s) queued and will send when the peer comes online.",
                   "", queuedCount));
    }

    emit messageReplyed();
    close();
}

void MainWindow::broadcastToOnline()
{
    const QString text = inputEdit->toPlainText();
    if (text.trimmed().isEmpty()) {
        QMessageBox::information(this, tr("Broadcast"),
                tr("Type a message, then broadcast it to every online peer."));
        return;
    }
    const int sent = Global::windowManager->broadcastTextToOnline(text);
    if (sent <= 0) {
        QMessageBox::information(this, tr("Broadcast"),
                tr("No online peers to broadcast to."));
        return;
    }
    QMessageBox::information(this, tr("Broadcast"),
            tr("Broadcast sent to %n online peer(s).", "", sent));
    inputEdit->clear();
}

void MainWindow::dropEvent(QDropEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        QList<QUrl> urlList = e->mimeData()->urls();
        foreach (QUrl url, urlList) {
            if (url.scheme() == "file") {
                m_sendFileModel.addFile(url.toLocalFile());
            }
        }
    }

    fileListButton->setText(QString(tr("Show send file list (%1)"))
            .arg(Global::fileCountString(m_sendFileModel.rowCount())));
    if (!fileListButton->isVisible()) {
        fileListButton->show();
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}

void MainWindow::showSendFileList()
{
    sendFileWindow = new SendFileWindow(&m_sendFileModel, this);
    connect(sendFileWindow, SIGNAL(addFileFinished()),
            this, SLOT(updateFileCount()));
    sendFileWindow->setWindowFlags(Qt::Dialog);
    sendFileWindow->setWindowModality(Qt::WindowModal);
    setAcceptDrops(false);
    sendFileWindow->show();
}

void MainWindow::updateFileCount()
{
    setAcceptDrops(true);

    if (m_sendFileModel.rowCount() == 0) {
        if (fileListButton->isVisible()) {
            fileListButton->hide();
        }
    } else {
        fileListButton->setText(QString(tr("Show send file list (%1)"))
                .arg(Global::fileCountString(m_sendFileModel.rowCount())));
        if (!fileListButton->isVisible()) {
            fileListButton->show();
        }
    }
}

bool MainWindow::hasSendFile()
{
    return m_sendFileModel.rowCount() > 0;
}

void MainWindow::selectUser()
{
    if (m_selectedIp.isEmpty()) {
        return;
    }
    foreach (QTreeWidgetItem *item, allPeerItems()) {
        if (item->data(0, Qt::UserRole).toString() == m_selectedIp) {
            item->setSelected(true);
            contactTree->scrollToItem(item);
            break;
        }
    }
}

void MainWindow::logSendMsg(QString text, const QString &ip)
{
    if (!Global::preferences->logFile.isWritable()) {
        return;
    }

    const int row = Global::userManager->ipToRow(ip);
    QTextStream ts(&Global::preferences->logFile);

    QString s("=====================================\n");
    s.append(tr(" To: "));
    s.append(row != -1 ? Global::userManager->name(row)
                       : Global::userManager->displayNameForIp(ip));
    if (row != -1 && Global::preferences->isLogLoginName) {
        s.append("[");
        s.append(Global::userManager->loginName(row));
        s.append("]");
    }

    s.append(" (");
    if (row != -1 && !Global::userManager->group(row).isEmpty()) {
        s.append(Global::userManager->group(row));
        s.append("/");
    }
    if (row != -1) {
        s.append(Global::userManager->host(row));
    }
    if (Global::preferences->isLogIP) {
        s.append("/");
        s.append(ip);
    }
    s.append(")\n");
    s.append(tr(" Time: "));
    s.append(QDateTime::currentDateTime().toString("yyyy-MM-dd dddd hh:mm:ss"));
    if (encapCheckBox->isChecked()) {
        s.append(tr(" (Sealed)"));
    }
    s.append("\n");

    if (hasSendFile()) {
        QString files = "  ("
            + Global::fileCountString(m_sendFileModel.rowCount()) + ") ";
        files.append(m_sendFileModel.fileNameJoin(", "));
        s.append(files);
        s.append(QChar('\n'));
    }

    s.append("-------------------------------------\n");
    s.append(text);
    s.append("\n\n");
    ts << s << Qt::flush;
}

void MainWindow::refreshUserList()
{
    Global::userManager->announcePresence();
}

void MainWindow::openSchedule()
{
    QString ip;
    if (contactTree && !contactTree->selectedItems().isEmpty()) {
        ip = contactTree->selectedItems().first()->data(0, Qt::UserRole).toString();
    }
    ScheduleDialog dialog(ip, this);
    dialog.exec();
}

void MainWindow::onContactDoubleClicked(QTreeWidgetItem *item, int)
{
    if (!item) {
        return;
    }
    const QString discId = item->data(0, Qt::UserRole + 7).toString();
    if (!discId.isEmpty()) {
        Global::windowManager->openDiscussion(discId, true);
        return;
    }
    const QString ip = item->data(0, Qt::UserRole).toString();
    if (!ip.isEmpty()) {
        Global::windowManager->openChat(ip, true);
    }
}

void MainWindow::openSelectedChat()
{
    foreach (const QString &id, selectedDiscussionIds()) {
        Global::windowManager->openDiscussion(id, true);
    }
    foreach (const QString &ip, selectedPeerIps()) {
        Global::windowManager->openChat(ip, true);
    }
}

void MainWindow::findInHistory()
{
    HistorySearchWindow *window = new HistorySearchWindow;
    window->show();
    window->raise();
    window->activateWindow();
}

void MainWindow::addSendFile()
{
    setAcceptDrops(false);
    QStringList pathList = QFileDialog::getOpenFileNames(this, tr("Add Files"),
            Global::preferences->lastSendFilePath,
            tr("All Files (*)"));
    m_sendFileModel.addFile(pathList);
    if (!pathList.isEmpty()) {
        Global::preferences->lastSendFilePath = pathList.at(0);
    }
    updateFileCount();
}

void MainWindow::addSendDir()
{
    DirDialog *dirDialog = new DirDialog(&m_sendFileModel, this);
    connect(dirDialog, SIGNAL(dirSelected()),
            this, SLOT(updateFileCount()));
    dirDialog->show();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    QMainWindow::keyPressEvent(event);
}

void MainWindow::showSearchDialog()
{
    searchEdit->setFocus();
    searchEdit->selectAll();
}

void MainWindow::search(QString searchString)
{
    searchEdit->setText(searchString);
}

QString MainWindow::constructFileInfoStr()
{
    return m_sendFileModel.fileNameJoin(", ");
}
