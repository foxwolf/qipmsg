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

#include "dataBackup.h"
#include "avatarStore.h"
#include "global.h"
#include "helper.h"
#include "lockfile.h"
#include "preferences.h"
#include "user_manager.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QTimer>

namespace {


void flushSettings()
{
    if (Global::preferences) {
        Global::preferences->save();
    }
    if (Global::settings) {
        Global::settings->sync();
    }
}

void reloadSettingsFromDisk()
{
    if (!Global::settings || !Global::preferences) {
        return;
    }
    const QString fileName = Global::settings->fileName();
    delete Global::settings;
    Global::settings = new QSettings(fileName, QSettings::IniFormat);
    Global::preferences->load();
}

bool restartApplication()
{
    const QString program = QCoreApplication::applicationFilePath();
    QStringList args = QCoreApplication::arguments();
    if (!args.isEmpty()) {
        args.removeFirst();
    }
    if (LockFile::instance()) {
        LockFile::instance()->unlock();
    }
    return QProcess::startDetached(program, args);
}

} // namespace

void DataBackup::exportInteractive(QWidget *parent)
{
    QString zipPath = QFileDialog::getSaveFileName(
                parent,
                DataBackup::tr("Backup private data"),
                QDir::homePath() + QLatin1String("/qipmsg-backup.zip"),
                DataBackup::tr("Zip archive (*.zip)"));
    if (zipPath.isEmpty()) {
        return;
    }
    if (!zipPath.endsWith(QLatin1String(".zip"), Qt::CaseInsensitive)) {
        zipPath.append(QLatin1String(".zip"));
    }

    flushSettings();
    QString error;
    int count = 0;
    if (!exportTree(Helper::appHomePath(), zipPath, &error, &count)) {
        QMessageBox::warning(parent, DataBackup::tr("Backup private data"),
                             error.isEmpty()
                             ? DataBackup::tr("Could not write the backup.")
                             : error);
        return;
    }
    QMessageBox::information(
                parent, DataBackup::tr("Backup private data"),
                DataBackup::tr("Saved %1 files to %2.\n\n"
                         "Included: preferences, peers, history, stickers, "
                         "group policy, recent contacts, resume data, "
                         "discussions, the offline queue, and local avatars. "
                         "Received files and screenshots are not included.")
                .arg(count)
                .arg(zipPath));
}

void DataBackup::importInteractive(QWidget *parent)
{
    const QString zipPath = QFileDialog::getOpenFileName(
                parent,
                DataBackup::tr("Restore private data"),
                QDir::homePath(),
                DataBackup::tr("Zip archive (*.zip)"));
    if (zipPath.isEmpty()) {
        return;
    }

    const QString home = Helper::appHomePath();
    const auto answer = QMessageBox::warning(
                parent,
                DataBackup::tr("Restore private data"),
                DataBackup::tr("This replaces matching files under %1.\n\n"
                         "Preferences, peers, history, stickers, group policy, "
                         "recent contacts, resume data, discussions, the offline "
                         "queue, and local avatars in the archive overwrite the "
                         "same paths. Other files, including received files and "
                         "screenshots, stay. This cannot be undone.\n\n"
                         "Continue?")
                .arg(home),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No);
    if (answer != QMessageBox::Yes) {
        return;
    }

    flushSettings();
    QString error;
    int count = 0;
    if (!importTree(home, zipPath, &error, &count)) {
        QMessageBox::warning(parent, DataBackup::tr("Restore private data"),
                             error.isEmpty()
                             ? DataBackup::tr("Could not restore the backup.")
                             : error);
        return;
    }

    reloadSettingsFromDisk();
    if (Global::avatarStore) {
        Global::avatarStore->noteFilesChanged();
    }

    const auto restart = QMessageBox::question(
                parent,
                DataBackup::tr("Restore private data"),
                DataBackup::tr("Restored %1 files. Restart QIpMsg now so this session "
                         "does not write older contacts back over the restored "
                         "files.\n\nRestart now?")
                .arg(count),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::Yes);
    if (restart == QMessageBox::Yes) {
        if (Global::userManager) {
            Global::userManager->suspendPeerPersist();
        }
        if (!restartApplication()) {
            QMessageBox::warning(
                        parent, DataBackup::tr("Restore private data"),
                        DataBackup::tr("Could not start a new copy. Quit QIpMsg and "
                                 "open it again. Avoid changing settings before "
                                 "you quit."));
            return;
        }
        QTimer::singleShot(0, QCoreApplication::instance(), SLOT(quit()));
        return;
    }

    if (Global::userManager) {
        Global::userManager->adoptRestoredPeers();
    }
    QMessageBox::information(
                parent, DataBackup::tr("Restore private data"),
                DataBackup::tr("This session is still using contacts and windows from "
                         "before the restore. Restart when you can. Until then, "
                         "avoid changing stickers, groups, or the offline queue, "
                         "or those files may be saved from this session."));
}
