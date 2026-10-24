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
#include <QUrl>
#include <QWidget>

#include "send_file_model.h"

class QLabel;
class QPushButton;
class QTextEdit;
class QPixmap;

class ChatWindow : public QWidget
{
    Q_OBJECT

public:
    ChatWindow(const QString &ip, QWidget *parent = 0);

    QString peerIp() const { return m_peerIp; }

    void appendMessage(const QDateTime &when, const QString &name,
                       const QString &text, bool incoming);
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

private:
    void createUi();
    QString localName() const;
    void logOutgoing(const QString &text);
    bool hasSendFile() const;
    void sendPayload(const QString &text, const QStringList &pathList);
    void sendScreenshot(const QPixmap &pixmap, const QString &prefix);
    void addDroppedUrls(const QList<QUrl> &urls);
    void clearSendFiles();

    QString m_peerIp;
    QLabel *m_peerLabel;
    QTextEdit *m_historyEdit;
    QTextEdit *m_inputEdit;
    QPushButton *m_sendButton;
    QPushButton *m_fileListButton;
    QPushButton *m_shakeButton;
    QPushButton *m_shotButton;
    QPushButton *m_attachButton;
    QPushButton *m_findButton;
    SendFileModel m_sendFileModel;
    bool m_historyLoaded;
};

#endif // !CHAT_WINDOW_H
