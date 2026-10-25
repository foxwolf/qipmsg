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
#include "shareBrowser.h"
#include "constants.h"
#include "emoji_pack.h"
#include "stickerPicker.h"
#include "global.h"
#include "helper.h"
#include "history_search_window.h"
#include "ipmsg.h"
#include "message_store.h"
#include "msg.h"
#include "msg_thread.h"
#include "preferences.h"
#include "screen_capture.h"
#include "shotEditor.h"
#include "avatarStore.h"
#include "send_file_manager.h"
#include "send_file_map.h"
#include "send_file_window.h"
#include "send_msg.h"
#include "offline_queue.h"
#include "msgCrypto.h"
#include "user_manager.h"
#include "window_manager.h"

#include "transfer_file_window.h"

#include <QApplication>
#include <QCheckBox>
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
#include <QTextBlock>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextFormat>
#include <QTextDocument>
#include <QVariant>
#include <QTextEdit>
#include <QTextImageFormat>
#include <QTextTable>
#include <QDateTime>
#include <QTextTableCell>
#include <QTimer>
#include <QTextTableFormat>
#include <QToolButton>
#include <QUrl>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <QWidgetAction>

ChatWindow::ChatWindow(const QString &ip, QWidget *parent)
    : QWidget(parent), m_peerIp(ip), m_historyLoaded(false),
      m_peerTyping(false), m_localTyping(false), m_lastTypingSentMs(0),
      m_typingIdle(0)
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
    if (Global::avatarStore) {
        connect(Global::avatarStore, &AvatarStore::changed,
                this, &ChatWindow::refreshPeerCaption);
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
    toolLayout->addWidget(makeToolButton(tr("Browse peer share"),
                                         tr("List a qipmsg shared folder. FeiQ share has no verified packet format."),
                                         SLOT(browsePeerShare())));
    QToolButton *shot = makeToolButton(tr("Screenshot"),
                                       tr("Screenshot"), 0);
    QMenu *shotMenu = new QMenu(shot);
    shotMenu->addAction(tr("Full screen"), this, SLOT(captureFullScreen()));
    shotMenu->addAction(tr("Region"), this, SLOT(captureRegion()));
    shot->setMenu(shotMenu);
    shot->setPopupMode(QToolButton::InstantPopup);
    toolLayout->addWidget(shot);
    toolLayout->addWidget(makeToolButton(tr("Emoji"),
                                         tr("Emoji and custom stickers"),
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
    connect(m_inputEdit, SIGNAL(textChanged()),
            this, SLOT(composerChanged()));
    m_typingIdle = new QTimer(this);
    m_typingIdle->setSingleShot(true);
    connect(m_typingIdle, SIGNAL(timeout()), this, SLOT(typingIdle()));

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

    m_encapCheck = new QCheckBox(tr("&Encap"));
    m_encapCheck->setToolTip(tr("Send a sealed envelope (hidden until the peer opens it)"));
    if (Global::preferences->isSealed) {
        m_encapCheck->setChecked(true);
    }

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_encapCheck);
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
    m_localPhotoButton = new QPushButton(tr("Local photo..."));
    m_localPhotoButton->setToolTip(tr(
            "PNG or JPEG shown for this IP on this computer only. "
            "FeiQ peers still see a color block."));
    m_clearPhotoButton = new QPushButton(tr("Clear local photo"));
    connect(m_localPhotoButton, SIGNAL(clicked()),
            this, SLOT(setLocalPhoto()));
    connect(m_clearPhotoButton, SIGNAL(clicked()),
            this, SLOT(clearLocalPhoto()));
    profileLayout->addWidget(m_localPhotoButton);
    profileLayout->addWidget(m_clearPhotoButton);
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
    if (m_peerTyping) {
        meta += QString(" · %1").arg(tr("Typing…"));
    }
    if (Global::preferences && Global::preferences->isEncryptMsg
            && MsgCrypto::hasPeerKey(m_peerIp)) {
        meta += QString(" · %1").arg(tr("Key ready"));
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
    if (m_peerTyping) {
        status = tr("Typing…");
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
    const QColor status = Helper::presenceColor(
                Global::userManager->presenceForIp(m_peerIp));
    if (Global::avatarStore) {
        const QPixmap photo = Global::avatarStore->rounded(m_peerIp, size, status);
        if (!photo.isNull()) {
            return photo;
        }
    }
    return Helper::avatarPixmap(name, size, Helper::avatarColor(name), status);
}

void ChatWindow::setLocalPhoto()
{
    if (Global::avatarStore) {
        Global::avatarStore->chooseAndSet(m_peerIp, this);
    }
}

void ChatWindow::clearLocalPhoto()
{
    if (Global::avatarStore) {
        Global::avatarStore->clearKey(m_peerIp);
    }
}

void ChatWindow::browsePeerShare()
{
    ShareBrowser dialog(m_peerIp, this);
    dialog.exec();
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
                      record.imagePaths, record.packetNo, record.expectRead,
                      record.readReceipt, record.encrypted);
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
                               const QStringList &imagePaths,
                               const QString &packetNo,
                               bool expectRead,
                               bool readReceipt,
                               bool encrypted)
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
    metaFmt.setForeground(Helper::bubbleMetaFg());
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
        cellFmt.setBackground(Helper::bubbleIncomingBg());
        cellFmt.setForeground(Helper::bubbleIncomingFg());
    } else {
        cellFmt.setBackground(Helper::bubbleOutgoingBg());
        cellFmt.setForeground(Helper::bubbleOutgoingFg());
    }
    cell.setFormat(cellFmt);

    QTextCursor cellCursor = cell.firstCursorPosition();
    QTextCharFormat textFmt;
    textFmt.setForeground(incoming ? Helper::bubbleIncomingFg()
                                   : Helper::bubbleOutgoingFg());
    textFmt.setFontPointSize(10);
    if (encrypted) {
        QTextCharFormat lockFmt = textFmt;
        lockFmt.setFontPointSize(8);
        lockFmt.setFontWeight(QFont::Bold);
        lockFmt.setForeground(QColor("#0F766E"));
        cellCursor.insertText(tr("Encrypted"), lockFmt);
        if (!text.isEmpty()) {
            cellCursor.insertText(QStringLiteral("\n"), textFmt);
        }
    }
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
    if (!incoming && !packetNo.isEmpty()) {
        QTextBlockFormat statusBlock;
        statusBlock.setAlignment(Qt::AlignRight);
        statusBlock.setTopMargin(0);
        statusBlock.setBottomMargin(2);
        statusBlock.setRightMargin(8);
        cursor.insertBlock(statusBlock);

        QTextCharFormat statusFmt;
        statusFmt.setFontPointSize(8);
        statusFmt.setProperty(QTextFormat::UserProperty + 21, packetNo);
        QString statusText;
        if (readReceipt) {
            statusFmt.setForeground(QColor("#059669"));
            statusText = tr("Read");
        } else if (expectRead) {
            statusFmt.setForeground(QColor("#9CA3AF"));
            statusText = tr("Unread");
        } else {
            statusFmt.setForeground(QColor("#9CA3AF"));
            statusText = QString(QChar(0x200B));
        }
        cursor.insertText(statusText, statusFmt);
    }
    cursor.movePosition(QTextCursor::End);
    m_historyEdit->setTextCursor(cursor);
    m_historyEdit->ensureCursorVisible();
}

bool ChatWindow::markPacketRead(const QString &packetNo)
{
    if (packetNo.isEmpty() || !m_historyEdit) {
        return false;
    }
    QTextDocument *doc = m_historyEdit->document();
    for (QTextBlock block = doc->begin(); block.isValid(); block = block.next()) {
        for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            if (!frag.isValid()) {
                continue;
            }
            const QString marked = frag.charFormat()
                    .property(QTextFormat::UserProperty + 21).toString();
            if (marked != packetNo) {
                continue;
            }
            QTextCursor edit(doc);
            edit.setPosition(frag.position());
            edit.setPosition(frag.position() + frag.length(),
                             QTextCursor::KeepAnchor);
            QTextCharFormat fmt = frag.charFormat();
            fmt.setForeground(QColor("#059669"));
            fmt.setProperty(QTextFormat::UserProperty + 21, packetNo);
            edit.insertText(tr("Read"), fmt);
            return true;
        }
    }
    return false;
}

QString ChatWindow::sealedHref(const QString &packetNo) const
{
    QUrl url;
    url.setScheme(QStringLiteral("qipmsg"));
    url.setHost(QStringLiteral("open-seal"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("ip"), m_peerIp);
    query.addQueryItem(QStringLiteral("pkt"), packetNo);
    url.setQuery(query);
    return url.toString();
}

void ChatWindow::appendSealedLetter(const QString &packetNo, const QString &name)
{
    const QString timeText = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm");
    QTextCursor cursor(m_historyEdit->document());
    cursor.movePosition(QTextCursor::End);

    QTextBlockFormat metaBlock;
    metaBlock.setTopMargin(10);
    metaBlock.setBottomMargin(2);
    metaBlock.setAlignment(Qt::AlignLeft);
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
    tableFormat.setAlignment(Qt::AlignLeft);
    tableFormat.setLeftMargin(4);
    tableFormat.setRightMargin(72);

    QTextTable *table = cursor.insertTable(1, 1, tableFormat);
    QTextTableCell cell = table->cellAt(0, 0);
    QTextCharFormat cellFmt = cell.format();
    cellFmt.setBackground(QColor("#FFF7ED"));
    cellFmt.setForeground(QColor("#9A3412"));
    cell.setFormat(cellFmt);

    QTextCursor cellCursor = cell.firstCursorPosition();
    QTextCharFormat linkFmt;
    linkFmt.setAnchor(true);
    linkFmt.setAnchorHref(sealedHref(packetNo));
    linkFmt.setForeground(QColor("#9A3412"));
    linkFmt.setFontUnderline(true);
    linkFmt.setFontPointSize(10);
    cellCursor.insertText(tr("Sealed letter — click to open"), linkFmt);

    cursor.movePosition(QTextCursor::End);
    m_historyEdit->setTextCursor(cursor);
    m_historyEdit->ensureCursorVisible();
}

void ChatWindow::markSealedOpened(const QString &packetNo)
{
    const QString href = sealedHref(packetNo);
    QTextDocument *doc = m_historyEdit->document();
    for (QTextBlock block = doc->begin(); block.isValid(); block = block.next()) {
        for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            if (!frag.isValid() || frag.charFormat().anchorHref() != href) {
                continue;
            }
            QTextCursor edit(doc);
            edit.setPosition(frag.position());
            edit.setPosition(frag.position() + frag.length(), QTextCursor::KeepAnchor);
            QTextCharFormat fmt = frag.charFormat();
            fmt.setAnchor(false);
            fmt.setAnchorHref(QString());
            fmt.setFontUnderline(false);
            fmt.setForeground(QColor("#6B7280"));
            edit.insertText(tr("Envelope opened"), fmt);
            return;
        }
    }
}

QString ChatWindow::localName() const
{
    return Global::userManager->ourself().name();
}

bool ChatWindow::hasSendFile() const
{
    return m_sendFileModel.rowCount() > 0;
}

void ChatWindow::setPeerTyping(bool typing)
{
    if (m_peerTyping == typing) {
        return;
    }
    m_peerTyping = typing;
    refreshPeerCaption();
}

void ChatWindow::composerChanged()
{
    if (!m_inputEdit || !m_typingIdle) {
        return;
    }
    const bool hasText = !m_inputEdit->toPlainText().isEmpty();
    if (!hasText) {
        m_typingIdle->stop();
        stopLocalTyping();
        return;
    }
    if (!Global::preferences || !Global::preferences->isShareTyping) {
        m_typingIdle->stop();
        stopLocalTyping();
        return;
    }
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (!m_localTyping || now - m_lastTypingSentMs >= 2000) {
        m_localTyping = true;
        m_lastTypingSentMs = now;
        publishTyping(true);
    }
    m_typingIdle->start(2000);
}

void ChatWindow::typingIdle()
{
    stopLocalTyping();
}

void ChatWindow::stopLocalTyping()
{
    if (!m_localTyping) {
        return;
    }
    m_localTyping = false;
    publishTyping(false);
}

void ChatWindow::publishTyping(bool active)
{
    if (!Global::msgThread || !Global::userManager || m_peerIp.isEmpty()) {
        return;
    }
    if (active && (!Global::preferences || !Global::preferences->isShareTyping)) {
        return;
    }
    const quint32 flags = active ? IPMSG_FEIQ_TYPING : IPMSG_FEIQ_TYPINGSTOP;
    SendMsg sendMsg(QHostAddress(m_peerIp),
                    Global::userManager->sendPortForIp(m_peerIp),
                    QString(), QString(), flags);
    Global::msgThread->addSendMsg(Msg(sendMsg));
}

void ChatWindow::sendMessage()
{
    stopLocalTyping();
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
    const bool expectRead = m_encapCheck && m_encapCheck->isChecked();
    if (expectRead) {
        flags |= IPMSG_SECRETOPT | IPMSG_READCHECKOPT;
    }
    flags = Global::userManager->sendFlagsForIp(m_peerIp, flags);
    QString additionalInfo = text;
    additionalInfo.append(QChar('\0'));

    SendFileMap *sendFileMap = 0;
    if (!pathList.isEmpty()) {
        sendFileMap = new SendFileMap;
        flags |= IPMSG_FILEATTACHOPT;
        sendFileMap->addFile(pathList);
        const bool withHash = Global::userManager
                && Global::userManager->peerIsQipmsg(m_peerIp);
        additionalInfo.append(sendFileMap->packetString(withHash));
        additionalInfo.append(QChar('\0'));
    }

    const bool encrypted = MsgCrypto::prepareOutgoing(m_peerIp, &additionalInfo, &flags);

    SendMsg sendMsg(QHostAddress(m_peerIp),
                    Global::userManager->sendPortForIp(m_peerIp),
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
                imagePathsFromFiles(pathList),
                sendMsg.packetNoString(), expectRead, encrypted);
    }
}

void ChatWindow::sendShake()
{
    if (!Global::userManager->isReachable(m_peerIp)) {
        QMessageBox::information(this, tr("Offline queue"),
                tr("Window shake is not queued for offline peers."));
        return;
    }
    quint32 flags = Global::userManager->sendFlagsForIp(
                m_peerIp, IPMSG_SENDMSG | IPMSG_SENDCHECKOPT);
    QString additionalInfo = Helper::shakeToken();
    additionalInfo.append(QChar('\0'));
    const bool encrypted = MsgCrypto::prepareOutgoing(m_peerIp, &additionalInfo, &flags);

    SendMsg sendMsg(QHostAddress(m_peerIp),
                    Global::userManager->sendPortForIp(m_peerIp),
                    additionalInfo, ""/* extendedInfo */, flags);
    Global::msgThread->addSendMsg(Msg(sendMsg));

    Global::windowManager->recordOutgoing(m_peerIp,
            Global::userManager->displayNameForIp(m_peerIp),
            tr("[Window shake]"), QStringList(), sendMsg.packetNoString(),
            false, encrypted);
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
    bool cancelled = false;
    const QPixmap edited = ShotEditor::edit(pixmap, &cancelled, this);
    if (cancelled) {
        return;
    }
    if (edited.isNull()) {
        QMessageBox::information(this, tr("Screenshot"),
                tr("Could not capture the screen."));
        return;
    }
    const QString path = ScreenCapture::savePixmap(edited, prefix);
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
    if (url.scheme() == QLatin1String("qipmsg")) {
        if (url.host() == QLatin1String("open-seal") && Global::windowManager) {
            const QUrlQuery query(url);
            Global::windowManager->openSealedLetter(
                        query.queryItemValue(QStringLiteral("ip")),
                        query.queryItemValue(QStringLiteral("pkt")));
        }
        return;
    }
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

void ChatWindow::sendSticker(const QString &path, const QString &title)
{
    if (path.isEmpty() || !QFileInfo::exists(path)) {
        return;
    }
    QString caption = title.trimmed();
    if (caption.isEmpty()) {
        caption = QFileInfo(path).completeBaseName();
    }
    sendPayload(tr("[Sticker] %1").arg(caption), QStringList() << path);
}

void ChatWindow::showEmojiPicker()
{
    StickerPicker picker(this);
    connect(&picker, &StickerPicker::unicodeChosen, this,
            [this](const QString &glyph) {
        insertEmoji(glyph);
    });
    connect(&picker, &StickerPicker::stickerChosen, this,
            [this](const QString &path, const QString &title) {
        sendSticker(path, title);
    });
    QWidget *anchor = qobject_cast<QWidget *>(sender());
    if (anchor) {
        const QPoint pos = anchor->mapToGlobal(QPoint(0, anchor->height()));
        picker.move(pos);
    }
    picker.exec();
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
