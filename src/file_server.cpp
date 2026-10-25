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

#include "send_file_thread.h"
#include "file_server.h"
#include "constants.h"
#include "global.h"
#include "preferences.h"
#include "nicBind.h"
#include "portBind.h"
#include "send_file_model.h"
#include "send_file_manager.h"

#include <QtDebug>

FileServer::FileServer(QObject *parent)
    : QTcpServer(parent)
{
}

FileServer::~FileServer()
{
}

void FileServer::incomingConnection(qintptr socketDescriptor)
{
    SendFileThread *sendFileThread
        = new SendFileThread(socketDescriptor, this);

    connect(sendFileThread, SIGNAL(finished()),
            sendFileThread, SLOT(deleteLater()));

    sendFileThread->start();
}

void FileServer::startListening()
{
    const QHostAddress listenAddress = Global::preferences->listenAddress();
    const quint16 listenPort = Global::preferences->listenPort();
    const portSideResult tried = PortBind::tryTcp(this, listenAddress, listenPort,
                                                   true);
    if (!tried.ok) {
        m_listenError = tried.error;
        qWarning() << "FileServer::startListening: listen"
                   << listenAddress.toString() << listenPort
                   << "failed:" << tried.error
                   << "kind" << static_cast<int>(tried.kind)
                   << "(continuing without TCP file listen)";
        return;
    }
    m_listenError.clear();
    const NicChoice choice = NicBind::resolve(
                Global::preferences->bindMac,
                Global::preferences->bindInterfaceName,
                Global::preferences->bindAddress,
                NicBind::localInterfaces());
    if (!choice.any) {
        QString stickError;
        if (!NicBind::stickSocket(socketDescriptor(), choice.name, choice.index,
                                  &stickError)) {
            qWarning() << "FileServer::startListening: interface stick failed"
                       << choice.interfaceId << stickError;
        }
    }
}

QString FileServer::listenError() const
{
    if (!m_listenError.isEmpty()) {
        return m_listenError;
    }
    return errorString();
}

