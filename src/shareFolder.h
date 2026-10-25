// This file is part of QIpMsg.
//
// QIpMsg is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef SHAREFOLDER_H
#define SHAREFOLDER_H

#include <QByteArray>
#include <QList>
#include <QString>

// qipmsg-to-qipmsg shared folder. FeiQ's share download packets are not
// in the IPMSG spec and were not verified, so this path is private:
// TCP command 126 (0x7e) on the file port, UTF-8 hex fields, path
// confined to the configured root.

struct ShareEntry
{
    QString name;
    bool directory;
    qint64 size;
    qint64 mtime;
};

struct ShareAnswer
{
    QByteArray payload;
    QString filePath;
    qint64 offset;
    qint64 fileSize;
    bool sendFile;
};

class ShareFolder
{
public:
    static const int maxListEntries;

    static QString localToken(bool enabled, const QString &path,
                              const QString &password);
    static bool tokenIsQipmsg(const QString &token);
    static bool tokenHasShare(const QString &token);
    static bool tokenHasLock(const QString &token);

    static bool passwordMatches(const QString &expected, const QString &given);

    static bool resolveInside(const QString &root, const QString &relative,
                               QString *absolute, QString *error);

    static bool listDirectory(const QString &root, const QString &relative,
                               QList<ShareEntry> *entries, bool *truncated,
                               QString *error);

    static QByteArray encodeList(const QList<ShareEntry> &entries,
                                 bool truncated);
    static bool decodeList(const QByteArray &payload,
                           QList<ShareEntry> *entries, bool *truncated,
                           QString *error);

    static QByteArray buildRequest(const QString &verb, const QString &relative,
                                   const QString &password, qint64 offset);
    static QByteArray buildGroupRequest(const QString &verb, const QString &relative,
                                        const QString &groupId, qint64 offset);
    static bool parseRequest(const QByteArray &packet, QString *verb,
                             QString *relative, QString *password,
                             qint64 *offset, QString *groupId = 0);

    static QString groupShareLine(const QString &groupId);
    static QString groupIdFromText(const QString &text);
    static bool groupIdOk(const QString &groupId);

    static ShareAnswer answer(const QByteArray &request, bool enabled,
                              const QString &root,
                              const QString &expectedPassword);
    static ShareAnswer answerGroup(const QByteArray &request, const QString &root,
                                   bool memberAllowed);
};

#endif // !SHAREFOLDER_H
