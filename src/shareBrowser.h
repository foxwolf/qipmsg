// This file is part of QIpMsg.
//
// QIpMsg is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#ifndef SHAREBROWSER_H
#define SHAREBROWSER_H

#include "shareFolder.h"

#include <QDialog>
#include <QString>

class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;

class ShareBrowser : public QDialog
{
    Q_OBJECT

public:
    explicit ShareBrowser(const QString &ip, QWidget *parent = 0);
    ShareBrowser(const QString &ip, const QString &groupId, QWidget *parent = 0);

private slots:
    void refreshList();
    void goUp();
    void enterRow(int row, int column);
    void downloadSelected();

private:
    void init();
    bool peerReady() const;
    QByteArray makeRequest(const QString &verb, const QString &relative) const;
    bool fetchList();
    bool downloadEntry(const ShareEntry &entry);
    QByteArray exchange(const QByteArray &request, QString *error,
                        const QString &savePath, qint64 *wrote);
    QString currentRelative() const;

    QString m_ip;
    QString m_groupId;
    QString m_relative;
    QString m_password;
    bool m_busy;
    QLabel *m_status;
    QLabel *m_pathLabel;
    QLineEdit *m_passwordEdit;
    QPushButton *m_upButton;
    QPushButton *m_refreshButton;
    QPushButton *m_downloadButton;
    QTableWidget *m_table;
    QList<ShareEntry> m_entries;
};

#endif // !SHAREBROWSER_H
