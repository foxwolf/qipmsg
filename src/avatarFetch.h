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

#ifndef AVATARFETCH_H
#define AVATARFETCH_H

#include <QAbstractSocket>
#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>

class QTcpSocket;
class QTimer;

// Fetches a qipmsg peer photo over TCP command 126. One peer at a time
// so a presence refresh does not open a socket per row. FeiQ peers are
// not contacted.

class AvatarFetch : public QObject
{
    Q_OBJECT

public:
    explicit AvatarFetch(QObject *parent = 0);

    void consider(const QString &ip);
    void clearLocal(const QString &ip);

public slots:
    void republish();

private slots:
    void startNext();
    void onConnected();
    void onReadyRead();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    void onTimeout();

private:
    QString explain(const QString &code) const;
    void fail(const QString &reason);
    void stopSocket();

    QStringList m_queue;
    QHash<QString, QString> m_expect;
    QString m_current;
    QString m_wantId;
    QTcpSocket *m_socket;
    QTimer *m_timer;
    QByteArray m_buf;
    int m_pngSize;
    int m_headerAt;
    bool m_busy;
    bool m_closing;
};

#endif // !AVATARFETCH_H
