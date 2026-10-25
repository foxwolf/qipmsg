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
#include "avatarFetch.h"
#include "doodleCanvas.h"
#include "favoriteDialog.h"
#include "zoneBrowser.h"
#include "winShare.h"
#include "send_file_manager.h"
#include "send_file_map.h"
#include "send_file_window.h"
#include "send_msg.h"
#include "offline_queue.h"
#include "msgCrypto.h"
#include "richText.h"
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
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QFormLayout>
#include <QGridLayout>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QHideEvent>
#include <QHostAddress>
#include <QIcon>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QScreen>
#include <QSettings>
#include <QShortcut>
#include <QShowEvent>
#include <QSizePolicy>
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

namespace {

enum ToolGlyph {
    toolGlyphFile = 1,
    toolGlyphFolder,
    toolGlyphShot,
    toolGlyphEmoji,
    toolGlyphShake,
    toolGlyphFind,
    toolGlyphMore,
    toolGlyphProfile
};

const int kChatMinWidth = 560;
const int kChatMinHeight = 400;
const int kProfilePaneWidth = 220;

QIcon toolGlyphIcon(ToolGlyph glyph, const QColor &ink)
{
    QPixmap pix(16, 16);
    pix.fill(Qt::transparent);
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen pen(ink, 1.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    switch (glyph) {
    case toolGlyphFile:
        painter.drawRoundedRect(QRectF(3.5, 2.2, 9, 11.6), 1.2, 1.2);
        painter.drawLine(QPointF(5.6, 6), QPointF(10.4, 6));
        painter.drawLine(QPointF(5.6, 8.4), QPointF(10.4, 8.4));
        painter.drawLine(QPointF(5.6, 10.8), QPointF(9, 10.8));
        break;
    case toolGlyphFolder: {
        QPainterPath path;
        path.moveTo(2.2, 6);
        path.lineTo(2.2, 13);
        path.lineTo(13.8, 13);
        path.lineTo(13.8, 5.2);
        path.lineTo(8, 5.2);
        path.lineTo(6.6, 3.2);
        path.lineTo(2.2, 3.2);
        path.closeSubpath();
        painter.drawPath(path);
        break;
    }
    case toolGlyphShot:
        painter.drawRoundedRect(QRectF(2.2, 4.2, 11.6, 8.2), 1.3, 1.3);
        painter.drawLine(QPointF(5, 4.2), QPointF(6.6, 2.6));
        painter.drawLine(QPointF(6.6, 2.6), QPointF(9.4, 2.6));
        painter.drawLine(QPointF(9.4, 2.6), QPointF(11, 4.2));
        painter.drawEllipse(QPointF(8, 8.3), 2.1, 2.1);
        break;
    case toolGlyphEmoji:
        painter.drawEllipse(QRectF(2.4, 2.4, 11.2, 11.2));
        painter.setBrush(ink);
        painter.drawEllipse(QPointF(6.2, 7.1), 0.7, 0.7);
        painter.drawEllipse(QPointF(9.8, 7.1), 0.7, 0.7);
        painter.setBrush(Qt::NoBrush);
        painter.drawArc(QRectF(5, 7.4, 6, 4.2), 210 * 16, 120 * 16);
        break;
    case toolGlyphShake:
        painter.drawLine(QPointF(3, 11), QPointF(6, 4));
        painter.drawLine(QPointF(6, 4), QPointF(8, 10));
        painter.drawLine(QPointF(8, 10), QPointF(11, 3));
        painter.drawLine(QPointF(11, 3), QPointF(13.2, 8));
        break;
    case toolGlyphFind:
        painter.drawEllipse(QRectF(2.4, 2.4, 7.6, 7.6));
        painter.drawLine(QPointF(9.2, 9.2), QPointF(13.2, 13.2));
        break;
    case toolGlyphMore:
        painter.setBrush(ink);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(3.4, 8), 1.15, 1.15);
        painter.drawEllipse(QPointF(8, 8), 1.15, 1.15);
        painter.drawEllipse(QPointF(12.6, 8), 1.15, 1.15);
        break;
    case toolGlyphProfile:
        painter.drawEllipse(QRectF(5.4, 2.2, 5.2, 5.2));
        painter.drawArc(QRectF(3.2, 8.2, 9.6, 8), 15 * 16, 150 * 16);
        break;
    default:
        break;
    }
    painter.end();
    return QIcon(pix);
}

void attachGlyph(QToolButton *button, ToolGlyph glyph)
{
    button->setProperty("toolGlyph", int(glyph));
    button->setIconSize(QSize(16, 16));
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
}

QWidget *toolSeparator()
{
    QFrame *line = new QFrame;
    line->setObjectName(QStringLiteral("chatToolSep"));
    line->setFixedWidth(1);
    line->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    return line;
}

QLabel *toolGroupLabel(const QString &text)
{
    QLabel *label = new QLabel(text);
    label->setObjectName(QStringLiteral("chatToolGroup"));
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    return label;
}

void addMenuCaption(QMenu *menu, const QString &text)
{
    QLabel *label = new QLabel(text, menu);
    label->setObjectName(QStringLiteral("menuCaption"));
    label->setAttribute(Qt::WA_TransparentForMouseEvents);
    QWidgetAction *action = new QWidgetAction(menu);
    action->setDefaultWidget(label);
    menu->addAction(action);
}

QSize chatSizeWithinScreen(const QSize &want)
{
    QSize out = want;
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        return out;
    }
    const QSize avail = screen->availableGeometry().size();
    if (avail.width() > 0) {
        const int lower = qMin(kChatMinWidth, avail.width());
        out.setWidth(qBound(lower, out.width(), avail.width()));
    }
    if (avail.height() > 0) {
        const int lower = qMin(kChatMinHeight, avail.height());
        out.setHeight(qBound(lower, out.height(), avail.height()));
    }
    return out;
}

} // namespace

ChatWindow::ChatWindow(const QString &ip, QWidget *parent)
    : QWidget(parent), m_peerIp(ip), m_toolbar(0), m_profilePanel(0),
      m_profileToggle(0), m_splitter(0), m_historyLoaded(false),
      m_peerTyping(false), m_localTyping(false), m_layoutReady(false),
      m_applyingLayout(false), m_lastTypingSentMs(0), m_typingIdle(0)
{
    setAcceptDrops(true);
    createUi();
    restoreChatLayout();
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

ChatWindow::~ChatWindow()
{
    saveChatLayout();
    if (Global::settings && m_layoutReady) {
        Global::settings->sync();
    }
}

QSize ChatWindow::sizeHint() const
{
    if (m_profilePanel && m_profilePanel->isVisible()) {
        return QSize(900, 520);
    }
    return QSize(680, 520);
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
    button->setFocusPolicy(Qt::NoFocus);
    button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    if (enabled && slot) {
        connect(button, SIGNAL(clicked()), this, slot);
    }
    return button;
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
    headerLayout->setContentsMargins(14, 8, 10, 8);
    headerLayout->setSpacing(10);
    headerLayout->addWidget(m_headerAvatar);
    headerLayout->addLayout(headerText, 1);
    m_profileToggle = new QToolButton;
    m_profileToggle->setObjectName(QStringLiteral("profileToggle"));
    m_profileToggle->setText(tr("Profile"));
    m_profileToggle->setToolTip(tr("Show or hide the peer profile"));
    m_profileToggle->setCheckable(true);
    m_profileToggle->setChecked(true);
    m_profileToggle->setAutoRaise(true);
    m_profileToggle->setFocusPolicy(Qt::NoFocus);
    attachGlyph(m_profileToggle, toolGlyphProfile);
    connect(m_profileToggle, SIGNAL(toggled(bool)),
            this, SLOT(toggleProfile(bool)));
    headerLayout->addWidget(m_profileToggle);

    m_toolbar = new QWidget;
    m_toolbar->setObjectName(QStringLiteral("chatToolbar"));
    QHBoxLayout *toolLayout = new QHBoxLayout(m_toolbar);
    toolLayout->setContentsMargins(6, 2, 6, 2);
    toolLayout->setSpacing(1);
    toolLayout->addWidget(toolGroupLabel(tr("Transfer")));
    QToolButton *fileButton = makeToolButton(tr("Send file"),
                                             tr("Send file"),
                                             SLOT(addSendFile()));
    attachGlyph(fileButton, toolGlyphFile);
    toolLayout->addWidget(fileButton);
    QToolButton *folderButton = makeToolButton(tr("Send folder"),
                                               tr("Send folder"),
                                               SLOT(addSendDir()));
    attachGlyph(folderButton, toolGlyphFolder);
    toolLayout->addWidget(folderButton);
    toolLayout->addWidget(toolSeparator());
    toolLayout->addWidget(toolGroupLabel(tr("Chat")));
    QToolButton *shot = makeToolButton(tr("Screenshot"),
                                       tr("Full screen or a region"), 0);
    QMenu *shotMenu = new QMenu(shot);
    shotMenu->setToolTipsVisible(true);
    shotMenu->addAction(tr("Full screen"), this, SLOT(captureFullScreen()));
    shotMenu->addAction(tr("Region"), this, SLOT(captureRegion()));
    shot->setMenu(shotMenu);
    shot->setPopupMode(QToolButton::InstantPopup);
    attachGlyph(shot, toolGlyphShot);
    toolLayout->addWidget(shot);
    QToolButton *emojiButton = makeToolButton(tr("Emoji"),
                                              tr("Emoji and custom stickers"),
                                              SLOT(showEmojiPicker()));
    attachGlyph(emojiButton, toolGlyphEmoji);
    toolLayout->addWidget(emojiButton);
    QToolButton *shakeButton = makeToolButton(tr("Shake"),
                                              tr("Window shake"),
                                              SLOT(sendShake()));
    attachGlyph(shakeButton, toolGlyphShake);
    toolLayout->addWidget(shakeButton);
    QToolButton *findButton = makeToolButton(tr("Find"),
                                             tr("Find in history"),
                                             SLOT(findInHistory()));
    attachGlyph(findButton, toolGlyphFind);
    toolLayout->addWidget(findButton);
    toolLayout->addWidget(toolSeparator());

    QToolButton *moreButton = makeToolButton(tr("More"),
                                             tr("More actions"), 0);
    QMenu *moreMenu = new QMenu(moreButton);
    moreMenu->setObjectName(QStringLiteral("chatMoreMenu"));
    moreMenu->setToolTipsVisible(true);
    addMenuCaption(moreMenu, tr("Transfer"));
    QAction *browseShare = moreMenu->addAction(tr("Browse peer share"),
                                               this, SLOT(browsePeerShare()));
    browseShare->setToolTip(tr(
            "List a qipmsg shared folder. FeiQ share has no verified packet format."));
    QAction *groupFolder = moreMenu->addAction(tr("Group folder"),
                                               this, SLOT(browseGroupShare()));
    groupFolder->setToolTip(tr(
            "Request files from a discussion folder this peer published."));
    addMenuCaption(moreMenu, tr("Chat extras"));
    QAction *doodle = moreMenu->addAction(tr("Doodle"), this, SLOT(openDoodle()));
    doodle->setToolTip(tr("Draw a picture and send it as an image attachment."));
    QAction *favorites = moreMenu->addAction(tr("Favorites"),
                                             this, SLOT(showFavorites()));
    favorites->setToolTip(tr(
            "Save chat text, a file path, or a note about this peer on this computer."));
    QAction *zone = moreMenu->addAction(tr("Zone"), this, SLOT(showZone()));
    zone->setToolTip(tr(
            "Open local notes under the zone folder. This is not a FeiQ zone database."));
    QAction *logAction = moreMenu->addAction(tr("Log"), this, SLOT(showZone()));
    logAction->setToolTip(tr(
            "Open the same local note folder. This is not a FeiQ zone log and not the message log file."));
    addMenuCaption(moreMenu, tr("Not implemented yet"));
    QAction *remoteAssist = moreMenu->addAction(tr("Remote assist"));
    QAction *remoteControl = moreMenu->addAction(tr("Remote control"));
    QAction *voiceCall = moreMenu->addAction(tr("Voice call"));
    remoteAssist->setEnabled(false);
    remoteControl->setEnabled(false);
    voiceCall->setEnabled(false);
    remoteAssist->setToolTip(tr("Not implemented yet"));
    remoteControl->setToolTip(tr("Not implemented yet"));
    voiceCall->setToolTip(tr("Not implemented yet"));
    moreButton->setMenu(moreMenu);
    moreButton->setPopupMode(QToolButton::InstantPopup);
    attachGlyph(moreButton, toolGlyphMore);
    toolLayout->addWidget(moreButton);
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

    m_inputEdit = new RichComposer;
    m_inputEdit->setObjectName("composerEdit");
    m_inputEdit->setTabChangesFocus(true);
    m_inputEdit->setMaximumHeight(140);
    m_inputEdit->setPlaceholderText(tr("Type a message..."));
    RichComposer *composerEdit = static_cast<RichComposer *>(m_inputEdit);
    connect(composerEdit, &RichComposer::imagePasted,
            this, [this](const QPixmap &pixmap) { attachPastedPixmap(pixmap); });
    connect(composerEdit, &RichComposer::imageFilesPasted,
            this, [this](const QStringList &paths) { attachPastedImageFiles(paths); });
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
    composerLayout->addWidget(new RichComposerBar(m_inputEdit, composer));
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
    chatLayout->addWidget(m_toolbar);
    chatLayout->addWidget(m_historyEdit, 3);
    chatLayout->addWidget(m_queuePanel);
    chatLayout->addWidget(composer);

    m_profilePanel = new QWidget;
    m_profilePanel->setObjectName("profilePanel");
    m_profilePanel->setMinimumWidth(0);
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
    m_profileNick = new QLabel;
    m_profileHost->setWordWrap(true);
    m_profileLogin->setWordWrap(true);
    m_profileGroup->setWordWrap(true);
    m_profileIp->setWordWrap(true);
    m_profileNick->setWordWrap(true);

    QWidget *profileForm = new QWidget;
    profileForm->setObjectName("profileForm");
    QFormLayout *form = new QFormLayout(profileForm);
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

    QVBoxLayout *profileLayout = new QVBoxLayout(m_profilePanel);
    profileLayout->setContentsMargins(12, 12, 12, 12);
    profileLayout->setSpacing(6);
    profileLayout->addWidget(profileTitle);
    profileLayout->addSpacing(8);
    profileLayout->addWidget(m_profileAvatar, 0, Qt::AlignHCenter);
    m_photoStatus = new QLabel;
    m_photoStatus->setObjectName("profileMeta");
    m_photoStatus->setWordWrap(true);
    m_photoStatus->setAlignment(Qt::AlignCenter);
    m_localPhotoButton = new QPushButton(tr("Local photo..."));
    m_localPhotoButton->setToolTip(tr(
            "PNG or JPEG for this IP on this computer. It overrides a photo "
            "fetched from another qipmsg. FeiQ still has no photo packet."));
    m_clearPhotoButton = new QPushButton(tr("Clear local photo"));
    m_clearPhotoButton->setToolTip(tr(
            "Remove the photo set on this computer. A fetched qipmsg photo "
            "can show again. If only a fetched photo is saved, it stays "
            "hidden until that peer publishes a different one."));
    connect(m_localPhotoButton, SIGNAL(clicked()),
            this, SLOT(setLocalPhoto()));
    connect(m_clearPhotoButton, SIGNAL(clicked()),
            this, SLOT(clearLocalPhoto()));
    m_shareButton = new QPushButton(tr("Network neighborhood"));
    connect(m_shareButton, SIGNAL(clicked()),
            this, SLOT(openNetworkNeighborhood()));
    profileLayout->addWidget(m_photoStatus);
    profileLayout->addWidget(m_localPhotoButton);
    profileLayout->addWidget(m_clearPhotoButton);
    profileLayout->addWidget(m_shareButton);
    profileLayout->addWidget(m_profileName);
    profileLayout->addWidget(m_profileStatus);
    profileLayout->addWidget(m_profileSign);
    profileLayout->addSpacing(8);
    profileLayout->addWidget(profileForm);
    profileLayout->addStretch();

    m_splitter = new QSplitter(Qt::Horizontal);
    m_splitter->addWidget(chatPane);
    m_splitter->addWidget(m_profilePanel);
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 0);
    m_splitter->setCollapsible(0, false);
    m_splitter->setCollapsible(1, true);
    m_splitter->setChildrenCollapsible(true);
    connect(m_splitter, &QSplitter::splitterMoved, this, [this](int, int) {
        if (m_layoutReady && !m_applyingLayout) {
            saveChatLayout();
        }
    });

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_splitter);
    setLayout(mainLayout);
    setMinimumSize(kChatMinWidth, kChatMinHeight);
    polishToolbarIcons();

    m_inputEdit->setFocus();
}

void ChatWindow::polishToolbarIcons()
{
    const QColor ink = palette().color(QPalette::ButtonText);
    QList<QToolButton *> buttons;
    if (m_toolbar) {
        buttons = m_toolbar->findChildren<QToolButton *>();
    }
    if (m_profileToggle) {
        buttons.append(m_profileToggle);
    }
    for (int i = 0; i < buttons.size(); ++i) {
        QToolButton *button = buttons.at(i);
        const QVariant glyph = button->property("toolGlyph");
        if (!glyph.isValid()) {
            continue;
        }
        button->setIcon(toolGlyphIcon(static_cast<ToolGlyph>(glyph.toInt()), ink));
        button->setIconSize(QSize(16, 16));
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        const int textWidth = button->sizeHint().width();
        if (textWidth > 0) {
            button->setMinimumWidth(textWidth);
        }
    }
}

void ChatWindow::applyProfileVisible(bool visible)
{
    if (!m_profilePanel || !m_splitter) {
        return;
    }
    if (m_profileToggle) {
        const bool blocked = m_profileToggle->blockSignals(true);
        m_profileToggle->setChecked(visible);
        m_profileToggle->blockSignals(blocked);
    }
    if (visible) {
        m_profilePanel->setMinimumWidth(196);
        m_profilePanel->setMaximumWidth(320);
        m_profilePanel->show();
        const QList<int> sizes = m_splitter->sizes();
        if (sizes.size() == 2 && sizes.at(1) < 160) {
            const int right = kProfilePaneWidth;
            const int left = qMax(360, width() - right);
            if (width() < left + right) {
                resize(left + right, qMax(height(), kChatMinHeight));
            }
            m_splitter->setSizes(QList<int>() << left << right);
        }
    } else {
        const int oldWidth = width();
        const int pane = m_profilePanel->isVisible() ? m_profilePanel->width() : 0;
        m_profilePanel->hide();
        m_profilePanel->setMinimumWidth(0);
        m_profilePanel->setMaximumWidth(0);
        if (layout()) {
            layout()->activate();
        }
        if (m_layoutReady && !isMaximized() && pane > 40) {
            int floorWidth = kChatMinWidth;
            if (m_toolbar) {
                floorWidth = qMax(floorWidth, m_toolbar->minimumSizeHint().width() + 8);
            }
            const int target = qMax(floorWidth, oldWidth - pane);
            resize(target, qMax(height(), kChatMinHeight));
        }
    }
}

void ChatWindow::saveChatLayout()
{
    if (!Global::settings || !m_layoutReady || m_applyingLayout) {
        return;
    }
    QSettings *set = Global::settings;
    set->beginGroup(QStringLiteral("ChatWindow"));
    const QRect normal = normalGeometry();
    if (normal.width() < 100 || normal.height() < 100) {
        set->endGroup();
        return;
    }
    const int savedWidth = qBound(kChatMinWidth, normal.width(), 4000);
    const int savedHeight = qBound(kChatMinHeight, normal.height(), 3000);
    set->setValue(QStringLiteral("width"), savedWidth);
    set->setValue(QStringLiteral("height"), savedHeight);
    set->setValue(QStringLiteral("maximized"), isMaximized());
    set->setValue(QStringLiteral("profileVisible"),
                  m_profilePanel && m_profilePanel->isVisible());
    if (m_splitter && m_profilePanel && m_profilePanel->isVisible()) {
        set->setValue(QStringLiteral("splitter"), m_splitter->saveState());
    }
    set->endGroup();
}

void ChatWindow::restoreChatLayout()
{
    m_applyingLayout = true;
    bool profileVisible = true;
    int savedWidth = 0;
    int savedHeight = 0;
    bool maximized = false;
    QByteArray splitterState;
    if (Global::settings) {
        QSettings *set = Global::settings;
        set->beginGroup(QStringLiteral("ChatWindow"));
        savedWidth = set->value(QStringLiteral("width"), 0).toInt();
        savedHeight = set->value(QStringLiteral("height"), 0).toInt();
        maximized = set->value(QStringLiteral("maximized"), false).toBool();
        if (set->contains(QStringLiteral("profileVisible"))) {
            profileVisible = set->value(QStringLiteral("profileVisible")).toBool();
        }
        splitterState = set->value(QStringLiteral("splitter")).toByteArray();
        set->endGroup();
    }
    applyProfileVisible(profileVisible);
    if (profileVisible && m_splitter && !splitterState.isEmpty()) {
        m_splitter->restoreState(splitterState);
        const QList<int> sizes = m_splitter->sizes();
        if (sizes.size() == 2 && sizes.at(1) < 160) {
            m_splitter->setSizes(QList<int>() << qMax(360, sizes.at(0))
                                 << kProfilePaneWidth);
        }
    }
    if (savedWidth >= kChatMinWidth && savedHeight >= kChatMinHeight) {
        resize(chatSizeWithinScreen(QSize(savedWidth, savedHeight)));
    } else {
        resize(chatSizeWithinScreen(sizeHint()));
    }
    if (maximized) {
        setWindowState(windowState() | Qt::WindowMaximized);
    }
    m_applyingLayout = false;
}

void ChatWindow::toggleProfile(bool visible)
{
    m_applyingLayout = true;
    applyProfileVisible(visible);
    m_applyingLayout = false;
    if (m_layoutReady) {
        saveChatLayout();
        if (Global::settings) {
            Global::settings->sync();
        }
    }
}

void ChatWindow::refreshPeerCaption()
{
    if (Global::avatarFetch) {
        Global::avatarFetch->consider(m_peerIp);
    }
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
    QString photo = Global::avatarStore
            ? Global::avatarStore->status(m_peerIp) : QString();
    if (photo.isEmpty()) {
        photo = tr("No peer photo yet.");
    }
    if (m_photoStatus) {
        m_photoStatus->setText(photo);
    }
    if (m_shareButton) {
        if (!WinShare::platformSupports()) {
            m_shareButton->setEnabled(false);
            m_shareButton->setToolTip(tr(
                    "Windows network neighborhood is only available on Windows."));
        } else if (!WinShare::isSafeIpv4(m_peerIp, 0)) {
            m_shareButton->setEnabled(false);
            m_shareButton->setToolTip(tr("The address is not a plain IPv4 address."));
        } else {
            m_shareButton->setEnabled(true);
            m_shareButton->setToolTip(tr(
                    "Open \\\\%1\\ in Explorer. Windows asks for a password if one is required. This program does not ask for share credentials.")
                                      .arg(m_peerIp));
        }
    }
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
    if (Global::avatarFetch) {
        Global::avatarFetch->clearLocal(m_peerIp);
        return;
    }
    if (Global::avatarStore) {
        Global::avatarStore->clearKey(m_peerIp);
    }
}

void ChatWindow::openDoodle()
{
    const QPixmap pixmap = DoodleCanvas::edit(this);
    if (pixmap.isNull()) {
        return;
    }
    const QString path = ScreenCapture::savePixmap(pixmap, QStringLiteral("doodle"));
    if (path.isEmpty()) {
        QMessageBox::information(this, tr("Doodle"),
                                 tr("Could not save the doodle."));
        return;
    }
    sendPayload(tr("[Doodle] %1").arg(QFileInfo(path).fileName()),
                QStringList() << path);
}

void ChatWindow::showZone()
{
    ZoneBrowser dialog(this);
    dialog.exec();
}

void ChatWindow::openNetworkNeighborhood()
{
    const WinShareOpen opened = WinShare::openNeighborhood(m_peerIp);
    if (opened.result == winShareOk) {
        return;
    }
    QMessageBox::warning(this, tr("Network neighborhood"), WinShare::describe(opened));
}

void ChatWindow::showFavorites()
{
    QString selected;
    if (m_historyEdit) {
        selected = m_historyEdit->textCursor().selectedText();
    }
    if (selected.trimmed().isEmpty() && m_inputEdit) {
        selected = m_inputEdit->textCursor().selectedText();
    }
    selected.replace(QChar(0x2029), QChar('\n'));
    selected = selected.trimmed();
    const QString name = Global::userManager
            ? Global::userManager->displayNameForIp(m_peerIp) : m_peerIp;
    const QString note = Global::userManager
            ? Global::userManager->memoForIp(m_peerIp) : QString();
    FavoriteDialog dialog(this);
    dialog.setContext(m_peerIp, name, selected, note);
    dialog.exec();
}

void ChatWindow::browsePeerShare()
{
    ShareBrowser dialog(m_peerIp, this);
    dialog.exec();
}

void ChatWindow::browseGroupShare()
{
    QString groupId;
    if (Global::messageStore) {
        const QList<ChatRecord> records =
                Global::messageStore->loadHistory(m_peerIp, 200);
        for (int i = 0; i < records.size(); ++i) {
            if (records.at(i).direction == QLatin1String("out")) {
                continue;
            }
            const QString found = ShareFolder::groupIdFromText(records.at(i).text);
            if (!found.isEmpty()) {
                groupId = found;
            }
        }
    }
    if (groupId.isEmpty()) {
        QMessageBox::information(this, tr("Group folder"),
                tr("No discussion-folder token from this peer yet. "
                   "They publish it from their discussion window. "
                   "FeiQ has no packet for this."));
        return;
    }
    ShareBrowser dialog(m_peerIp, groupId, this);
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
                      record.readReceipt, record.encrypted, record.richHtml);
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
                               bool encrypted,
                               const QString &richHtml)
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
    QTextCharFormat tagged = textFmt;
    if (!packetNo.isEmpty()) {
        tagged.setProperty(RichText::packetProperty(), packetNo);
    }
    const bool useRich = !richHtml.isEmpty() && RichText::coversPlain(richHtml, text);
    if (useRich) {
        const QString body = RichText::plainFromHtml(richHtml);
        const QString full = text;
        RichText::insertFormatted(cellCursor, richHtml, tagged);
        if (full.size() > body.size()) {
            cellCursor.insertText(full.mid(body.size()), tagged);
        }
    } else {
        const QString displayText = EmojiPack::applyAliases(text);
        if (!displayText.isEmpty()) {
            cellCursor.insertText(displayText, tagged);
        }
    }

    QStringList paths = existingImagePaths(imagePaths);
    if (paths.isEmpty()) {
        paths = imagePathsFromText(text);
    }
    if (!paths.isEmpty()) {
        if (!text.isEmpty()) {
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

QString ChatWindow::composerRichHtml() const
{
    if (!m_inputEdit || !m_inputEdit->document()) {
        return QString();
    }
    const QString plain = m_inputEdit->toPlainText();
    const QString html = RichText::canonicalFromDocument(m_inputEdit->document());
    if (!RichText::hasFormatting(html)
            || !RichText::samePlain(RichText::plainFromHtml(html), plain)) {
        return QString();
    }
    return html;
}

void ChatWindow::sendMessage()
{
    stopLocalTyping();
    const QString text = m_inputEdit->toPlainText();
    if (text.trimmed().isEmpty() && !hasSendFile()) {
        return;
    }

    sendPayload(text, m_sendFileModel.pathList(), composerRichHtml());
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
                    .arg(item.created.toString("hh:mm:ss"),
                         OfflineQueue::previewText(item)));
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

void ChatWindow::sendPayload(const QString &text, const QStringList &pathList,
                             const QString &richHtml)
{
    if (!Global::userManager->isReachable(m_peerIp)) {
        if (!pathList.isEmpty()) {
            const bool hasText = !text.trimmed().isEmpty();
            QMessageBox::information(this, tr("Offline queue"),
                    hasText
                    ? tr("Files stay out of the offline queue because there is no "
                         "transfer server to resume them. Only the text can be queued.")
                    : tr("Files stay out of the offline queue because there is no "
                         "transfer server to resume them. Nothing was queued."));
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
        if (!richHtml.isEmpty() && !encrypted && !expectRead && Global::windowManager) {
            Global::windowManager->sendRichSidecar(
                        m_peerIp, sendMsg.packetNoString(), richHtml);
        }
        Global::windowManager->recordOutgoing(m_peerIp,
                Global::userManager->displayNameForIp(m_peerIp), recorded,
                imagePathsFromFiles(pathList),
                sendMsg.packetNoString(), expectRead, encrypted, richHtml);
    }
}

void ChatWindow::sendShake()
{
    if (!Global::userManager->isReachable(m_peerIp)) {
        if (!Global::windowManager->queueOrDeliverShake(m_peerIp, this)) {
            return;
        }
        refreshQueuePanel();
        shakeWindow();
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

void ChatWindow::attachPastedPixmap(const QPixmap &pixmap)
{
    if (pixmap.isNull()) {
        return;
    }
    const QString path = ScreenCapture::savePixmap(pixmap, "paste");
    if (path.isEmpty()) {
        return;
    }
    m_sendFileModel.addFile(path);
    updateFileCount();
}

void ChatWindow::attachPastedImageFiles(const QStringList &paths)
{
    bool added = false;
    foreach (const QString &path, paths) {
        if (!path.isEmpty() && Helper::isImageFile(path) && QFileInfo::exists(path)) {
            m_sendFileModel.addFile(path);
            added = true;
        }
    }
    if (added) {
        updateFileCount();
    }
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
        attachPastedPixmap(pixmap);
        return true;
    }

    if (mime->hasUrls()) {
        QStringList images;
        foreach (const QUrl &url, mime->urls()) {
            const QString path = url.toLocalFile();
            if (!path.isEmpty() && Helper::isImageFile(path)
                    && QFileInfo::exists(path)) {
                images << path;
            }
        }
        if (!images.isEmpty()) {
            attachPastedImageFiles(images);
            return true;
        }
    }

    return false;
}

void ChatWindow::applyRichHtml(const QString &packetNo, const QString &html)
{
    if (packetNo.isEmpty() || html.isEmpty() || !m_historyEdit) {
        return;
    }
    QTextDocument *doc = m_historyEdit->document();
    int start = -1;
    int end = -1;
    QTextCharFormat baseFmt;
    for (QTextBlock block = doc->begin(); block.isValid(); block = block.next()) {
        for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            if (!frag.isValid()) {
                continue;
            }
            if (frag.charFormat().property(RichText::packetProperty()).toString()
                    != packetNo) {
                continue;
            }
            if (start < 0) {
                start = frag.position();
                baseFmt = frag.charFormat();
            }
            end = frag.position() + frag.length();
        }
    }
    if (start < 0 || end <= start) {
        return;
    }
    QTextCursor edit(doc);
    edit.setPosition(start);
    edit.setPosition(end, QTextCursor::KeepAnchor);
    QString existing = edit.selectedText();
    existing.replace(QChar(0x2029), QLatin1Char('\n'));
    if (!RichText::coversPlain(html, existing)) {
        return;
    }
    const QString body = RichText::plainFromHtml(html);
    const QString full = existing;
    edit.removeSelectedText();
    RichText::insertFormatted(edit, html, baseFmt);
    if (full.size() > body.size()) {
        edit.insertText(full.mid(body.size()), baseFmt);
    }
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
    if (!m_layoutReady) {
        m_layoutReady = true;
        restoreChatLayout();
    }
    QWidget::showEvent(event);
    polishToolbarIcons();
    const QSize minSize = minimumSizeHint();
    if (width() < minSize.width() || height() < minSize.height()) {
        m_applyingLayout = true;
        resize(chatSizeWithinScreen(QSize(qMax(width(), minSize.width()),
                                           qMax(height(), minSize.height()))));
        m_applyingLayout = false;
        saveChatLayout();
    }
    refreshPeerCaption();
    emit chatShown(m_peerIp);
}

void ChatWindow::hideEvent(QHideEvent *event)
{
    saveChatLayout();
    if (Global::settings && m_layoutReady) {
        Global::settings->sync();
    }
    QWidget::hideEvent(event);
}

void ChatWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (!m_layoutReady || m_applyingLayout || isMaximized()) {
        return;
    }
    if (width() < kChatMinWidth || height() < kChatMinHeight) {
        return;
    }
    saveChatLayout();
}

void ChatWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::PaletteChange
            || event->type() == QEvent::StyleChange) {
        polishToolbarIcons();
    }
    if (event->type() == QEvent::WindowStateChange && m_layoutReady
            && !m_applyingLayout) {
        saveChatLayout();
    }
    QWidget::changeEvent(event);
}
