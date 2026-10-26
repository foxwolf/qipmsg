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

#include "scheduleDialog.h"
#include "scheduleStore.h"
#include "global.h"
#include "user_manager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <algorithm>

namespace {

bool reminderLess(const ScheduleReminder &left, const ScheduleReminder &right)
{
    if (left.done != right.done) {
        return !left.done;
    }
    if (!left.done) {
        return left.due < right.due;
    }
    return left.due > right.due;
}

} // namespace

ScheduleDialog::ScheduleDialog(const QString &preferredPeerIp, QWidget *parent)
    : QDialog(parent), m_preferredPeerIp(preferredPeerIp), m_list(0),
      m_textEdit(0), m_dueEdit(0), m_notifyCheck(0), m_peerCombo(0),
      m_hint(0), m_saveButton(0), m_deleteButton(0)
{
    setWindowTitle(tr("Schedule"));
    setModal(true);
    resize(480, 520);

    m_list = new QListWidget;
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(m_list, SIGNAL(currentRowChanged(int)),
            this, SLOT(applySelection()));

    m_textEdit = new QPlainTextEdit;
    m_textEdit->setPlaceholderText(tr("Reminder text"));
    m_textEdit->setMaximumHeight(90);

    m_dueEdit = new QDateTimeEdit(QDateTime::currentDateTime().addSecs(3600));
    m_dueEdit->setCalendarPopup(true);
    m_dueEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd HH:mm"));

    m_notifyCheck = new QCheckBox(tr("Notify peer with an ordinary message"));
    m_notifyCheck->setToolTip(tr(
            "When this reminder is due, send the text as a normal SENDMSG. "
            "There is no FeiQ clock packet."));
    m_peerCombo = new QComboBox;

    m_hint = new QLabel(tr(
            "Saved in schedule.json on this computer. A reminder rings only "
            "while QIpMsg is running. If the time passes while the program "
            "is quit, the next launch shows it once. There is no background "
            "service, and nothing is uploaded."));
    m_hint->setWordWrap(true);

    m_saveButton = new QPushButton(tr("Save"));
    m_deleteButton = new QPushButton(tr("Delete"));
    QPushButton *newButton = new QPushButton(tr("New"));
    QPushButton *closeButton = new QPushButton(tr("Close"));
    connect(m_saveButton, SIGNAL(clicked()), this, SLOT(saveCurrent()));
    connect(m_deleteButton, SIGNAL(clicked()), this, SLOT(deleteCurrent()));
    connect(newButton, SIGNAL(clicked()), this, SLOT(clearForm()));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));

    QHBoxLayout *dueRow = new QHBoxLayout;
    dueRow->addWidget(new QLabel(tr("When:")));
    dueRow->addWidget(m_dueEdit, 1);

    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addWidget(newButton);
    buttons->addWidget(m_saveButton);
    buttons->addWidget(m_deleteButton);
    buttons->addStretch();
    buttons->addWidget(closeButton);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_list, 1);
    layout->addWidget(m_textEdit);
    layout->addLayout(dueRow);
    layout->addWidget(m_notifyCheck);
    layout->addWidget(m_peerCombo);
    layout->addWidget(m_hint);
    layout->addLayout(buttons);

    if (Global::scheduleStore) {
        connect(Global::scheduleStore, SIGNAL(remindersChanged()),
                this, SLOT(reload()));
    }
    fillPeers(m_preferredPeerIp);
    reload();
    clearForm();
}

void ScheduleDialog::fillPeers(const QString &selectedIp)
{
    const QString keep = selectedIp;
    m_peerCombo->clear();
    m_peerCombo->addItem(tr("(no peer)"), QString());
    if (!Global::userManager) {
        return;
    }
    // One combo entry per person, on the address chat would use.
    // A reminder that still names the other NIC selects that person
    // instead of adding a second line.
    QString wanted = keep;
    if (!wanted.isEmpty()) {
        const int preferred = Global::userManager->preferredRowForIp(wanted);
        if (preferred >= 0) {
            const QString preferredIp = Global::userManager->ip(preferred);
            if (!preferredIp.isEmpty()) {
                wanted = preferredIp;
            }
        }
    }
    bool seen = wanted.isEmpty();
    const QList<PeerFoldGroup> folds = Global::userManager->contactFolds();
    for (int g = 0; g < folds.size(); ++g) {
        const int row = folds.at(g).preferred;
        if (row < 0) {
            continue;
        }
        const QString ip = Global::userManager->ip(row);
        if (ip.isEmpty()) {
            continue;
        }
        const QString name = Global::userManager->displayNameForIp(ip);
        m_peerCombo->addItem(QString("%1 (%2)").arg(name, ip), ip);
        if (ip == wanted) {
            seen = true;
        }
    }
    if (!wanted.isEmpty() && !seen) {
        m_peerCombo->addItem(wanted, wanted);
    }
    const int index = m_peerCombo->findData(wanted);
    m_peerCombo->setCurrentIndex(index < 0 ? 0 : index);
}

QString ScheduleDialog::selectedId() const
{
    QListWidgetItem *item = m_list->currentItem();
    if (!item) {
        return QString();
    }
    return item->data(Qt::UserRole).toString();
}

void ScheduleDialog::reload()
{
    const QString keep = selectedId();
    m_list->blockSignals(true);
    m_list->clear();
    QList<ScheduleReminder> rows;
    if (Global::scheduleStore) {
        rows = Global::scheduleStore->items();
    }
    std::sort(rows.begin(), rows.end(), reminderLess);
    int selectRow = -1;
    for (int i = 0; i < rows.size(); ++i) {
        const ScheduleReminder &item = rows.at(i);
        QString title = item.text;
        title.replace(QLatin1Char('\n'), QLatin1Char(' '));
        if (title.size() > 48) {
            title = title.left(47) + QStringLiteral("…");
        }
        QString line = QString("%1  %2").arg(item.due.toString("MM-dd HH:mm"), title);
        if (item.done) {
            line = tr("Done") + QStringLiteral("  ") + line;
        }
        if (item.notifyPeer && !item.peerName.isEmpty()) {
            line += QStringLiteral("  → ") + item.peerName;
        } else if (item.notifyPeer && !item.peerIp.isEmpty()) {
            line += QStringLiteral("  → ") + item.peerIp;
        }
        QListWidgetItem *row = new QListWidgetItem(line, m_list);
        row->setData(Qt::UserRole, item.id);
        if (item.id == keep) {
            selectRow = i;
        }
    }
    m_list->blockSignals(false);
    if (selectRow >= 0) {
        m_list->setCurrentRow(selectRow);
    }
    m_deleteButton->setEnabled(m_list->currentItem() != 0);
}

void ScheduleDialog::applySelection()
{
    const QString id = selectedId();
    m_deleteButton->setEnabled(!id.isEmpty());
    if (id.isEmpty() || !Global::scheduleStore) {
        return;
    }
    const QList<ScheduleReminder> rows = Global::scheduleStore->items();
    for (int i = 0; i < rows.size(); ++i) {
        if (rows.at(i).id != id) {
            continue;
        }
        const ScheduleReminder &item = rows.at(i);
        m_textEdit->setPlainText(item.text);
        m_dueEdit->setDateTime(item.due);
        m_notifyCheck->setChecked(item.notifyPeer);
        const QString ip = item.peerIp.isEmpty() ? m_preferredPeerIp : item.peerIp;
        if (m_peerCombo->findData(ip) < 0 && !ip.isEmpty()) {
            const QString name = item.peerName.isEmpty() ? ip : item.peerName;
            m_peerCombo->addItem(QString("%1 (%2)").arg(name, ip), ip);
        }
        const int index = m_peerCombo->findData(ip);
        m_peerCombo->setCurrentIndex(index < 0 ? 0 : index);
        return;
    }
}

void ScheduleDialog::clearForm()
{
    m_list->setCurrentRow(-1);
    m_textEdit->clear();
    m_dueEdit->setDateTime(QDateTime::currentDateTime().addSecs(3600));
    m_notifyCheck->setChecked(false);
    fillPeers(m_preferredPeerIp);
    m_textEdit->setFocus();
}

void ScheduleDialog::saveCurrent()
{
    if (!Global::scheduleStore) {
        return;
    }
    const QString text = m_textEdit->toPlainText().trimmed();
    if (text.isEmpty()) {
        QMessageBox::information(this, tr("Schedule"),
                                 tr("Enter the reminder text."));
        return;
    }
    const bool notify = m_notifyCheck->isChecked();
    const QString ip = m_peerCombo->currentData().toString();
    if (notify && ip.isEmpty()) {
        QMessageBox::information(this, tr("Schedule"),
                                 tr("Choose a peer, or turn off notify peer."));
        return;
    }
    QString peerName;
    if (!ip.isEmpty() && Global::userManager) {
        peerName = Global::userManager->displayNameForIp(ip);
    }
    if (peerName.isEmpty()) {
        peerName = m_peerCombo->currentText();
    }
    const QString id = selectedId();
    bool ok = false;
    if (id.isEmpty()) {
        const QString newId = Global::scheduleStore->add(
                    text, m_dueEdit->dateTime(), notify, ip, peerName);
        ok = !newId.isEmpty();
        if (!ok) {
            QMessageBox::warning(this, tr("Schedule"),
                                 tr("Could not save the reminder. The list may be full."));
        } else {
            for (int row = 0; row < m_list->count(); ++row) {
                if (m_list->item(row)->data(Qt::UserRole).toString() == newId) {
                    m_list->setCurrentRow(row);
                    break;
                }
            }
        }
    } else {
        ok = Global::scheduleStore->update(id, text, m_dueEdit->dateTime(),
                                           notify, ip, peerName);
        if (!ok) {
            QMessageBox::warning(this, tr("Schedule"),
                                 tr("Could not update the reminder."));
        }
    }
}

void ScheduleDialog::deleteCurrent()
{
    const QString id = selectedId();
    if (id.isEmpty() || !Global::scheduleStore) {
        return;
    }
    if (!Global::scheduleStore->remove(id)) {
        QMessageBox::warning(this, tr("Schedule"),
                             tr("Could not delete the reminder."));
        return;
    }
    clearForm();
}
