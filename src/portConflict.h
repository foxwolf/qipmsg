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

#ifndef PORTCONFLICT_H
#define PORTCONFLICT_H

#include <QObject>

class QHostAddress;
class QWidget;
struct portListenResult;
struct portSideResult;

// Startup dialog for a UDP/TCP port that cannot be opened.
// resolveBeforeListen returns true when the user kept a port that did not
// probe cleanly. The contact window stays open either way.
class PortConflict : public QObject {
    Q_OBJECT

public:
    static bool resolveBeforeListen(QWidget *parent);
    static void warnStillDown(QWidget *parent, quint16 port, const QString &detail);

private:
    PortConflict() {}
    static QString sideLine(const QString &name, const portSideResult &side);
    static QString explainFailure(const portListenResult &result);
    static void offerDefaultPort(QWidget *parent, const QHostAddress &address,
                                 quint16 port);
    static bool askUseSuggested(QWidget *parent, const portListenResult &result,
                                const QHostAddress &address, quint16 suggested);
};

#endif // !PORTCONFLICT_H
