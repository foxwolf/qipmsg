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

#ifndef CHAT_WINDOW_H
#define CHAT_WINDOW_H

#include <QDateTime>
#include <QList>
#include <QStringList>
#include <QUrl>
#include <QWidget>

#include "send_file_model.h"

class QEvent;
class QLabel;
class QListWidget;
class QPushButton;
class QTextBrowser;
class QTextEdit;
class QTextCursor;
class QPixmap;
class QToolButton;

class ChatWindow : public QWidget
{
    Q_OBJECT

public:
    ChatWindow(const QString &ip, QWidget *parent = 0);

    QString peerIp() const { return m_peerIp; }

    void appendMessage(const QDateTime &when, const QString &name,
                       const QString &text, bool incoming,
                       const QStringList &imagePaths = QStringList());
    void loadHistory();
    void setDraft(const QString &text);
    void refreshPeerCaption();
    void shakeWindow();

    virtual QSize sizeHint() const;

signals:
    void chatShown(QString ip);

protected:
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

private slots:
    void sendMessage();
    void sendShake();
    void captureFullScreen();
    void captureRegion();
    void addSendFile();
    void addSendDir();
    void showSendFileList();
    void updateFileCount();
    void findInHistory();
    void showTransferMonitor();
    void cancelAttachments();
    void showEmojiPicker();
    void openChatLink(const QUrl &url);
    void refreshQueuePanel();
    void cancelSelectedQueued();
    void clearQueued();

private:
    void createUi();
    QToolButton *makeToolButton(const QString &text, const QString &tip,
                                const char *slot, bool enabled = true);
    QToolButton *makeStubButton(const QString &text);
    QString localName() const;
    void logOutgoing(const QString &text);
    bool hasSendFile() const;
    void sendPayload(const QString &text, const QStringList &pathList);
    void sendScreenshot(const QPixmap &pixmap, const QString &prefix);
    void addDroppedUrls(const QList<QUrl> &urls);
    void clearSendFiles();
    void refreshProfile();
    QPixmap peerAvatar(const QString &name, int size) const;
    QStringList imagePathsFromText(const QString &text) const;
    QStringList existingImagePaths(const QStringList &paths) const;
    void insertBubbleImages(QTextCursor &cursor, const QStringList &paths);
    void insertEmoji(const QString &glyph);
    bool pasteClipboardImage();
    QStringList imagePathsFromFiles(const QStringList &pathList) const;

    QString m_peerIp;
    QLabel *m_peerLabel;
    QLabel *m_peerMeta;
    QLabel *m_headerAvatar;
    QLabel *m_profileAvatar;
    QLabel *m_profileName;
    QLabel *m_profileNick;
    QLabel *m_profileStatus;
    QLabel *m_profileSign;
    QLabel *m_profileHost;
    QLabel *m_profileLogin;
    QLabel *m_profileGroup;
    QLabel *m_profileIp;
    QTextBrowser *m_historyEdit;
    QWidget *m_queuePanel;
    QLabel *m_queueBanner;
    QListWidget *m_queueList;
    QPushButton *m_cancelQueuedButton;
    QPushButton *m_clearQueueButton;
    QTextEdit *m_inputEdit;
    QPushButton *m_sendButton;
    QPushButton *m_fileListButton;
    QPushButton *m_stripFile;
    QPushButton *m_stripMonitor;
    QPushButton *m_stripDir;
    QPushButton *m_stripCancel;
    SendFileModel m_sendFileModel;
    bool m_historyLoaded;
};

#endif // !CHAT_WINDOW_H
