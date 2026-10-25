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

#ifndef LOCALMSG_H
#define LOCALMSG_H

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

#include <functional>

class QLocalServer;
class QLocalSocket;

// One plain text SENDMSG from a second process into the running qipmsg.
// The channel is a per-user local socket (named pipe on Windows). It is not
// a TCP port and not a plugin API.

struct LocalPeerRef {
    QString ip;
    QString login;
    QString host;
    bool reachable;
};

enum LocalPlanKind {
    localPlanSend = 0,
    localPlanQueue,
    localPlanBad,
    localPlanMissing,
    localPlanAmbiguous,
    localPlanEmpty,
    localPlanTooLong,
    localPlanNul,
    localPlanOffline
};

struct LocalPlan {
    LocalPlanKind kind;
    QString ip;
};

struct LocalCli {
    bool isMsg;
    QString target;
    QString text;
    QString error;
};

struct LocalSendResult {
    int code;
    QString response;
    QString error;
};

class LocalMsg {
public:
    static const int maxTextChars;

    static QString serverName();
    static QString usageText();
    static LocalCli parseCli(const QStringList &args);
    static LocalPlan planDelivery(const QString &target, const QString &text,
                                  const QList<LocalPeerRef> &peers,
                                  bool queueEnabled);
    static QString planError(const LocalPlan &plan);
    static QByteArray encodeRequest(const QString &target, const QString &text,
                                    QString *error);
    static bool takeRequest(QByteArray *buffer, QString *target, QString *text,
                            QString *error, bool *needMore);
    static LocalSendResult sendToServer(const QString &serverName,
                                        const QString &target,
                                        const QString &text,
                                        int attempts);
};

class LocalMsgServer : public QObject
{
    Q_OBJECT

public:
    typedef std::function<QString (const QString &target, const QString &text)> Handler;

    explicit LocalMsgServer(QObject *parent = 0);

    void setHandler(const Handler &handler);
    bool start(const QString &name, QString *error);
    QString name() const;

private slots:
    void acceptConnection();
    void readSocket();
    void discardSocket();

private:
    void consume(QLocalSocket *socket);

    QLocalServer *m_server;
    Handler m_handler;
    QHash<QLocalSocket *, QByteArray> m_buffers;
};

#endif // !LOCALMSG_H
