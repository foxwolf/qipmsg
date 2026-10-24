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
#include "user_manager.h"
#include "window_manager.h"

#include "transfer_file_window.h"

#include <QCloseEvent>
#include <QDateTime>
#include <QList>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHostAddress>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QPushButton>
#include <QShortcut>
#include <QShowEvent>
#include <QSplitter>
#include <QTextCursor>
#include <QTextEdit>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

ChatWindow::ChatWindow(const QString &ip, QWidget *parent)
    : QWidget(parent), m_peerIp(ip), m_historyLoaded(false)
{
    setAcceptDrops(true);
    createUi();
    refreshPeerCaption();
    setWindowIcon(*Global::iconSet.value("normal"));
}

QSize ChatWindow::sizeHint() const
{
    return QSize(760, 520);
}

QToolButton *ChatWindow::makeToolButton(const QString &text, const QString &tip,
                                        const char *slot, bool enabled)
{
    QToolButton *button = new QToolButton;
    button->setText(text);
    button->setToolTip(tip);
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
    m_headerAvatar = new QLabel;
    m_headerAvatar->setFixedSize(28, 28);
    m_peerLabel = new QLabel;
    m_peerLabel->setStyleSheet("font-weight: bold;");

    QWidget *header = new QWidget;
    header->setObjectName("chatHeader");
    header->setStyleSheet(
        "#chatHeader { background: #d4f3df; border: 1px solid #b5e0c6; }");
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->addWidget(m_headerAvatar);
    headerLayout->addWidget(m_peerLabel, 1);

    QHBoxLayout *toolLayout = new QHBoxLayout;
    toolLayout->setSpacing(4);
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

    m_historyEdit = new QTextEdit;
    m_historyEdit->setReadOnly(true);
    m_historyEdit->setAcceptRichText(true);
    m_historyEdit->setTabChangesFocus(true);

    m_inputEdit = new QTextEdit;
    m_inputEdit->setAcceptRichText(false);
    m_inputEdit->setTabChangesFocus(true);
    m_inputEdit->setMaximumHeight(120);
    m_inputEdit->setAcceptDrops(false);

    m_sendButton = new QPushButton(tr("&Send"));
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

    QWidget *chatPane = new QWidget;
    QVBoxLayout *chatLayout = new QVBoxLayout(chatPane);
    chatLayout->setContentsMargins(0, 0, 0, 0);
    chatLayout->addWidget(header);
    chatLayout->addLayout(toolLayout);
    chatLayout->addWidget(m_historyEdit, 3);
    chatLayout->addLayout(stripLayout);
    chatLayout->addWidget(m_inputEdit, 1);
    chatLayout->addLayout(buttonLayout);

    QGroupBox *profileBox = new QGroupBox(tr("Peer profile"));
    profileBox->setMinimumWidth(220);
    m_profileName = new QLabel;
    m_profileHost = new QLabel;
    m_profileLogin = new QLabel;
    m_profileGroup = new QLabel;
    m_profileIp = new QLabel;
    QFormLayout *form = new QFormLayout;
    form->addRow(tr("User:"), m_profileName);
    form->addRow(tr("Host:"), m_profileHost);
    form->addRow(tr("Login:"), m_profileLogin);
    form->addRow(tr("Group:"), m_profileGroup);
    form->addRow(tr("IP address:"), m_profileIp);
    profileBox->setLayout(form);

    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(chatPane);
    splitter->addWidget(profileBox);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    QVBoxLayout *mainLayout = new QVBoxLayout;
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
    m_peerLabel->setText(header);
    m_headerAvatar->setPixmap(avatarPixmap(QColor(80, 170, 90), name));
    setWindowTitle(header);
    refreshProfile();
}

void ChatWindow::refreshProfile()
{
    const int row = Global::userManager->ipToRow(m_peerIp);
    m_profileName->setText(Global::userManager->displayNameForIp(m_peerIp));
    m_profileHost->setText(row != -1 ? Global::userManager->host(row) : QString("-"));
    m_profileLogin->setText(row != -1 ? Global::userManager->loginName(row) : QString("-"));
    m_profileGroup->setText(row != -1 && !Global::userManager->group(row).isEmpty()
                            ? Global::userManager->group(row) : QString("-"));
    m_profileIp->setText(m_peerIp);
}

QPixmap ChatWindow::avatarPixmap(const QColor &color, const QString &letter) const
{
    QPixmap pix(28, 28);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setBrush(color);
    p.setPen(Qt::NoPen);
    p.drawEllipse(1, 1, 26, 26);
    p.setPen(Qt::white);
    QFont font = p.font();
    font.setBold(true);
    p.setFont(font);
    QString ch = letter.trimmed();
    ch = ch.isEmpty() ? QString("?") : ch.left(1);
    p.drawText(pix.rect(), Qt::AlignCenter, ch);
    return pix;
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
        appendMessage(record.timestamp, name, record.text, incoming);
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
                               const QString &text, bool incoming)
{
    const QString timeText = when.toString("yyyy-MM-dd hh:mm:ss");
    const QString color = incoming ? QString("#1a4d8c") : QString("#1a7a3a");
    const QString safeName = name.toHtmlEscaped();
    const QString safeText = text.toHtmlEscaped().replace('\n', "<br/>");

    m_historyEdit->append(
        QString("<p style=\"margin:6px 0 2px 0;color:%1;\"><b>%2</b> "
                "<span style=\"color:#666;font-weight:normal;\">%3</span>"
                "<br/>%4</p>")
            .arg(color, safeName, timeText, safeText));

    QTextCursor cursor = m_historyEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_historyEdit->setTextCursor(cursor);
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

void ChatWindow::sendPayload(const QString &text, const QStringList &pathList)
{
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
                Global::userManager->displayNameForIp(m_peerIp), recorded);
    }
}

void ChatWindow::sendShake()
{
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
