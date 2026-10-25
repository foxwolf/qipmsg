// This file is part of QIpMsg.
//
// QIpMsg is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef USERSKIN_H
#define USERSKIN_H

#include <QColor>
#include <QList>
#include <QString>

// User theme pack under ~/.qipmsg/skins/<id>/ or ~/.qipmsg/skins/<id>.zip.
// See BUILD.md. No Qt Sql. A broken QSS does not replace the built-in sheet.

struct SkinInfo
{
    QString id;
    QString name;
    QString base;
};

struct SkinPack
{
    bool found;
    QString id;
    QString name;
    QString base;
    bool qssOk;
    QString qss;
    QString qssProblem;
    QString backgroundPath;
    QColor window;
    QColor windowText;
    QColor baseColor;
    QColor text;
    QColor button;
    QColor buttonText;
    QColor highlight;
    QColor highlightedText;
    QColor toolTipBase;
    QColor toolTipText;
    QColor placeholder;
    QColor bubbleInBg;
    QColor bubbleInFg;
    QColor bubbleOutBg;
    QColor bubbleOutFg;
    QColor bubbleMeta;
    bool colorsFromPack;

    SkinPack()
        : found(false), qssOk(false), colorsFromPack(false) {}
};

class UserSkin
{
public:
    static void setRootForTest(const QString &dir);
    static QString rootDir();

    static QList<SkinInfo> installed();
    static SkinPack load(const QString &id);
    static bool qssUsable(const QString &qss, QString *problem);

    static bool importDirectory(const QString &sourceDir, QString *idOut,
                                QString *error);
    static bool importZip(const QString &zipPath, QString *idOut,
                          QString *error);

    static QString lastProblem();
};

#endif // !USERSKIN_H
