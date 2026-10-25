// This file is part of QIpMsg.
//
// QIpMsg is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "nicBind.h"

#include <QHostAddress>
#include <QNetworkInterface>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#ifndef IP_UNICAST_IF
#define IP_UNICAST_IF 31
#endif
#else
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#if defined(Q_OS_LINUX)
#include <net/if.h>
#endif

#if defined(Q_OS_MAC)
#include <net/if.h>
#endif

namespace {

bool usableAddress(const QHostAddress &ip)
{
    return ip.protocol() == QAbstractSocket::IPv4Protocol
            && !ip.isLoopback()
            && !ip.isNull();
}

bool nameMatch(const QString &saved, const NicRecord &record)
{
    if (saved.isEmpty()) {
        return false;
    }
    return record.name.compare(saved, Qt::CaseInsensitive) == 0
            || record.humanName.compare(saved, Qt::CaseInsensitive) == 0;
}

void takeRecord(NicChoice *choice, const NicRecord &record)
{
    choice->found = true;
    choice->any = false;
    choice->ipv4 = record.ipv4;
    choice->mac = NicBind::normalizeMac(record.mac);
    choice->name = record.name;
    choice->humanName = record.humanName;
    choice->index = record.index;
    choice->interfaceId = NicBind::interfaceId(choice->mac, choice->name);
}

} // namespace

QString NicBind::normalizeMac(const QString &raw)
{
    QString hex;
    const QString text = raw.trimmed();
    for (int i = 0; i < text.size(); ++i) {
        const QChar c = text.at(i);
        if (c.isDigit() || (c >= QLatin1Char('a') && c <= QLatin1Char('f'))
                || (c >= QLatin1Char('A') && c <= QLatin1Char('F'))) {
            hex.append(c.toLower());
        }
    }
    if (hex.size() != 12) {
        return QString();
    }
    QString out;
    for (int i = 0; i < 12; i += 2) {
        if (!out.isEmpty()) {
            out.append(QLatin1Char(':'));
        }
        out.append(hex.mid(i, 2));
    }
    return out;
}

QString NicBind::interfaceId(const QString &mac, const QString &name)
{
    const QString normalized = normalizeMac(mac);
    if (!normalized.isEmpty()) {
        return normalized;
    }
    return name.trimmed();
}

bool NicBind::sameInterface(const NicRecord &record, const NicChoice &choice)
{
    const QString mac = normalizeMac(choice.mac);
    if (!mac.isEmpty()) {
        return normalizeMac(record.mac) == mac;
    }
    if (!choice.name.isEmpty()) {
        return nameMatch(choice.name, record);
    }
    if (!choice.ipv4.isEmpty()) {
        return record.ipv4 == choice.ipv4;
    }
    return false;
}

QList<NicRecord> NicBind::localInterfaces()
{
    QList<NicRecord> records;
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for (int i = 0; i < interfaces.size(); ++i) {
        const QNetworkInterface nic = interfaces.at(i);
        const QNetworkInterface::InterfaceFlags flags = nic.flags();
        if (!(flags & QNetworkInterface::IsUp)
                || (flags & QNetworkInterface::IsLoopBack)) {
            continue;
        }
        const QList<QNetworkAddressEntry> entries = nic.addressEntries();
        for (int e = 0; e < entries.size(); ++e) {
            const QHostAddress ip = entries.at(e).ip();
            if (!usableAddress(ip)) {
                continue;
            }
            NicRecord record;
            record.name = nic.name();
            record.humanName = nic.humanReadableName();
            record.mac = normalizeMac(nic.hardwareAddress());
            record.ipv4 = ip.toString();
            record.index = nic.index();
            const QHostAddress broadcast = entries.at(e).broadcast();
            if (usableAddress(broadcast)
                    && broadcast != QHostAddress::AnyIPv4
                    && broadcast != QHostAddress::Broadcast) {
                record.broadcast = broadcast.toString();
            }
            records.append(record);
        }
    }
    return records;
}

NicChoice NicBind::resolve(const QString &savedMac, const QString &savedName,
                           const QString &savedIpv4, const QList<NicRecord> &nics)
{
    NicChoice choice;
    const QString mac = normalizeMac(savedMac);
    const QString name = savedName.trimmed();
    const QString ipv4 = savedIpv4.trimmed();
    const bool any = mac.isEmpty() && name.isEmpty()
            && (ipv4.isEmpty()
                || ipv4 == QLatin1String("0.0.0.0")
                || ipv4.compare(QLatin1String("any"), Qt::CaseInsensitive) == 0);
    if (any) {
        choice.any = true;
        return choice;
    }
    choice.any = false;
    choice.mac = mac;
    choice.name = name;
    choice.ipv4 = (ipv4 == QLatin1String("0.0.0.0")) ? QString() : ipv4;
    choice.interfaceId = interfaceId(mac, name);
    for (int i = 0; i < nics.size(); ++i) {
        const NicRecord &record = nics.at(i);
        bool match = false;
        if (!mac.isEmpty()) {
            match = normalizeMac(record.mac) == mac;
        } else if (!name.isEmpty()) {
            match = nameMatch(name, record);
        } else if (!choice.ipv4.isEmpty()) {
            match = record.ipv4 == choice.ipv4;
        }
        if (match) {
            takeRecord(&choice, record);
            return choice;
        }
    }
    return choice;
}

QStringList NicBind::discoveryBroadcasts(const NicChoice &choice,
                                         const QList<NicRecord> &nics,
                                         bool *includeLimited)
{
    if (includeLimited) {
        *includeLimited = choice.any;
    }
    QStringList out;
    if (!choice.any && !choice.found) {
        return out;
    }
    for (int i = 0; i < nics.size(); ++i) {
        const NicRecord &record = nics.at(i);
        if (record.broadcast.isEmpty()) {
            continue;
        }
        if (!choice.any && !sameInterface(record, choice)) {
            continue;
        }
        if (!out.contains(record.broadcast)) {
            out.append(record.broadcast);
        }
    }
    return out;
}

bool NicBind::stickSocket(qintptr descriptor, const QString &name, int index,
                          QString *error)
{
    if (descriptor < 0) {
        if (error) {
            *error = QStringLiteral("socket");
        }
        return false;
    }
#if defined(Q_OS_LINUX)
    const QByteArray raw = name.toUtf8();
    if (raw.isEmpty() || raw.size() >= IFNAMSIZ) {
        if (error) {
            *error = QStringLiteral("name");
        }
        return false;
    }
    if (::setsockopt(static_cast<int>(descriptor), SOL_SOCKET, SO_BINDTODEVICE,
                     raw.constData(), static_cast<socklen_t>(raw.size())) != 0) {
        if (error) {
            *error = QString::fromLocal8Bit(::strerror(errno));
        }
        return false;
    }
    Q_UNUSED(index);
    return true;
#elif defined(Q_OS_MAC)
#ifndef IP_BOUND_IF
#define IP_BOUND_IF 25
#endif
    const QByteArray raw = name.toUtf8();
    const unsigned int ifIndex = if_nametoindex(raw.constData());
    if (ifIndex == 0) {
        if (error) {
            *error = QStringLiteral("name");
        }
        return false;
    }
    if (::setsockopt(static_cast<int>(descriptor), IPPROTO_IP, IP_BOUND_IF,
                     &ifIndex, sizeof(ifIndex)) != 0) {
        if (error) {
            *error = QString::fromLocal8Bit(::strerror(errno));
        }
        return false;
    }
    Q_UNUSED(index);
    return true;
#elif defined(Q_OS_WIN)
    if (index <= 0) {
        if (error) {
            *error = QStringLiteral("index");
        }
        return false;
    }
    const DWORD ifIndex = htonl(static_cast<u_long>(index));
    if (::setsockopt(static_cast<SOCKET>(descriptor), IPPROTO_IP, IP_UNICAST_IF,
                     reinterpret_cast<const char *>(&ifIndex),
                     sizeof(ifIndex)) == SOCKET_ERROR) {
        if (error) {
            *error = QString::number(WSAGetLastError());
        }
        return false;
    }
    Q_UNUSED(name);
    return true;
#else
    Q_UNUSED(name);
    Q_UNUSED(index);
    if (error) {
        *error = QStringLiteral("unsupported");
    }
    return false;
#endif
}

QString NicBind::platformNote()
{
#if defined(Q_OS_LINUX)
    return QStringLiteral(
            "Linux listens on the NIC IPv4 and tries SO_BINDTODEVICE. "
            "That option can fail without permission; the IPv4 bind still applies. "
            "There is no raw MAC bind.");
#elif defined(Q_OS_MAC)
    return QStringLiteral(
            "macOS listens on the NIC IPv4 and sets IP_BOUND_IF when the "
            "interface index is known. There is no raw MAC bind.");
#elif defined(Q_OS_WIN)
    return QStringLiteral(
            "Windows listens on the NIC IPv4 and sets IP_UNICAST_IF. "
            "Winsock cannot bind a socket to a MAC address.");
#else
    return QStringLiteral(
            "This system listens on the selected IPv4. MAC bind is not available.");
#endif
}
