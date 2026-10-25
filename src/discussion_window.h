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

#ifndef DISCUSSION_WINDOW_H
#define DISCUSSION_WINDOW_H

#include <QDateTime>
#include <QWidget>

#include "discussion_store.h"

class QCheckBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QTextEdit;
class MentionHighlighter;

class DiscussionWindow : public QWidget
{
    Q_OBJECT

public:
    DiscussionWindow(const QString &groupId, QWidget *parent = 0);

    QString groupId() const { return m_groupId; }
    void loadHistory();

    virtual QSize sizeHint() const;

protected:
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);

private slots:
    void refreshCaption();
    void sendMessage();
    void insertMention();
    void editNotice();
    void chooseShareFolder();
    void clearShareFolder();
    void publishShare();
    void addLogEntry();
    void exportLog();

private:
    void createUi();
    void refreshShare();
    void refreshLog();
    void appendMessage(const QDateTime &when, const QString &name,
                       const QString &text, bool incoming,
                       bool encrypted = false,
                       const QString &richHtml = QString());
    void refreshMembers();
    void publishNotice(const QString &notice);
    void deliverPlain(const QString &body);
    QStringList mentionNames() const;
    QString localName() const;

    QString m_groupId;
    DiscussionGroup m_group;
    QLabel *m_titleLabel;
    QLabel *m_metaLabel;
    QLabel *m_noticeBanner;
    QPushButton *m_noticeButton;
    QLabel *m_shareLabel;
    QListWidget *m_shareList;
    QPushButton *m_shareSetButton;
    QPushButton *m_shareClearButton;
    QPushButton *m_sharePublishButton;
    QLabel *m_logStatus;
    QListWidget *m_logList;
    QLineEdit *m_logEdit;
    QCheckBox *m_logTell;
    QTextEdit *m_historyEdit;
    QTextEdit *m_inputEdit;
    QListWidget *m_memberList;
    QPushButton *m_sendButton;
    MentionHighlighter *m_composerHighlight;
    bool m_historyLoaded;
};

#endif // !DISCUSSION_WINDOW_H
