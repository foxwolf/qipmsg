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

#include "sendThrottle.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>

namespace {

const int kMaxKBps = 102400;

struct Bucket
{
    Bucket()
        : mutex(), limitKBps(0), applied(-1), credit(0), lastMs(0)
    {
    }

    QMutex mutex;
    int limitKBps;
    int applied;
    double credit;
    qint64 lastMs;
};

Bucket &bucket()
{
    static Bucket state;
    return state;
}

} // namespace

int SendThrottle::clampKBps(int kbps)
{
    if (kbps < 0) {
        return 0;
    }
    if (kbps > kMaxKBps) {
        return kMaxKBps;
    }
    return kbps;
}

void SendThrottle::setLimitKBps(int kbps)
{
    kbps = clampKBps(kbps);
    QMutexLocker locker(&bucket().mutex);
    if (bucket().limitKBps == kbps) {
        return;
    }
    bucket().limitKBps = kbps;
    bucket().applied = -1;
}

int SendThrottle::limitKBps()
{
    QMutexLocker locker(&bucket().mutex);
    return bucket().limitKBps;
}

QString SendThrottle::formatLimit(int kbps)
{
    kbps = clampKBps(kbps);
    if (kbps <= 0) {
        return QCoreApplication::translate("SendThrottle", "Unlimited");
    }
    if (kbps % 1024 == 0) {
        return QCoreApplication::translate("SendThrottle", "%1 MB/s")
                .arg(kbps / 1024);
    }
    if (kbps > 1024) {
        return QCoreApplication::translate("SendThrottle", "%1 KB/s (%2 MB/s)")
                .arg(kbps)
                .arg(kbps / 1024.0, 0, 'f', 1);
    }
    return QCoreApplication::translate("SendThrottle", "%1 KB/s").arg(kbps);
}

bool SendThrottle::pace(qint64 nbytes, const std::function<bool()> &shouldAbort)
{
    if (nbytes <= 0) {
        return true;
    }

    qint64 sleepMs = 0;
    {
        QMutexLocker locker(&bucket().mutex);
        const int kbps = bucket().limitKBps;
        if (kbps <= 0) {
            return true;
        }

        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        const double bytesPerMs = (static_cast<double>(kbps) * 1024.0) / 1000.0;
        if (bucket().applied != kbps || bucket().lastMs == 0) {
            bucket().credit = static_cast<double>(kbps) * 1024.0;
            bucket().lastMs = now;
            bucket().applied = kbps;
        } else {
            const qint64 elapsed = qMax(qint64(0), now - bucket().lastMs);
            bucket().credit += static_cast<double>(elapsed) * bytesPerMs;
            bucket().lastMs = now;
            const double cap = static_cast<double>(kbps) * 1024.0 * 2.0;
            if (bucket().credit > cap) {
                bucket().credit = cap;
            }
        }

        if (bucket().credit >= static_cast<double>(nbytes)) {
            bucket().credit -= static_cast<double>(nbytes);
            return true;
        }

        const double deficit = static_cast<double>(nbytes) - bucket().credit;
        bucket().credit = 0;
        if (bytesPerMs <= 0) {
            return true;
        }
        sleepMs = static_cast<qint64>(deficit / bytesPerMs + 0.5);
        if (sleepMs < 1) {
            sleepMs = 1;
        }
    }

    while (sleepMs > 0) {
        if (shouldAbort && shouldAbort()) {
            return false;
        }
        if (limitKBps() <= 0) {
            return true;
        }
        const int slice = static_cast<int>(qMin(sleepMs, qint64(50)));
        QThread::msleep(static_cast<unsigned long>(slice));
        sleepMs -= slice;
    }
    if (shouldAbort && shouldAbort()) {
        return false;
    }
    return true;
}
