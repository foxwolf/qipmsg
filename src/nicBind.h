// This file is part of QIpMsg.
//
// QIpMsg is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef NICBIND_H
#define NICBIND_H

#include <QList>
#include <QString>
#include <QtGlobal>

// IPv4 NIC choice. Sockets bind an address, not a MAC. The MAC (or the
// interface name when the MAC is empty) is the persisted interface id.
// Linux may also set SO_BINDTODEVICE, macOS IP_BOUND_IF, Windows
// IP_UNICAST_IF. See BUILD.md.

struct NicRecord
{
    QString name;
    QString humanName;
    QString mac;
    QString ipv4;
    QString broadcast;
    int index;

    NicRecord() : index(0) {}
};

struct NicChoice
{
    bool any;
    bool found;
    QString ipv4;
    QString mac;
    QString name;
    QString humanName;
    QString interfaceId;
    int index;

    NicChoice()
        : any(true), found(false), index(0) {}
};

class NicBind
{
public:
    static QString normalizeMac(const QString &raw);
    static QString interfaceId(const QString &mac, const QString &name);
    static bool sameInterface(const NicRecord &record, const NicChoice &choice);

    static QList<NicRecord> localInterfaces();
    static NicChoice resolve(const QString &savedMac, const QString &savedName,
                             const QString &savedIpv4,
                             const QList<NicRecord> &nics);
    static QStringList discoveryBroadcasts(const NicChoice &choice,
                                           const QList<NicRecord> &nics,
                                           bool *includeLimited);

    static bool stickSocket(qintptr descriptor, const QString &name, int index,
                            QString *error);
    static QString platformNote();
};

#endif // !NICBIND_H
