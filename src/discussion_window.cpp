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

#include "discussion_window.h"
#include "msgCrypto.h"
#include "constants.h"
#include "global.h"
#include "helper.h"
#include "ipmsg.h"
#include "mention_highlighter.h"
#include "message_store.h"
#include "richText.h"
#include "msg.h"
#include "msg_thread.h"
#include "send_msg.h"
#include "user_manager.h"
#include "window_manager.h"

#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QCursor>
#include <QFrame>
#include <QHBoxLayout>
#include <QHostAddress>
#include <QKeySequence>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QShortcut>
#include <QShowEvent>
#include <QSplitter>
#include <QTextBlockFormat>
#include <QTextCursor>
#include <QTextEdit>
#include <QTextTable>
#include <QTextTableCell>
#include <QVBoxLayout>

DiscussionWindow::DiscussionWindow(const QString &groupId, QWidget *parent)
    : QWidget(parent), m_groupId(groupId), m_noticeBanner(0), m_noticeButton(0),
      m_composerHighlight(0), m_historyLoaded(false)
{
    createUi();
    refreshCaption();
    setWindowIcon(*Global::iconSet.value("normal"));
    if (Global::discussionStore) {
        connect(Global::discussionStore, SIGNAL(groupsChanged()),
                this, SLOT(refreshCaption()));
    }
}

QSize DiscussionWindow::sizeHint() const
{
    return QSize(880, 600);
}

void DiscussionWindow::createUi()
{
    setObjectName("chatRoot");

    m_titleLabel = new QLabel;
    m_titleLabel->setObjectName("chatPeerName");
    m_metaLabel = new QLabel;
    m_metaLabel->setObjectName("chatPeerMeta");
    m_metaLabel->setWordWrap(true);

    m_noticeButton = new QPushButton(tr("Group notice"));
    m_noticeButton->setToolTip(tr("Set, edit, or clear the notice for this discussion"));
    connect(m_noticeButton, SIGNAL(clicked()), this, SLOT(editNotice()));

    m_noticeBanner = new QLabel;
    m_noticeBanner->setObjectName(QStringLiteral("groupNoticeBanner"));
    m_noticeBanner->setWordWrap(true);
    m_noticeBanner->hide();

    QWidget *header = new QWidget;
    header->setObjectName("chatHeader");
    QVBoxLayout *headerLayout = new QVBoxLayout(header);
    headerLayout->setContentsMargins(14, 10, 14, 10);
    QHBoxLayout *titleRow = new QHBoxLayout;
    titleRow->addWidget(m_titleLabel, 1);
    titleRow->addWidget(m_noticeButton);
    headerLayout->addLayout(titleRow);
    headerLayout->addWidget(m_noticeBanner);
    headerLayout->addWidget(m_metaLabel);

    m_historyEdit = new QTextEdit;
    m_historyEdit->setObjectName("historyView");
    m_historyEdit->setReadOnly(true);
    m_historyEdit->setAcceptRichText(true);
    m_historyEdit->setFrameShape(QFrame::NoFrame);

    m_inputEdit = new RichComposer;
    m_inputEdit->setObjectName("composerEdit");
    m_inputEdit->setMaximumHeight(140);
    m_inputEdit->setPlaceholderText(tr("Message the discussion…  @name to mention"));
    m_composerHighlight = new MentionHighlighter(m_inputEdit->document());
    connect(static_cast<RichComposer *>(m_inputEdit), &RichComposer::imagePasted,
            this, [this](const QPixmap &) {
        m_inputEdit->insertPlainText(tr("[image — send it from a private chat]"));
    });

    m_sendButton = new QPushButton(tr("&Send"));
    m_sendButton->setObjectName("primaryButton");
    connect(m_sendButton, SIGNAL(clicked()), this, SLOT(sendMessage()));
    connect(new QShortcut(QKeySequence("Ctrl+Return"), this),
            SIGNAL(activated()), this, SLOT(sendMessage()));
    connect(new QShortcut(QKeySequence("Ctrl+Enter"), this),
            SIGNAL(activated()), this, SLOT(sendMessage()));

    QPushButton *mentionButton = new QPushButton(tr("@ Mention"));
    connect(mentionButton, SIGNAL(clicked()), this, SLOT(insertMention()));

    QWidget *composer = new QWidget;
    composer->setObjectName("composerBar");
    QVBoxLayout *composerLayout = new QVBoxLayout(composer);
    composerLayout->setContentsMargins(12, 8, 12, 12);
    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addWidget(mentionButton);
    buttons->addStretch();
    buttons->addWidget(m_sendButton);
    composerLayout->addWidget(new RichComposerBar(m_inputEdit, composer));
    composerLayout->addWidget(m_inputEdit);
    composerLayout->addLayout(buttons);

    QWidget *chatPane = new QWidget;
    QVBoxLayout *chatLayout = new QVBoxLayout(chatPane);
    chatLayout->setContentsMargins(0, 0, 0, 0);
    chatLayout->setSpacing(0);
    chatLayout->addWidget(header);
    chatLayout->addWidget(m_historyEdit, 3);
    chatLayout->addWidget(composer);

    QLabel *memberTitle = new QLabel(tr("Members"));
    memberTitle->setObjectName("profileName");
    QLabel *hint = new QLabel(tr("Local multi-send. Not a FeiQ multicast room."));
    hint->setObjectName("profileMeta");
    hint->setWordWrap(true);
    m_memberList = new QListWidget;
    m_memberList->setObjectName("profileForm");
    m_memberList->setWordWrap(true);
    m_memberList->setTextElideMode(Qt::ElideMiddle);

    QWidget *side = new QWidget;
    side->setObjectName("profilePanel");
    side->setMinimumWidth(220);
    QVBoxLayout *sideLayout = new QVBoxLayout(side);
    sideLayout->setContentsMargins(16, 16, 16, 16);
    sideLayout->addWidget(memberTitle);
    sideLayout->addWidget(hint);
    sideLayout->addWidget(m_memberList, 1);

    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(chatPane);
    splitter->addWidget(side);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(splitter);
}

void DiscussionWindow::refreshCaption()
{
    if (!Global::discussionStore) {
        return;
    }
    m_group = Global::discussionStore->groupById(m_groupId);
    if (m_group.id.isEmpty()) {
        close();
        return;
    }
    const QString title = m_group.name.isEmpty()
            ? tr("Discussion") : m_group.name;
    m_titleLabel->setText(title);
    m_metaLabel->setText(tr("%n member(s) · same text is sent to each peer",
                            "", m_group.members.size()));
    setWindowTitle(tr("Discussion: %1").arg(title));
    if (m_noticeBanner) {
        const QString notice = m_group.notice.trimmed();
        if (notice.isEmpty()) {
            m_noticeBanner->clear();
            m_noticeBanner->hide();
        } else {
            m_noticeBanner->setText(tr("Notice: %1").arg(notice));
            m_noticeBanner->show();
        }
    }
    refreshMembers();
    QStringList names = mentionNames();
    const QString me = localName();
    if (!me.isEmpty() && !names.contains(me)) {
        names << me;
    }
    if (m_composerHighlight) {
        m_composerHighlight->setNames(names);
    }
}

void DiscussionWindow::refreshMembers()
{
    m_memberList->clear();
    foreach (const DiscussionMember &member, m_group.members) {
        const QString line = QString("%1  %2")
                .arg(member.displayName(), member.key());
        m_memberList->addItem(line);
    }
}

QStringList DiscussionWindow::mentionNames() const
{
    return m_group.mentionNames();
}

QString DiscussionWindow::localName() const
{
    return Global::userManager ? Global::userManager->ourself().name()
                               : QString();
}

void DiscussionWindow::loadHistory()
{
    if (m_historyLoaded) {
        return;
    }
    m_historyLoaded = true;
    m_historyEdit->clear();
    const QString key = DiscussionStore::historyKey(m_groupId);
    const QList<ChatRecord> records =
            Global::messageStore->loadHistory(key, CHAT_HISTORY_LOAD_LIMIT);
    foreach (const ChatRecord &record, records) {
        const bool incoming = (record.direction != QLatin1String("out"));
        const QString name = incoming ? record.peerName : localName();
        appendMessage(record.timestamp, name, record.text, incoming,
                      record.encrypted, record.richHtml);
    }
}

void DiscussionWindow::appendMessage(const QDateTime &when, const QString &name,
                                     const QString &text, bool incoming,
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
    if (!richHtml.isEmpty() && RichText::coversPlain(richHtml, text)) {
        RichText::insertFormatted(cellCursor, richHtml, textFmt);
        const QString body = RichText::plainFromHtml(richHtml);
        if (text.size() > body.size()) {
            cellCursor.insertText(text.mid(body.size()), textFmt);
        }
    } else {
        MentionHighlighter::insertHighlighted(cellCursor, text, textFmt,
                                              mentionNames(),
                                              incoming ? QColor("#1D4ED8")
                                                       : QColor("#FDE68A"));
    }

    cursor.movePosition(QTextCursor::End);
    m_historyEdit->setTextCursor(cursor);
    m_historyEdit->ensureCursorVisible();
}

void DiscussionWindow::sendMessage()
{
    const QString text = m_inputEdit->toPlainText();
    if (text.trimmed().isEmpty()) {
        return;
    }
    QString richHtml = RichText::canonicalFromDocument(m_inputEdit->document());
    if (!RichText::hasFormatting(richHtml)
            || !RichText::samePlain(RichText::plainFromHtml(richHtml), text)) {
        richHtml.clear();
    }

    const QStringList ips = m_group.memberIps();
    quint32 baseFlags = IPMSG_SENDMSG | IPMSG_SENDCHECKOPT;
    if (ips.size() > 1) {
        baseFlags |= IPMSG_MULTICASTOPT;
    }

    int sealedCount = 0;
    int sentCount = 0;
    foreach (const QString &ip, ips) {
        quint32 flags = Global::userManager->sendFlagsForIp(ip, baseFlags);
        QString additionalInfo = text;
        additionalInfo.append(QChar('\0'));
        const bool encrypted = MsgCrypto::prepareOutgoing(ip, &additionalInfo, &flags);
        SendMsg sendMsg(QHostAddress(ip),
                        Global::userManager->sendPortForIp(ip),
                        additionalInfo, ""/* extendedInfo */, flags);
        Global::msgThread->addSendMsg(Msg(sendMsg));
        ++sentCount;
        if (encrypted) {
            ++sealedCount;
        }
        if (!richHtml.isEmpty() && !encrypted && Global::windowManager) {
            Global::windowManager->sendRichSidecar(
                        ip, sendMsg.packetNoString(), richHtml);
        }
        if (Global::windowManager) {
            Global::windowManager->recordOutgoing(ip,
                    Global::userManager->displayNameForIp(ip), text,
                    QStringList(), sendMsg.packetNoString(), false, encrypted,
                    richHtml);
        }
    }

    const bool allSealed = sentCount > 0 && sealedCount == sentCount;
    ChatRecord record;
    record.timestamp = QDateTime::currentDateTime();
    record.peerIp = DiscussionStore::historyKey(m_groupId);
    record.peerName = m_group.name;
    record.direction = "out";
    record.text = text;
    record.richHtml = richHtml;
    record.encrypted = allSealed;
    Global::messageStore->append(record);
    appendMessage(record.timestamp, localName(), text, false, allSealed, richHtml);

    m_inputEdit->clear();
    m_inputEdit->setFocus();
}

void DiscussionWindow::editNotice()
{
    if (!Global::discussionStore) {
        return;
    }
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Group notice"));
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QLabel *hint = new QLabel(tr(
            "Shown at the top of this discussion. Online members receive a "
            "plain text copy. Leave it empty to clear."));
    hint->setWordWrap(true);
    QPlainTextEdit *edit = new QPlainTextEdit;
    edit->setPlainText(m_group.notice);
    edit->setPlaceholderText(tr("Notice"));
    layout->addWidget(hint);
    layout->addWidget(edit);
    QDialogButtonBox *box = new QDialogButtonBox(
            QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    QPushButton *clearButton = box->addButton(tr("Clear"), QDialogButtonBox::ResetRole);
    layout->addWidget(box);
    connect(box, SIGNAL(accepted()), &dialog, SLOT(accept()));
    connect(box, SIGNAL(rejected()), &dialog, SLOT(reject()));
    connect(clearButton, SIGNAL(clicked()), edit, SLOT(clear()));
    dialog.resize(420, 240);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    const QString before = m_group.notice;
    if (!Global::discussionStore->setNotice(m_groupId, edit->toPlainText())) {
        return;
    }
    m_group = Global::discussionStore->groupById(m_groupId);
    if (m_group.id.isEmpty() || m_group.notice == before) {
        return;
    }
    publishNotice(m_group.notice);
}

void DiscussionWindow::publishNotice(const QString &notice)
{
    const QString body = notice.isEmpty()
            ? tr("[Group notice cleared] %1").arg(m_group.name)
            : tr("[Group notice] %1\n%2").arg(m_group.name, notice);

    QStringList reachable;
    if (Global::userManager) {
        foreach (const QString &ip, m_group.memberIps()) {
            if (Global::userManager->isReachable(ip)) {
                reachable << ip;
            }
        }
    }

    quint32 baseFlags = IPMSG_SENDMSG | IPMSG_SENDCHECKOPT;
    if (reachable.size() > 1) {
        baseFlags |= IPMSG_MULTICASTOPT;
    }
    foreach (const QString &ip, reachable) {
        if (!Global::msgThread || !Global::userManager) {
            break;
        }
        quint32 flags = Global::userManager->sendFlagsForIp(ip, baseFlags);
        QString additionalInfo = body;
        additionalInfo.append(QChar('\0'));
        const bool encrypted = MsgCrypto::prepareOutgoing(ip, &additionalInfo, &flags);
        SendMsg sendMsg(QHostAddress(ip),
                        Global::userManager->sendPortForIp(ip),
                        additionalInfo, QString(), flags);
        Global::msgThread->addSendMsg(Msg(sendMsg));
        if (Global::windowManager) {
            Global::windowManager->recordOutgoing(
                    ip, Global::userManager->displayNameForIp(ip), body,
                    QStringList(), sendMsg.packetNoString(), false, encrypted);
        }
    }

    ChatRecord record;
    record.timestamp = QDateTime::currentDateTime();
    record.peerIp = DiscussionStore::historyKey(m_groupId);
    record.peerName = m_group.name;
    record.direction = "out";
    record.text = body;
    if (Global::messageStore) {
        Global::messageStore->append(record);
    }
    appendMessage(record.timestamp, localName(), body, false, false);

    if (reachable.isEmpty()) {
        QMessageBox::information(
                this, tr("Group notice"),
                tr("Saved on this computer. No online member received it."));
    }
}

void DiscussionWindow::insertMention()
{
    QMenu menu(this);
    foreach (const QString &name, mentionNames()) {
        menu.addAction(QString("@%1").arg(name));
    }
    if (menu.isEmpty()) {
        return;
    }
    QAction *picked = menu.exec(QCursor::pos());
    if (!picked) {
        return;
    }
    m_inputEdit->insertPlainText(picked->text() + QLatin1Char(' '));
    m_inputEdit->setFocus();
}

void DiscussionWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
}

void DiscussionWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    loadHistory();
}

DiscussionMember DiscussionStore::memberFromIp(const QString &ip)
{
    DiscussionMember member;
    member.ip = ip;
    if (!Global::userManager) {
        member.name = ip;
        return member;
    }
    const int row = Global::userManager->ipToRow(ip);
    member.name = Global::userManager->displayNameForIp(ip);
    if (row != -1) {
        member.login = Global::userManager->loginName(row);
        member.host = Global::userManager->host(row);
        if (member.name.isEmpty()) {
            member.name = Global::userManager->name(row);
        }
    }
    if (member.name.isEmpty()) {
        member.name = ip;
    }
    return member;
}
