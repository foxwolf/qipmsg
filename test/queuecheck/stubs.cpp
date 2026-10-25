#include "global.h"
#include "helper.h"

#include <QDir>

namespace Global {

OfflineQueue *offlineQueue = 0;

QString peerLabelForIp(const QString &ip)
{
    return ip;
}

}

QString Helper::appHomePath()
{
    return QDir::homePath() + QStringLiteral("/.qipmsg");
}
