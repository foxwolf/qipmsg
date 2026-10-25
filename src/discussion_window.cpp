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
#include "groupLog.h"
#include "msgCrypto.h"
#include "constants.h"
#include "global.h"
#include "helper.h"
#include "ipmsg.h"
#include "mention_highlighter.h"
#include "message_store.h"
#include "richText.h"
#include "shareFolder.h"
#include "msg.h"
#include "msg_thread.h"
#include "send_msg.h"
#include "user_manager.h"
#include "window_manager.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QCursor>
#include <QDir>
#include <QFrame>
#include <QHBoxLayout>
#include <QHostAddress>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
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
      m_shareLabel(0), m_shareList(0), m_shareSetButton(0), m_shareClearButton(0),
      m_sharePublishButton(0), m_logStatus(0), m_logList(0), m_logEdit(0),
      m_logTell(0), m_composerHighlight(0), m_historyLoaded(false)
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

    QLabel *logTitle = new QLabel(tr("Group log"));
    logTitle->setObjectName("profileName");
    m_logStatus = new QLabel(tr("Local notes for this discussion. Not a FeiQ room."));
    m_logStatus->setObjectName("profileMeta");
    m_logStatus->setWordWrap(true);
    m_logList = new QListWidget;
    m_logList->setObjectName("profileForm");
    m_logList->setMaximumHeight(120);
    m_logList->setWordWrap(true);
    m_logEdit = new QLineEdit;
    m_logEdit->setMaxLength(500);
    m_logEdit->setPlaceholderText(tr("Short log note"));
    m_logTell = new QCheckBox(tr("Tell online members"));
    m_logTell->setChecked(true);
    m_logTell->setToolTip(tr(
            "Also send this note as plain text. Offline members are not queued. "
            "This is not a FeiQ room protocol."));
    QPushButton *logAdd = new QPushButton(tr("Add log entry"));
    QPushButton *logExport = new QPushButton(tr("Export log"));
    connect(logAdd, SIGNAL(clicked()), this, SLOT(addLogEntry()));
    connect(logExport, SIGNAL(clicked()), this, SLOT(exportLog()));
    connect(m_logEdit, SIGNAL(returnPressed()), this, SLOT(addLogEntry()));
    sideLayout->addWidget(logTitle);
    sideLayout->addWidget(m_logStatus);
    sideLayout->addWidget(m_logList);
    sideLayout->addWidget(m_logEdit);
    sideLayout->addWidget(m_logTell);
    sideLayout->addWidget(logAdd);
    sideLayout->addWidget(logExport);

    m_shareLabel = new QLabel(tr("No group folder yet."));
    m_shareLabel->setObjectName("profileMeta");
    m_shareLabel->setWordWrap(true);
    m_shareList = new QListWidget;
    m_shareList->setObjectName("profileForm");
    m_shareSetButton = new QPushButton(tr("Set folder"));
    m_shareClearButton = new QPushButton(tr("Clear folder"));
    m_sharePublishButton = new QPushButton(tr("Tell the group"));
    m_shareSetButton->setToolTip(tr(
            "Folder on this computer for this discussion only."));
    m_sharePublishButton->setToolTip(tr(
            "Send the file names and a qipmsg token. Members request files "
            "while you are online. This is not a FeiQ room."));
    connect(m_shareSetButton, SIGNAL(clicked()), this, SLOT(chooseShareFolder()));
    connect(m_shareClearButton, SIGNAL(clicked()), this, SLOT(clearShareFolder()));
    connect(m_sharePublishButton, SIGNAL(clicked()), this, SLOT(publishShare()));
    sideLayout->addWidget(m_shareLabel);
    sideLayout->addWidget(m_shareList, 1);
    sideLayout->addWidget(m_shareSetButton);
    sideLayout->addWidget(m_shareClearButton);
    sideLayout->addWidget(m_sharePublishButton);

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
    refreshShare();
    refreshLog();
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
    deliverPlain(body);
}

void DiscussionWindow::deliverPlain(const QString &body)
{
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
                this, tr("Discussion"),
                tr("Saved on this computer. No online member received it."));
    }
}

void DiscussionWindow::refreshShare()
{
    if (!m_shareList || !m_shareLabel) {
        return;
    }
    m_shareList->clear();
    const QString path = m_group.sharePath.trimmed();
    if (path.isEmpty()) {
        m_shareLabel->setText(tr(
                "No group folder yet. Set one on this computer, then tell the group. "
                "Members can fetch files only while you are online."));
        return;
    }
    m_shareLabel->setText(tr("This discussion folder: %1").arg(path));
    QList<ShareEntry> entries;
    bool truncated = false;
    QString error;
    if (!ShareFolder::listDirectory(path, QString(), &entries, &truncated, &error)) {
        m_shareList->addItem(tr("Folder is not available (%1).").arg(error));
        return;
    }
    if (entries.isEmpty()) {
        m_shareList->addItem(tr("(empty)"));
    }
    for (int i = 0; i < entries.size(); ++i) {
        const ShareEntry &entry = entries.at(i);
        const QString kind = entry.directory ? tr("Folder") : tr("File");
        m_shareList->addItem(QString("%1  %2").arg(entry.name, kind));
    }
    if (truncated) {
        m_shareList->addItem(tr("List truncated."));
    }
}

void DiscussionWindow::chooseShareFolder()
{
    if (!Global::discussionStore) {
        return;
    }
    const QString current = m_group.sharePath.isEmpty()
            ? QDir::homePath() : m_group.sharePath;
    const QString dir = QFileDialog::getExistingDirectory(
                this, tr("Discussion folder"), current);
    if (dir.isEmpty()) {
        return;
    }
    if (QDir::cleanPath(dir) == QDir::cleanPath(m_group.sharePath)) {
        return;
    }
    if (!Global::discussionStore->setSharePath(m_groupId, dir)) {
        QMessageBox::warning(this, tr("Discussion folder"),
                             tr("That folder could not be saved."));
        return;
    }
    m_group = Global::discussionStore->groupById(m_groupId);
    refreshShare();
}

void DiscussionWindow::clearShareFolder()
{
    if (!Global::discussionStore || m_group.sharePath.isEmpty()) {
        return;
    }
    Global::discussionStore->setSharePath(m_groupId, QString());
    m_group = Global::discussionStore->groupById(m_groupId);
    refreshShare();
}

void DiscussionWindow::publishShare()
{
    if (m_group.sharePath.trimmed().isEmpty()) {
        QMessageBox::information(this, tr("Discussion folder"),
                tr("Set a folder on this computer first."));
        return;
    }
    const QString token = ShareFolder::groupShareLine(m_group.id);
    if (token.isEmpty()) {
        return;
    }
    QList<ShareEntry> entries;
    bool truncated = false;
    QString error;
    QStringList lines;
    if (!ShareFolder::listDirectory(m_group.sharePath, QString(), &entries,
                                    &truncated, &error)) {
        lines << tr("Folder is not available right now.");
    } else {
        const int limit = qMin(entries.size(), 40);
        for (int i = 0; i < limit; ++i) {
            lines << (entries.at(i).directory
                      ? entries.at(i).name + QLatin1Char('/')
                      : entries.at(i).name);
        }
        if (truncated || entries.size() > limit) {
            lines << tr("...");
        }
        if (lines.isEmpty()) {
            lines << tr("(empty)");
        }
    }
    const QString body = tr("[Group share] %1\n%2\n%3\n"
                            "qipmsg peers can request these files while I am online. "
                            "FeiQ cannot browse this folder.")
            .arg(m_group.name, lines.join(QLatin1Char('\n')), token);
    deliverPlain(body);
}

void DiscussionWindow::refreshLog()
{
    if (!m_logList) {
        return;
    }
    m_logList->clear();
    const QList<GroupLogEntry> rows = GroupLog::load(m_groupId, 200);
    if (rows.isEmpty()) {
        m_logList->addItem(tr("No log entries yet."));
        return;
    }
    for (int i = 0; i < rows.size(); ++i) {
        const GroupLogEntry &entry = rows.at(i);
        m_logList->addItem(QString("%1  %2: %3")
                           .arg(entry.when, entry.author, entry.text));
    }
}

void DiscussionWindow::addLogEntry()
{
    if (!m_logEdit) {
        return;
    }
    const QString text = m_logEdit->text().trimmed();
    if (text.isEmpty()) {
        QMessageBox::information(this, tr("Group log"),
                                 tr("Write a short note first."));
        return;
    }
    QString error;
    if (!GroupLog::append(m_groupId, localName(), text, &error)) {
        QString message = tr("Could not write the log.");
        if (error == QLatin1String("full")) {
            message = tr("This log file is full (2 MB). Export it before adding more.");
        } else if (error == QLatin1String("empty")) {
            message = tr("Write a short note first.");
        } else if (error == QLatin1String("id")) {
            message = tr("This discussion cannot store a log.");
        }
        QMessageBox::warning(this, tr("Group log"), message);
        return;
    }
    m_logEdit->clear();
    refreshLog();
    if (m_logStatus) {
        m_logStatus->setText(tr("Saved on this computer."));
    }
    if (m_logTell && m_logTell->isChecked()) {
        const QString body = tr("[Group log] %1\n%2").arg(m_group.name, text);
        deliverPlain(body);
    }
}

void DiscussionWindow::exportLog()
{
    const QList<GroupLogEntry> rows = GroupLog::load(m_groupId, 1);
    if (rows.isEmpty()) {
        QMessageBox::information(this, tr("Group log"),
                                 tr("There is nothing to export."));
        return;
    }
    const QString path = QFileDialog::getSaveFileName(
                this, tr("Export group log"),
                QDir::homePath(),
                tr("Text (*.txt);;JSON lines (*.jsonl)"));
    if (path.isEmpty()) {
        return;
    }
    QByteArray bytes;
    if (path.endsWith(QLatin1String(".jsonl"), Qt::CaseInsensitive)) {
        QFile in(GroupLog::filePath(m_groupId));
        if (!in.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, tr("Group log"),
                                 tr("Could not read the log."));
            return;
        }
        bytes = in.readAll();
    } else {
        bytes = GroupLog::exportPlain(m_groupId).toUtf8();
    }
    QFile out(path);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)
            || out.write(bytes) != bytes.size()) {
        QMessageBox::warning(this, tr("Group log"),
                             tr("Could not write %1.").arg(path));
        return;
    }
    if (m_logStatus) {
        m_logStatus->setText(tr("Exported to %1.").arg(path));
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
