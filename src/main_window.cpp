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

quint32 MainWindow::m_levelOneCount = 0;
quint32 MainWindow::m_levelTwoCount = 0;

MainWindow::MainWindow(QString text, QString ip,
        QWidget *parent)
    : QMainWindow(parent), m_initText(text), m_selectedIp(ip),
    m_lastSearchDialog(0)
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
    connect(refreshButton, SIGNAL(clicked()),
            this, SLOT(refreshUserList()));

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

    QWidget *searchBar = new QWidget;
    searchBar->setObjectName("searchBar");
    QHBoxLayout *searchRow = new QHBoxLayout(searchBar);
    searchRow->setContentsMargins(12, 8, 12, 8);
    searchRow->addWidget(searchEdit, 1);
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

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(fileListButton);
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
    menu.addMenu(selectGroupMenu);
    menu.addAction(sendFileAct);
    menu.addAction(sendDirAct);
    menu.addAction(searchUserAct);
    menu.addAction(findHistoryAct);
    menu.addSeparator();
    menu.addAction(createDiscussionAct);
    menu.addAction(renameDiscussionAct);
    menu.addAction(deleteDiscussionAct);
    menu.addAction(addToDiscussionAct);
    menu.addAction(setMemoAct);
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
    searchUserAct = new QAction(tr("Search user"), this);
    findHistoryAct = new QAction(tr("Find in history..."), this);
    connect(findHistoryAct, SIGNAL(triggered()),
            this, SLOT(findInHistory()));
    blacklistAct = new QAction(tr("Add to blacklist"), this);
    setMemoAct = new QAction(tr("Set memo..."), this);
    connect(setMemoAct, SIGNAL(triggered()),
            this, SLOT(editPeerMemo()));

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

void MainWindow::updateSelectGroupMenu()
{
    selectGroupMenu->clear();

    foreach (QString group, Global::preferences->groupNameList) {
        if (!group.isEmpty()) {
            QAction *action = new QAction(group, this);
            action->setData(group);
            connect(action, SIGNAL(triggered()), this, SLOT(selectGroup()));
            selectGroupMenu->addAction(action);
        }
    }
}

void MainWindow::selectGroup()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action) {
        return;
    }
    const QString group = action->data().toString();
    contactTree->clearSelection();

    foreach (QTreeWidgetItem *item, allPeerItems()) {
        if (item->data(0, Qt::UserRole + 3).toString() == group) {
            item->setSelected(true);
            contactTree->scrollToItem(item);
        }
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
    nickStatusLabel->setText(me.name());
    onlineCountLabel->setText(tr("%1 · %n online", "",
                                 Global::userManager->onlineCount()).arg(status));
    avatarLabel->setPixmap(peerAvatar(me.name(), presence, 40));
}

QPixmap MainWindow::peerAvatar(const QString &name, int presence, int size) const
{
    return Helper::avatarPixmap(name, size, Helper::avatarColor(name),
                                Helper::presenceColor(presence));
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
    const QString name = um->name(row);
    const QString host = um->host(row);
    const QString ip = um->ip(row);
    const QString memo = um->memoForIp(ip);
    const int presence = um->presence(row);
    const QString display = memo.isEmpty() ? name : memo;

    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    const QString subtitle = memo.isEmpty()
            ? (host.isEmpty() ? ip : QString("%1 · %2").arg(host, ip))
            : (host.isEmpty()
               ? QString("%1 · %2").arg(name, ip)
               : QString("%1 · %2 · %3").arg(name, host, ip));
    item->setText(0, QString("%1 %2 %3 %4").arg(display, name, host, ip));
    item->setData(0, Qt::UserRole, ip);
    item->setData(0, Qt::UserRole + 3, um->group(row));
    item->setData(0, Qt::UserRole + 4, subtitle);
    item->setData(0, Qt::UserRole + 6, display);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    item->setSizeHint(0, QSize(180, 52));

    if (presence == PRESENCE_STALE) {
        item->setForeground(0, QBrush(QColor(120, 120, 120)));
    }
    item->setIcon(0, QIcon(peerAvatar(display, presence, 36)));
    item->setToolTip(0, um->peerCaption(ip));
}

void MainWindow::addSelfItem(QTreeWidgetItem *parent)
{
    const Owner me = Global::userManager->ourself();
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    const QString host = me.host();
    const QString caption = me.name();
    item->setText(0, caption);
    item->setData(0, Qt::UserRole + 4,
                  host.isEmpty() ? tr("This is you") : host);
    item->setData(0, Qt::UserRole + 6, caption);
    item->setFlags(Qt::ItemIsEnabled);
    item->setIcon(0, QIcon(peerAvatar(me.name(), PRESENCE_ONLINE, 36)));
    item->setToolTip(0, tr("This is you"));
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
                 << "discussions";
    }

    contactTree->clear();

    QTreeWidgetItem *friends = makeGroupItem(tr("My Friends"), "friends");
    QTreeWidgetItem *ungrouped = makeGroupItem(tr("Ungrouped friends"),
                                               "ungrouped");
    friends->addChild(ungrouped);

    QMap<QString, QTreeWidgetItem *> depts;
    QTreeWidgetItem *discGroup = makeGroupItem(tr("Discussions"), "discussions");
    QTreeWidgetItem *selfGroup = makeGroupItem(tr("Myself"), "self");
    QTreeWidgetItem *blackGroup = makeGroupItem(tr("Blacklist"), "blacklist");

    UserManager *um = Global::userManager;
    for (int row = 0; row < um->userCount(); ++row) {
        const QString ip = um->ip(row);
        if (ip.isEmpty()) {
            continue;
        }
        if (isBlacklisted(ip)) {
            addPeerItem(blackGroup, row);
            continue;
        }
        const QString group = um->group(row);
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

    contactTree->addTopLevelItem(friends);
    contactTree->addTopLevelItem(discGroup);
    QMapIterator<QString, QTreeWidgetItem *> it(depts);
    while (it.hasNext()) {
        it.next();
        updateGroupTitle(it.value(), it.key());
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
                QMessageBox::information(this, tr("Offline queue"),
                        tr("File attachments cannot be queued (no transfer server). "
                           "Text will be queued for offline peers."));
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
            additionalInfo.append(m_sendFileMap->packetString());
            additionalInfo.append(QChar('\0'));
        }

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
                    (baseFlags & IPMSG_READCHECKOPT) != 0);

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
