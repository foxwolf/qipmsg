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
#include <QWidget>

class QLabel;
class QPushButton;
class QTextEdit;

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

    virtual QSize sizeHint() const;

signals:
    void chatShown(QString ip);

protected:
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);

private slots:
    void sendMessage();

private:
    void createUi();
    QString localName() const;
    void logOutgoing(const QString &text);

    QString m_peerIp;
    QLabel *m_peerLabel;
    QTextEdit *m_historyEdit;
    QTextEdit *m_inputEdit;
    QPushButton *m_sendButton;
    bool m_historyLoaded;
};

#endif // !CHAT_WINDOW_H
