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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "send_file_model.h"
#include "send_file_map.h"
#include "search_user_dialog.h"

#include <QMainWindow>
#include <QIcon>
#include <QSet>
#include <QSystemTrayIcon>
#include <QMap>
#include <QFileInfoList>
#include <QStringList>


class QHBoxLayout;
class QLabel;
class QPushButton;
class QCheckBox;
class QSplitter;
class QTextEdit;
class QTreeWidget;
class QTreeWidgetItem;
class QLineEdit;

class SendFileWindow;
class SendFileModel;
class SendMsg;
class RecvMsg;
class MsgWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QString text = "", QString ip = "",
            QWidget *parent = 0);

    virtual ~MainWindow();

    virtual QSize sizeHint() const;

protected:
    void contextMenuEvent(QContextMenuEvent *event);
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

protected slots:
    virtual void dragEnterEvent(QDragEnterEvent *);
    virtual void dropEvent(QDropEvent *);

private slots:
    void updateUserCount(int i);
    void rebuildContactTree();
    void sendMessage();
    void broadcastToOnline();
    void refreshUserList();
    void showSendFileList();
    void updateFileCount();
    void addSendFile();
    void addSendDir();
    void selectGroup();
    void search(QString searchString);
    void showSearchDialog();
    void openSelectedChat();
    void onContactDoubleClicked(QTreeWidgetItem *item, int column);
    void findInHistory();
    void saveSignature();
    void applyContactFilter(const QString &text);
    void toggleBlacklist();
    void createDiscussion();
    void renameDiscussion();
    void deleteDiscussion();
    void addSelectedToDiscussion();
    void editPeerMemo();
    void toggleHideFromGroup();
    void toggleMuteGroup();
    void openGroupPolicyDialog();
    void sendToDepartment();
    void onPeerTyping(QString ip, bool typing);
    void onRecentClicked(QTreeWidgetItem *item, int column);
    void setMyPhoto();
    void setPeerPhoto();
    void clearPeerPhoto();
    void openNetworkNeighborhood();
    void updateHeader();
    void openSchedule();

signals:
    void messageReplyed();

private:
    void createPeerWidget();
    void createInputWidget();
    void createSplitterWidget();
    void createSendLayout();
    void createActions();
    void createConnections();
    void selectUser();
    void logSendMsg(QString s, const QString &ip);
    void updateGroupTitle(QTreeWidgetItem *group, const QString &title);
    QTreeWidgetItem *makeGroupItem(const QString &title, const QString &key);
    void addPeerItem(QTreeWidgetItem *parent, int row);
    void addSelfItem(QTreeWidgetItem *parent);
    void addDiscussionItem(QTreeWidgetItem *parent, const QString &id);
    QStringList selectedPeerIps() const;
    QString groupAtCursor() const;
    int reachableSendCount(const QString &groupName) const;
    int selectReachableInGroup(const QString &groupName);
    void showGroupSendHint(const QString &groupName, int count);
    void addRecentItems(QTreeWidgetItem *parent);
    QStringList selectedDiscussionIds() const;
    bool isDiscussionItem(const QTreeWidgetItem *item) const;
    QList<QTreeWidgetItem *> allPeerItems() const;
    bool isBlacklisted(const QString &ip) const;
    void applyFilterToItem(QTreeWidgetItem *item, const QString &needle);
    QPixmap peerAvatar(const QString &name, int presence, int size,
                       const QString &photoKey = QString()) const;

    bool hasSendFile();
    QString constructFileInfoStr();

    void updateSelectGroupMenu();
    void updateShareAction(const QStringList &ips);

    QWidget *peerWidget;
    QWidget *inputWidget;
    QTextEdit *inputEdit;
    QPushButton *fileListButton;
    QHBoxLayout *sendLayout;
    QSplitter *splitter;

    QTreeWidget *contactTree;
    QLineEdit *searchEdit;
    QLineEdit *signatureEdit;
    QLabel *nickStatusLabel;
    QLabel *onlineCountLabel;
    QLabel *avatarLabel;
    QLabel *m_groupSendHint;
    QPushButton *refreshButton;
    QPushButton *m_scheduleButton;
    QPushButton *sendButton;
    QPushButton *broadcastButton;
    QCheckBox *encapCheckBox;
    QCheckBox *lockCheckBox;

    QAction *openChatAct;
    QAction *broadcastAllAct;
    QAction *createDiscussionAct;
    QAction *renameDiscussionAct;
    QAction *deleteDiscussionAct;
    QAction *addToDiscussionAct;
    QAction *sendFileAct;
    QAction *sendDirAct;
    QAction *searchUserAct;
    QAction *findHistoryAct;
    QAction *blacklistAct;
    QAction *setMemoAct;
    QAction *hideFromGroupAct;
    QAction *muteGroupAct;
    QAction *groupPolicyAct;
    QAction *sendToGroupAct;
    QAction *setMyPhotoAct;
    QAction *setPeerPhotoAct;
    QAction *clearPeerPhotoAct;
    QAction *openShareAct;
    QAction *setLevelOneAct;
    QAction *setLevelTwoAct;
    QMenu *setDisplayLevelMenu;
    QMenu *selectGroupMenu;

    SendFileWindow *sendFileWindow;
    SendFileModel m_sendFileModel;
    SendFileMap *m_sendFileMap;

    static quint32 m_levelOneCount;
    static quint32 m_levelTwoCount;

    QString m_initText;
    QString m_selectedIp;

    QString m_lastSearch;
    QObject *m_lastSearchDialog;
    QSet<QString> m_typingIps;
};

#endif // !MAIN_WINDOW_H
