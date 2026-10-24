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

#include "chat_window.h"
#include "constants.h"
#include "emoji_pack.h"
#include "global.h"
#include "helper.h"
#include "history_search_window.h"
#include "ipmsg.h"
#include "message_store.h"
#include "msg.h"
#include "msg_thread.h"
#include "preferences.h"
#include "screen_capture.h"
#include "send_file_manager.h"
#include "send_file_map.h"
#include "send_file_window.h"
#include "send_msg.h"
#include "offline_queue.h"
#include "user_manager.h"
#include "window_manager.h"

#include "transfer_file_window.h"

#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QDateTime>
#include <QDesktopServices>
#include <QList>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHostAddress>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QRegularExpression>
#include <QShortcut>
#include <QShowEvent>
#include <QSplitter>
#include <QTextBlockFormat>
#include <QCursor>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextDocument>
#include <QVariant>
#include <QTextEdit>
#include <QTextImageFormat>
#include <QTextTable>
#include <QTextTableCell>
#include <QTextTableFormat>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidgetAction>

ChatWindow::ChatWindow(const QString &ip, QWidget *parent)
    : QWidget(parent), m_peerIp(ip), m_historyLoaded(false)
{
    setAcceptDrops(true);
    createUi();
    m_inputEdit->installEventFilter(this);
    refreshPeerCaption();
    refreshQueuePanel();
    if (Global::offlineQueue) {
        connect(Global::offlineQueue, SIGNAL(queueChanged()),
                this, SLOT(refreshQueuePanel()));
    }
    setWindowIcon(*Global::iconSet.value("normal"));
}

QSize ChatWindow::sizeHint() const
{
    return QSize(920, 600);
}

QToolButton *ChatWindow::makeToolButton(const QString &text, const QString &tip,
                                        const char *slot, bool enabled)
{
    QToolButton *button = new QToolButton;
    button->setText(text);
    button->setToolTip(tip);
    button->setObjectName(enabled ? "chatToolButton" : "stubButton");
    button->setToolButtonStyle(Qt::ToolButtonTextOnly);
    button->setAutoRaise(true);
    button->setEnabled(enabled);
    if (enabled && slot) {
        connect(button, SIGNAL(clicked()), this, slot);
    }
    return button;
}

QToolButton *ChatWindow::makeStubButton(const QString &text)
{
    return makeToolButton(text, tr("Not implemented yet"), 0, false);
}

void ChatWindow::createUi()
{
    setObjectName("chatRoot");

    m_headerAvatar = new QLabel;
    m_headerAvatar->setFixedSize(36, 36);
    m_peerLabel = new QLabel;
    m_peerLabel->setObjectName("chatPeerName");
    m_peerMeta = new QLabel;
    m_peerMeta->setObjectName("chatPeerMeta");

    QVBoxLayout *headerText = new QVBoxLayout;
    headerText->setContentsMargins(0, 0, 0, 0);
    headerText->setSpacing(2);
    headerText->addWidget(m_peerLabel);
    headerText->addWidget(m_peerMeta);

    QWidget *header = new QWidget;
    header->setObjectName("chatHeader");
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(14, 10, 14, 10);
    headerLayout->setSpacing(10);
    headerLayout->addWidget(m_headerAvatar);
    headerLayout->addLayout(headerText, 1);

    QWidget *toolbar = new QWidget;
    toolbar->setObjectName("chatToolbar");
    QHBoxLayout *toolLayout = new QHBoxLayout(toolbar);
    toolLayout->setContentsMargins(8, 4, 8, 4);
    toolLayout->setSpacing(2);
    toolLayout->addWidget(makeStubButton(tr("Remote assist")));
    toolLayout->addWidget(makeStubButton(tr("Remote control")));
    toolLayout->addWidget(makeStubButton(tr("Voice call")));
    toolLayout->addWidget(makeToolButton(tr("Send file"),
                                         tr("Send file"), SLOT(addSendFile())));
    toolLayout->addWidget(makeToolButton(tr("Send folder"),
                                         tr("Send folder"), SLOT(addSendDir())));
    toolLayout->addWidget(makeStubButton(tr("Browse share")));
    QToolButton *shot = makeToolButton(tr("Screenshot"),
                                       tr("Screenshot"), 0);
    QMenu *shotMenu = new QMenu(shot);
    shotMenu->addAction(tr("Full screen"), this, SLOT(captureFullScreen()));
    shotMenu->addAction(tr("Region"), this, SLOT(captureRegion()));
    shot->setMenu(shotMenu);
    shot->setPopupMode(QToolButton::InstantPopup);
    toolLayout->addWidget(shot);
    toolLayout->addWidget(makeToolButton(tr("Emoji"),
                                         tr("Insert emoji"),
                                         SLOT(showEmojiPicker())));
    toolLayout->addWidget(makeToolButton(tr("Window shake"),
                                         tr("Window shake"), SLOT(sendShake())));
    toolLayout->addWidget(makeToolButton(tr("Find"),
                                         tr("Find in history"),
                                         SLOT(findInHistory())));
    toolLayout->addWidget(makeStubButton(tr("Favorites")));
    toolLayout->addWidget(makeStubButton(tr("Zone")));
    toolLayout->addStretch();

    m_fileListButton = new QPushButton;
    m_fileListButton->hide();
    connect(m_fileListButton, SIGNAL(clicked()),
            this, SLOT(showSendFileList()));

    m_historyEdit = new QTextBrowser;
    m_historyEdit->setObjectName("historyView");
    m_historyEdit->setReadOnly(true);
    m_historyEdit->setAcceptRichText(true);
    m_historyEdit->setTabChangesFocus(true);
    m_historyEdit->setFrameShape(QFrame::NoFrame);
    m_historyEdit->setOpenLinks(false);
    m_historyEdit->setOpenExternalLinks(false);
    connect(m_historyEdit, SIGNAL(anchorClicked(QUrl)),
            this, SLOT(openChatLink(QUrl)));

    m_inputEdit = new QTextEdit;
    m_inputEdit->setObjectName("composerEdit");
    m_inputEdit->setAcceptRichText(false);
    m_inputEdit->setTabChangesFocus(true);
    m_inputEdit->setMaximumHeight(120);
    m_inputEdit->setAcceptDrops(false);
    m_inputEdit->setPlaceholderText(tr("Type a message..."));

    m_sendButton = new QPushButton(tr("&Send"));
    m_sendButton->setObjectName("primaryButton");
    connect(m_sendButton, SIGNAL(clicked()),
            this, SLOT(sendMessage()));
    connect(new QShortcut(QKeySequence("Ctrl+Return"), this),
            SIGNAL(activated()),
            this, SLOT(sendMessage()));
    connect(new QShortcut(QKeySequence("Ctrl+Enter"), this),
            SIGNAL(activated()),
            this, SLOT(sendMessage()));
    connect(new QShortcut(QKeySequence("Ctrl+Shift+S"), this),
            SIGNAL(activated()),
            this, SLOT(captureFullScreen()));
    connect(new QShortcut(QKeySequence("Ctrl+Shift+A"), this),
            SIGNAL(activated()),
            this, SLOT(captureRegion()));
    connect(new QShortcut(QKeySequence("Ctrl+F"), this),
            SIGNAL(activated()),
            this, SLOT(findInHistory()));

    m_stripFile = new QPushButton(tr("Send file"));
    m_stripMonitor = new QPushButton(tr("Monitor"));
    m_stripDir = new QPushButton(tr("Folder"));
    m_stripCancel = new QPushButton(tr("Cancel"));
    connect(m_stripFile, SIGNAL(clicked()), this, SLOT(addSendFile()));
    connect(m_stripMonitor, SIGNAL(clicked()),
            this, SLOT(showTransferMonitor()));
    connect(m_stripDir, SIGNAL(clicked()), this, SLOT(addSendDir()));
    connect(m_stripCancel, SIGNAL(clicked()), this, SLOT(cancelAttachments()));

    QWidget *composer = new QWidget;
    composer->setObjectName("composerBar");
    QVBoxLayout *composerLayout = new QVBoxLayout(composer);
    composerLayout->setContentsMargins(12, 8, 12, 12);
    composerLayout->setSpacing(8);

    QHBoxLayout *stripLayout = new QHBoxLayout;
    stripLayout->addWidget(m_stripFile);
    stripLayout->addWidget(m_stripMonitor);
    stripLayout->addWidget(m_stripDir);
    stripLayout->addWidget(m_fileListButton);
    stripLayout->addWidget(m_stripCancel);
    stripLayout->addStretch();

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_sendButton);

    composerLayout->addLayout(stripLayout);
    composerLayout->addWidget(m_inputEdit);
    composerLayout->addLayout(buttonLayout);

    m_queuePanel = new QWidget;
    m_queuePanel->setObjectName("offlineQueuePanel");
    m_queueBanner = new QLabel;
    m_queueBanner->setObjectName("offlineQueueBanner");
    m_queueBanner->setWordWrap(true);
    m_queueList = new QListWidget;
    m_queueList->setMaximumHeight(96);
    m_cancelQueuedButton = new QPushButton(tr("Cancel selected"));
    m_clearQueueButton = new QPushButton(tr("Clear queue"));
    connect(m_cancelQueuedButton, SIGNAL(clicked()),
            this, SLOT(cancelSelectedQueued()));
    connect(m_clearQueueButton, SIGNAL(clicked()),
            this, SLOT(clearQueued()));
    QHBoxLayout *queueButtons = new QHBoxLayout;
    queueButtons->setContentsMargins(0, 0, 0, 0);
    queueButtons->addWidget(m_cancelQueuedButton);
    queueButtons->addWidget(m_clearQueueButton);
    queueButtons->addStretch();
    QVBoxLayout *queueLayout = new QVBoxLayout(m_queuePanel);
    queueLayout->setContentsMargins(12, 8, 12, 4);
    queueLayout->setSpacing(4);
    queueLayout->addWidget(m_queueBanner);
    queueLayout->addWidget(m_queueList);
    queueLayout->addLayout(queueButtons);
    m_queuePanel->hide();

    QWidget *chatPane = new QWidget;
    QVBoxLayout *chatLayout = new QVBoxLayout(chatPane);
    chatLayout->setContentsMargins(0, 0, 0, 0);
    chatLayout->setSpacing(0);
    chatLayout->addWidget(header);
    chatLayout->addWidget(toolbar);
    chatLayout->addWidget(m_historyEdit, 3);
    chatLayout->addWidget(m_queuePanel);
    chatLayout->addWidget(composer);

    QWidget *profilePanel = new QWidget;
    profilePanel->setObjectName("profilePanel");
    profilePanel->setMinimumWidth(240);
    m_profileAvatar = new QLabel;
    m_profileAvatar->setFixedSize(64, 64);
    m_profileAvatar->setAlignment(Qt::AlignCenter);
    m_profileName = new QLabel;
    m_profileName->setObjectName("profileName");
    m_profileName->setAlignment(Qt::AlignCenter);
    m_profileName->setWordWrap(true);
    m_profileStatus = new QLabel;
    m_profileStatus->setObjectName("profileMeta");
    m_profileStatus->setAlignment(Qt::AlignCenter);
    m_profileSign = new QLabel;
    m_profileSign->setObjectName("profileSign");
    m_profileSign->setAlignment(Qt::AlignCenter);
    m_profileSign->setWordWrap(true);
    m_profileHost = new QLabel;
    m_profileLogin = new QLabel;
    m_profileGroup = new QLabel;
    m_profileIp = new QLabel;

    QWidget *profileForm = new QWidget;
    profileForm->setObjectName("profileForm");
    QFormLayout *form = new QFormLayout(profileForm);
    m_profileNick = new QLabel;
    form->setContentsMargins(8, 8, 8, 8);
    form->setLabelAlignment(Qt::AlignLeft);
    form->addRow(tr("Nickname:"), m_profileNick);
    form->addRow(tr("Host:"), m_profileHost);
    form->addRow(tr("Login:"), m_profileLogin);
    form->addRow(tr("Group:"), m_profileGroup);
    form->addRow(tr("IP address:"), m_profileIp);

    QLabel *profileTitle = new QLabel(tr("Peer profile"));
    profileTitle->setObjectName("profileName");
    profileTitle->setAlignment(Qt::AlignLeft);

    QVBoxLayout *profileLayout = new QVBoxLayout(profilePanel);
    profileLayout->setContentsMargins(16, 16, 16, 16);
    profileLayout->setSpacing(8);
    profileLayout->addWidget(profileTitle);
    profileLayout->addSpacing(8);
    profileLayout->addWidget(m_profileAvatar, 0, Qt::AlignHCenter);
    profileLayout->addWidget(m_profileName);
    profileLayout->addWidget(m_profileStatus);
    profileLayout->addWidget(m_profileSign);
    profileLayout->addSpacing(8);
    profileLayout->addWidget(profileForm);
    profileLayout->addStretch();

    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(chatPane);
    splitter->addWidget(profilePanel);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);
    splitter->setChildrenCollapsible(false);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(splitter);
    setLayout(mainLayout);

    m_inputEdit->setFocus();
}

void ChatWindow::refreshPeerCaption()
{
    const int row = Global::userManager->ipToRow(m_peerIp);
    const QString name = Global::userManager->displayNameForIp(m_peerIp);
    const QString host = row != -1 ? Global::userManager->host(row) : QString();
    const QString header = host.isEmpty() ? name : QString("%1(%2)").arg(name, host);
    m_peerLabel->setText(name);
    const QString sign = Global::userManager->signatureForIp(m_peerIp);
    QString meta = host.isEmpty() ? m_peerIp : QString("%1 · %2").arg(host, m_peerIp);
    if (!sign.isEmpty()) {
        meta += QString(" · %1").arg(sign);
    }
    m_peerMeta->setText(meta);
    m_headerAvatar->setPixmap(peerAvatar(name, 36));
    setWindowTitle(header);
    refreshProfile();
}

void ChatWindow::refreshProfile()
{
    const int row = Global::userManager->ipToRow(m_peerIp);
    const QString name = Global::userManager->displayNameForIp(m_peerIp);
    const QString real = Global::userManager->realNameForIp(m_peerIp);
    const int presence = Global::userManager->presenceForIp(m_peerIp);
    QString status = tr("Offline");
    if (presence == PRESENCE_ONLINE) {
        status = tr("Online");
    } else if (presence == PRESENCE_AWAY) {
        status = tr("Away");
    }
    m_profileName->setText(name);
    m_profileNick->setText(real.isEmpty() ? QString("-") : real);
    m_profileStatus->setText(status);
    const QString sign = Global::userManager->signatureForIp(m_peerIp);
    m_profileSign->setText(sign.isEmpty() ? tr("No signature") : sign);
    m_profileHost->setText(row != -1 && !Global::userManager->host(row).isEmpty()
                           ? Global::userManager->host(row) : QString("-"));
    m_profileLogin->setText(row != -1 && !Global::userManager->loginName(row).isEmpty()
                            ? Global::userManager->loginName(row) : QString("-"));
    m_profileGroup->setText(row != -1 && !Global::userManager->group(row).isEmpty()
                            ? Global::userManager->group(row) : QString("-"));
    m_profileIp->setText(m_peerIp);
    m_profileAvatar->setPixmap(peerAvatar(name, 64));
}

QPixmap ChatWindow::peerAvatar(const QString &name, int size) const
{
    return Helper::avatarPixmap(name, size, Helper::avatarColor(name),
            Helper::presenceColor(Global::userManager->presenceForIp(m_peerIp)));
}

void ChatWindow::showTransferMonitor()
{
    TransferFileWindow *window = new TransferFileWindow;
    window->show();
}

void ChatWindow::cancelAttachments()
{
    clearSendFiles();
}

void ChatWindow::loadHistory()
{
    if (m_historyLoaded) {
        return;
    }
    m_historyLoaded = true;

    m_historyEdit->clear();
    const QList<ChatRecord> records =
        Global::messageStore->loadHistory(m_peerIp, CHAT_HISTORY_LOAD_LIMIT);
    foreach (const ChatRecord &record, records) {
        const bool incoming = (record.direction != QLatin1String("out"));
        const QString name = incoming ? record.peerName : localName();
        appendMessage(record.timestamp, name, record.text, incoming,
                      record.imagePaths);
    }
}

void ChatWindow::setDraft(const QString &text)
{
    if (text.isEmpty()) {
        return;
    }
    m_inputEdit->setPlainText(text);
    m_inputEdit->moveCursor(QTextCursor::End);
    m_inputEdit->setFocus();
}

void ChatWindow::appendMessage(const QDateTime &when, const QString &name,
                               const QString &text, bool incoming,
                               const QStringList &imagePaths)
{
    const QString timeText = when.toString("yyyy-MM-dd hh:mm");
    QTextCursor cursor(m_historyEdit->document());
    cursor.movePosition(QTextCursor::End);

    QTextBlockFormat metaBlock;
    metaBlock.setTopMargin(10);
    metaBlock.setBottomMargin(2);
    metaBlock.setAlignment(incoming ? Qt::AlignLeft : Qt::AlignRight);
    cursor.insertBlock(metaBlock);

    QTextCharFormat metaFmt;
    metaFmt.setForeground(QColor("#8A8F98"));
    metaFmt.setFontPointSize(9);
    cursor.insertText(QString("%1  %2").arg(name, timeText), metaFmt);

    QTextTableFormat tableFormat;
    tableFormat.setBorder(0);
    tableFormat.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    tableFormat.setCellPadding(8);
    tableFormat.setCellSpacing(0);
    tableFormat.setAlignment(incoming ? Qt::AlignLeft : Qt::AlignRight);
    tableFormat.setLeftMargin(incoming ? 4 : 72);
    tableFormat.setRightMargin(incoming ? 72 : 4);

    QTextTable *table = cursor.insertTable(1, 1, tableFormat);
    QTextTableCell cell = table->cellAt(0, 0);
    QTextCharFormat cellFmt = cell.format();
    if (incoming) {
        cellFmt.setBackground(QColor("#FFFFFF"));
        cellFmt.setForeground(QColor("#1F2937"));
    } else {
        cellFmt.setBackground(QColor("#2563EB"));
        cellFmt.setForeground(QColor("#FFFFFF"));
    }
    cell.setFormat(cellFmt);

    QTextCursor cellCursor = cell.firstCursorPosition();
    QTextCharFormat textFmt;
    textFmt.setForeground(incoming ? QColor("#1F2937") : QColor("#FFFFFF"));
    textFmt.setFontPointSize(10);
    const QString displayText = EmojiPack::applyAliases(text);
    if (!displayText.isEmpty()) {
        cellCursor.insertText(displayText, textFmt);
    }

    QStringList paths = existingImagePaths(imagePaths);
    if (paths.isEmpty()) {
        paths = imagePathsFromText(text);
    }
    if (!paths.isEmpty()) {
        if (!displayText.isEmpty()) {
            cellCursor.insertBlock();
        }
        insertBubbleImages(cellCursor, paths);
    }

    cursor.movePosition(QTextCursor::End);
    m_historyEdit->setTextCursor(cursor);
    m_historyEdit->ensureCursorVisible();
}

QString ChatWindow::localName() const
{
    return Global::userManager->ourself().name();
}

bool ChatWindow::hasSendFile() const
{
    return m_sendFileModel.rowCount() > 0;
}

void ChatWindow::sendMessage()
{
    const QString text = m_inputEdit->toPlainText();
    if (text.trimmed().isEmpty() && !hasSendFile()) {
        return;
    }

    sendPayload(text, m_sendFileModel.pathList());
    m_inputEdit->clear();
    clearSendFiles();
    m_inputEdit->setFocus();
}

void ChatWindow::refreshQueuePanel()
{
    if (!m_queuePanel || !Global::offlineQueue) {
        return;
    }
    const QList<OfflineItem> items = Global::offlineQueue->itemsForIp(m_peerIp);
    m_queueList->clear();
    foreach (const OfflineItem &item, items) {
        QListWidgetItem *row = new QListWidgetItem(
                    QString("%1  %2")
                    .arg(item.created.toString("hh:mm:ss"), item.text));
        row->setData(Qt::UserRole, item.id);
        row->setToolTip(item.text);
        m_queueList->addItem(row);
    }
    if (items.isEmpty()) {
        m_queuePanel->hide();
        return;
    }
    m_queueBanner->setText(tr("Offline queue · %n message(s) — send when the peer comes online",
                              "", items.size()));
    m_queuePanel->show();
}

void ChatWindow::cancelSelectedQueued()
{
    if (!Global::offlineQueue) {
        return;
    }
    foreach (QListWidgetItem *row, m_queueList->selectedItems()) {
        Global::offlineQueue->removeById(row->data(Qt::UserRole).toString());
    }
}

void ChatWindow::clearQueued()
{
    if (!Global::offlineQueue) {
        return;
    }
    Global::offlineQueue->removeForIp(m_peerIp);
}

void ChatWindow::sendPayload(const QString &text, const QStringList &pathList)
{
    if (!Global::userManager->isReachable(m_peerIp)) {
        if (!pathList.isEmpty()) {
            QMessageBox::information(this, tr("Offline queue"),
                    tr("File attachments cannot be queued (no transfer server). "
                       "The text will be queued if present."));
        }
        const QString body = text;
        if (body.trimmed().isEmpty()) {
            return;
        }
        if (!Global::windowManager->queueOrDeliverText(m_peerIp, body, this)) {
            return;
        }
        refreshQueuePanel();
        return;
    }

    quint32 flags = IPMSG_SENDMSG | IPMSG_SENDCHECKOPT;
    QString additionalInfo = text;
    additionalInfo.append(QChar('\0'));

    SendFileMap *sendFileMap = 0;
    if (!pathList.isEmpty()) {
        sendFileMap = new SendFileMap;
        flags |= IPMSG_FILEATTACHOPT;
        sendFileMap->addFile(pathList);
        additionalInfo.append(sendFileMap->packetString());
        additionalInfo.append(QChar('\0'));
    }

    SendMsg sendMsg(QHostAddress(m_peerIp), IPMSG_DEFAULT_PORT,
                    additionalInfo, ""/* extendedInfo */, flags);
    Global::msgThread->addSendMsg(Msg(sendMsg));

    if (sendFileMap) {
        const int row = Global::userManager->ipToRow(m_peerIp);
        sendFileMap->setRecvUser(Global::userManager->displayNameForIp(m_peerIp));
        sendFileMap->setRecvHostname(
                    row != -1 ? Global::userManager->host(row) : QString());
        sendFileMap->setPacketNoString(sendMsg.packetNoString());
        Global::sendFileManager
                ->addTransferLocked(sendFileMap->packetNoString(), sendFileMap);
    }

    QString recorded = text;
    if (!pathList.isEmpty()) {
        if (!recorded.isEmpty()) {
            recorded.append('\n');
        }
        recorded.append(tr("[File attachment] %1")
                        .arg(pathList.size() == 1
                             ? QFileInfo(pathList.first()).fileName()
                             : Global::fileCountString(pathList.size())));
    }

    if (Global::preferences->isLogMsg && !recorded.isEmpty()) {
        logOutgoing(recorded);
    }

    if (!recorded.isEmpty()) {
        Global::windowManager->recordOutgoing(m_peerIp,
                Global::userManager->displayNameForIp(m_peerIp), recorded,
                imagePathsFromFiles(pathList));
    }
}

void ChatWindow::sendShake()
{
    if (!Global::userManager->isReachable(m_peerIp)) {
        QMessageBox::information(this, tr("Offline queue"),
                tr("Window shake is not queued for offline peers."));
        return;
    }
    quint32 flags = IPMSG_SENDMSG | IPMSG_SENDCHECKOPT;
    QString additionalInfo = Helper::shakeToken();
    additionalInfo.append(QChar('\0'));

    SendMsg sendMsg(QHostAddress(m_peerIp), IPMSG_DEFAULT_PORT,
                    additionalInfo, ""/* extendedInfo */, flags);
    Global::msgThread->addSendMsg(Msg(sendMsg));

    Global::windowManager->recordOutgoing(m_peerIp,
            Global::userManager->displayNameForIp(m_peerIp),
            tr("[Window shake]"));
    shakeWindow();
}

void ChatWindow::shakeWindow()
{
    Helper::shakeWidget(this);
}

void ChatWindow::captureFullScreen()
{
    sendScreenshot(ScreenCapture::capturePrimaryScreen(), "screen");
}

void ChatWindow::captureRegion()
{
    sendScreenshot(ScreenCapture::captureRegion(), "region");
}

void ChatWindow::sendScreenshot(const QPixmap &pixmap, const QString &prefix)
{
    if (pixmap.isNull()) {
        QMessageBox::information(this, tr("Screenshot"),
                tr("Could not capture the screen."));
        return;
    }
    const QString path = ScreenCapture::savePixmap(pixmap, prefix);
    if (path.isEmpty()) {
        QMessageBox::information(this, tr("Screenshot"),
                tr("Could not save the screenshot."));
        return;
    }
    sendPayload(tr("[Screenshot] %1").arg(QFileInfo(path).fileName()),
                QStringList() << path);
}

void ChatWindow::addSendFile()
{
    const QStringList pathList = QFileDialog::getOpenFileNames(
                this, tr("Select files to send"),
                Global::preferences->lastSendFilePath);
    if (pathList.isEmpty()) {
        return;
    }
    m_sendFileModel.addFile(pathList);
    Global::preferences->lastSendFilePath = pathList.at(0);
    updateFileCount();
}

void ChatWindow::addSendDir()
{
    const QString path = QFileDialog::getExistingDirectory(
                this, tr("Select folder to send"),
                Global::preferences->lastSendDirPath);
    if (path.isEmpty()) {
        return;
    }
    m_sendFileModel.addFile(path);
    Global::preferences->lastSendDirPath = path;
    updateFileCount();
}

void ChatWindow::showSendFileList()
{
    SendFileWindow *sendFileWindow = new SendFileWindow(&m_sendFileModel, this);
    connect(sendFileWindow, SIGNAL(addFileFinished()),
            this, SLOT(updateFileCount()));
    sendFileWindow->setWindowFlags(Qt::Dialog);
    sendFileWindow->setWindowModality(Qt::WindowModal);
    sendFileWindow->show();
}

void ChatWindow::updateFileCount()
{
    if (!hasSendFile()) {
        m_fileListButton->hide();
        return;
    }
    m_fileListButton->setText(tr("Show send file list (%1)")
                              .arg(Global::fileCountString(
                                       m_sendFileModel.rowCount())));
    m_fileListButton->show();
}

void ChatWindow::clearSendFiles()
{
    QList<int> rows;
    for (int i = 0; i < m_sendFileModel.rowCount(); ++i) {
        rows << i;
    }
    if (!rows.isEmpty()) {
        m_sendFileModel.removeRows(rows);
    }
    updateFileCount();
}

void ChatWindow::findInHistory()
{
    HistorySearchWindow *window = new HistorySearchWindow;
    window->show();
    window->raise();
    window->activateWindow();
}

void ChatWindow::addDroppedUrls(const QList<QUrl> &urls)
{
    foreach (const QUrl &url, urls) {
        if (url.scheme() == "file") {
            const QString filePath = url.toLocalFile();
            if (!filePath.isEmpty()) {
                m_sendFileModel.addFile(filePath);
            }
        }
    }
    updateFileCount();
}

void ChatWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void ChatWindow::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        addDroppedUrls(event->mimeData()->urls());
        event->acceptProposedAction();
    }
}

void ChatWindow::logOutgoing(const QString &text)
{
    if (!Global::preferences->logFile.isWritable()) {
        return;
    }

    const int row = Global::userManager->ipToRow(m_peerIp);
    QTextStream ts(&Global::preferences->logFile);
    QString s("=====================================\n");
    s.append(tr(" To: "));
    s.append(row != -1 ? Global::userManager->name(row)
                       : Global::userManager->displayNameForIp(m_peerIp));
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
        s.append(m_peerIp);
    }
    s.append(")\n");
    s.append(tr(" Time: "));
    s.append(QDateTime::currentDateTime().toString("yyyy-MM-dd dddd hh:mm:ss"));
    s.append("\n-------------------------------------\n");
    s.append(text);
    s.append("\n\n");
    ts << s << Qt::flush;
}

QStringList ChatWindow::imagePathsFromFiles(const QStringList &pathList) const
{
    QStringList images;
    foreach (const QString &path, pathList) {
        if (Helper::isImageFile(path) && QFileInfo::exists(path)) {
            images << QFileInfo(path).absoluteFilePath();
        }
    }
    return images;
}

QStringList ChatWindow::existingImagePaths(const QStringList &paths) const
{
    QStringList existing;
    foreach (const QString &path, paths) {
        const QString resolved = Helper::resolveImagePath(
                    path, Helper::imageSearchDirs());
        if (!resolved.isEmpty() && !existing.contains(resolved)) {
            existing << resolved;
        }
    }
    return existing;
}

QStringList ChatWindow::imagePathsFromText(const QString &text) const
{
    QStringList found;
    const QRegularExpression fileRe(
                QStringLiteral("(?:file://)?(/?[^\\s]+\\.(?:png|jpe?g|gif|bmp|webp))"),
                QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator it = fileRe.globalMatch(text);
    while (it.hasNext()) {
        const QString token = it.next().captured(1);
        const QString resolved = Helper::resolveImagePath(
                    token, Helper::imageSearchDirs());
        if (!resolved.isEmpty() && !found.contains(resolved)) {
            found << resolved;
        }
    }
    return found;
}

void ChatWindow::insertBubbleImages(QTextCursor &cursor, const QStringList &paths)
{
    QTextDocument *doc = m_historyEdit->document();
    foreach (const QString &path, paths) {
        const QPixmap thumb = Helper::imageThumbnail(
                    path, CHAT_IMAGE_THUMB_WIDTH, CHAT_IMAGE_THUMB_HEIGHT);
        if (thumb.isNull()) {
            continue;
        }

        const QString key = QString("qipmsg-img-%1").arg(qHash(path));
        doc->addResource(QTextDocument::ImageResource, QUrl(key), thumb);

        QTextImageFormat imageFormat;
        imageFormat.setName(key);
        imageFormat.setWidth(thumb.width());
        imageFormat.setHeight(thumb.height());
        imageFormat.setAnchor(true);
        imageFormat.setAnchorHref(QUrl::fromLocalFile(path).toString());
        cursor.insertImage(imageFormat);
        cursor.insertBlock();

        QTextCharFormat linkFmt;
        linkFmt.setAnchor(true);
        linkFmt.setAnchorHref(QUrl::fromLocalFile(path).toString());
        linkFmt.setForeground(QColor("#93C5FD"));
        linkFmt.setFontPointSize(9);
        cursor.insertText(QFileInfo(path).fileName(), linkFmt);
        cursor.insertBlock();
    }
}

void ChatWindow::openChatLink(const QUrl &url)
{
    QString path = url.toLocalFile();
    if (path.isEmpty()) {
        path = url.toString();
    }
    if (path.isEmpty()) {
        return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void ChatWindow::insertEmoji(const QString &glyph)
{
    if (glyph.isEmpty()) {
        return;
    }
    m_inputEdit->insertPlainText(glyph);
    m_inputEdit->setFocus();
}

void ChatWindow::showEmojiPicker()
{
    QMenu menu(this);
    menu.setObjectName("emojiMenu");

    QWidget *grid = new QWidget(&menu);
    QGridLayout *layout = new QGridLayout(grid);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(4);

    const QList<EmojiItem> items = EmojiPack::items();
    for (int i = 0; i < items.size(); ++i) {
        const EmojiItem item = items.at(i);
        QToolButton *button = new QToolButton(grid);
        button->setAutoRaise(true);
        button->setText(item.glyph);
        button->setToolTip(item.tip);
        button->setFixedSize(32, 32);
        QFont font = button->font();
        font.setPixelSize(18);
        button->setFont(font);
        connect(button, &QToolButton::clicked, this, [this, &menu, item]() {
            insertEmoji(item.glyph);
            menu.close();
        });
        layout->addWidget(button, i / 8, i % 8);
    }

    QWidgetAction *action = new QWidgetAction(&menu);
    action->setDefaultWidget(grid);
    menu.addAction(action);

    QWidget *senderWidget = qobject_cast<QWidget *>(sender());
    const QPoint pos = senderWidget
            ? senderWidget->mapToGlobal(QPoint(0, senderWidget->height()))
            : QCursor::pos();
    menu.exec(pos);
}

bool ChatWindow::pasteClipboardImage()
{
    const QMimeData *mime = QApplication::clipboard()->mimeData();
    if (!mime) {
        return false;
    }

    if (mime->hasImage()) {
        const QPixmap pixmap = qvariant_cast<QPixmap>(mime->imageData());
        if (pixmap.isNull()) {
            return false;
        }
        const QString path = ScreenCapture::savePixmap(pixmap, "paste");
        if (path.isEmpty()) {
            return false;
        }
        m_sendFileModel.addFile(path);
        updateFileCount();
        return true;
    }

    if (mime->hasUrls()) {
        bool added = false;
        foreach (const QUrl &url, mime->urls()) {
            const QString path = url.toLocalFile();
            if (!path.isEmpty() && Helper::isImageFile(path)
                    && QFileInfo::exists(path)) {
                m_sendFileModel.addFile(path);
                added = true;
            }
        }
        if (added) {
            updateFileCount();
            return true;
        }
    }

    return false;
}

bool ChatWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_inputEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->matches(QKeySequence::Paste) && pasteClipboardImage()) {
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void ChatWindow::closeEvent(QCloseEvent *event)
{
    hide();
    event->ignore();
}

void ChatWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    refreshPeerCaption();
    emit chatShown(m_peerIp);
}
