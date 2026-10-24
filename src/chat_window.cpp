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

#include <QCloseEvent>
#include <QDateTime>
#include <QList>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHostAddress>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QShortcut>
#include <QShowEvent>
#include <QTextCursor>
#include <QTextEdit>
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
    return QSize(520, 480);
}

void ChatWindow::createUi()
{
    m_peerLabel = new QLabel;
    m_peerLabel->setWordWrap(true);

    m_shakeButton = new QPushButton(tr("Shake"));
    connect(m_shakeButton, SIGNAL(clicked()), this, SLOT(sendShake()));

    m_shotButton = new QPushButton(tr("Screenshot"));
    QMenu *shotMenu = new QMenu(m_shotButton);
    shotMenu->addAction(tr("Full screen"), this, SLOT(captureFullScreen()));
    shotMenu->addAction(tr("Region"), this, SLOT(captureRegion()));
    m_shotButton->setMenu(shotMenu);

    m_attachButton = new QPushButton(tr("Attach"));
    QMenu *attachMenu = new QMenu(m_attachButton);
    attachMenu->addAction(tr("Files..."), this, SLOT(addSendFile()));
    attachMenu->addAction(tr("Folder..."), this, SLOT(addSendDir()));
    m_attachButton->setMenu(attachMenu);

    m_findButton = new QPushButton(tr("Find"));
    connect(m_findButton, SIGNAL(clicked()), this, SLOT(findInHistory()));

    m_fileListButton = new QPushButton;
    m_fileListButton->hide();
    connect(m_fileListButton, SIGNAL(clicked()),
            this, SLOT(showSendFileList()));

    QHBoxLayout *toolLayout = new QHBoxLayout;
    toolLayout->addWidget(m_shakeButton);
    toolLayout->addWidget(m_shotButton);
    toolLayout->addWidget(m_attachButton);
    toolLayout->addWidget(m_findButton);
    toolLayout->addWidget(m_fileListButton, 1);
    toolLayout->addStretch();

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

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_sendButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_peerLabel);
    mainLayout->addLayout(toolLayout);
    mainLayout->addWidget(m_historyEdit, 3);
    mainLayout->addWidget(m_inputEdit, 1);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    m_inputEdit->setFocus();
}

void ChatWindow::refreshPeerCaption()
{
    const QString caption = Global::userManager->peerCaption(m_peerIp);
    m_peerLabel->setText(caption);
    setWindowTitle(tr("Chat - %1").arg(
                       Global::userManager->displayNameForIp(m_peerIp)));
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
