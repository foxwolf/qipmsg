// This file is part of QIpMsg.
//
// QIpMsg is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef FILEHASHINDEX_H
#define FILEHASHINDEX_H

#include <QString>

// Local index of files this process has successfully sent or received.
// Stored as JSON under ~/.qipmsg/filehash.json. No Qt Sql.
// Lookup is by size + SHA-256. Old rows are dropped when the cap is hit.

class FileHashIndex
{
public:
    static QString hashFile(const QString &path, qint64 *sizeOut);

    static void note(const QString &path, const QString &sha256, qint64 size,
                     const QString &kind, const QString &peer);

    // A still-existing file with this size and hash, or empty.
    static QString findExisting(const QString &sha256, qint64 size);

    // Same-name file in dir, or an indexed copy that still lives in dir.
    static QString findInDirectory(const QString &dir, const QString &fileName,
                                   const QString &sha256, qint64 size);

    static void setStoreFile(const QString &path);
    static void setEntryCap(int cap);
    static int entryCount();

private:
    static QString storeFile();
};

#endif // !FILEHASHINDEX_H
