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
#include "ipmsg.h"
#include "message_store.h"
#include "msg.h"
#include "msg_thread.h"
#include "preferences.h"
#include "send_msg.h"
#include "user_manager.h"
#include "window_manager.h"

#include <QCloseEvent>
#include <QDateTime>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QPushButton>
#include <QShortcut>
#include <QShowEvent>
#include <QTextCursor>
#include <QTextEdit>
#include <QVBoxLayout>

ChatWindow::ChatWindow(const QString &ip, QWidget *parent)
    : QWidget(parent), m_peerIp(ip), m_historyLoaded(false)
{
    createUi();
    refreshPeerCaption();
    setWindowIcon(*Global::iconSet.value("normal"));
}

QSize ChatWindow::sizeHint() const
{
    return QSize(480, 420);
}

void ChatWindow::createUi()
{
    m_peerLabel = new QLabel;
    m_peerLabel->setWordWrap(true);

    m_historyEdit = new QTextEdit;
    m_historyEdit->setReadOnly(true);
    m_historyEdit->setAcceptRichText(true);
    m_historyEdit->setTabChangesFocus(true);

    m_inputEdit = new QTextEdit;
    m_inputEdit->setAcceptRichText(false);
    m_inputEdit->setTabChangesFocus(true);
    m_inputEdit->setMaximumHeight(120);

    m_sendButton = new QPushButton(tr("&Send"));
    connect(m_sendButton, SIGNAL(clicked()),
            this, SLOT(sendMessage()));
    connect(new QShortcut(QKeySequence("Ctrl+Return"), this),
            SIGNAL(activated()),
            this, SLOT(sendMessage()));
    connect(new QShortcut(QKeySequence("Ctrl+Enter"), this),
            SIGNAL(activated()),
            this, SLOT(sendMessage()));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_sendButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_peerLabel);
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
    setWindowTitle(tr("Chat - %1").arg(caption));
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

void ChatWindow::sendMessage()
{
    const QString text = m_inputEdit->toPlainText();
    if (text.trimmed().isEmpty()) {
        return;
    }

    quint32 flags = IPMSG_SENDMSG | IPMSG_SENDCHECKOPT;
    QString additionalInfo = text;
    additionalInfo.append(QChar('\0'));

    SendMsg sendMsg(QHostAddress(m_peerIp), IPMSG_DEFAULT_PORT,
                    additionalInfo, ""/* extendedInfo */, flags);
    Global::msgThread->addSendMsg(Msg(sendMsg));

    if (Global::preferences->isLogMsg) {
        logOutgoing(text);
    }

    Global::windowManager->recordOutgoing(m_peerIp,
            Global::userManager->displayNameForIp(m_peerIp), text);

    m_inputEdit->clear();
    m_inputEdit->setFocus();
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
