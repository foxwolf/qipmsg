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

#include "groupPolicyDialog.h"
#include "global.h"
#include "groupPolicy.h"
#include "preferences.h"
#include "user_manager.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QSize>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>

GroupPolicyDialog::GroupPolicyDialog(QWidget *parent)
    : QDialog(parent), m_tree(0), m_hint(0), m_filling(false)
{
    setWindowTitle(tr("Department group permissions"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    resize(520, 420);

    m_hint = new QLabel(tr(
            "These are FeiQ department groups (the group name in entry packets), "
            "not local discussion groups. Hide skips discovery answers for that "
            "group and, while any group is hidden, announces presence by unicast "
            "instead of a subnet broadcast. Mute keeps history and the unread "
            "count, but does not raise a window, play a sound, or show a tray balloon. "
            "IPMSG has no per-group hide bit, so a peer that already cached you "
            "may keep showing you until it accepts a unicast leave."));
    m_hint->setWordWrap(true);

    m_tree = new QTreeWidget;
    m_tree->setColumnCount(3);
    m_tree->setHeaderLabels(QStringList()
                            << tr("Group")
                            << tr("Hide from group")
                            << tr("Mute messages"));
    m_tree->setRootIsDecorated(false);
    m_tree->header()->setStretchLastSection(false);
    m_tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_tree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    QPushButton *refreshButton = new QPushButton(tr("Refresh"));
    connect(refreshButton, SIGNAL(clicked()), this, SLOT(refill()));
    QPushButton *closeButton = new QPushButton(tr("Close"));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));

    QHBoxLayout *buttons = new QHBoxLayout;
    buttons->addWidget(refreshButton);
    buttons->addStretch();
    buttons->addWidget(closeButton);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_hint);
    layout->addWidget(m_tree, 1);
    layout->addLayout(buttons);

    if (Global::groupPolicy) {
        // Queued so a checkbox toggle can finish before the tree is rebuilt.
        connect(Global::groupPolicy, SIGNAL(policyChanged()),
                this, SLOT(refill()), Qt::QueuedConnection);
    }
    refill();
}

QStringList GroupPolicyDialog::knownGroups() const
{
    QStringList names;
    if (Global::preferences) {
        foreach (const QString &name, Global::preferences->groupNameList) {
            const QString key = GroupPolicy::normalizeName(name);
            if (!key.isEmpty() && !names.contains(key)) {
                names << key;
            }
        }
        const QString mine = GroupPolicy::normalizeName(Global::preferences->groupName);
        if (!mine.isEmpty() && !names.contains(mine)) {
            names << mine;
        }
    }
    if (Global::userManager) {
        for (int row = 0; row < Global::userManager->userCount(); ++row) {
            const QString key = GroupPolicy::normalizeName(
                        Global::userManager->group(row));
            if (!key.isEmpty() && !names.contains(key)) {
                names << key;
            }
        }
    }
    if (Global::groupPolicy) {
        foreach (const QString &name, Global::groupPolicy->hiddenGroups()) {
            if (!names.contains(name)) {
                names << name;
            }
        }
        foreach (const QString &name, Global::groupPolicy->mutedGroups()) {
            if (!names.contains(name)) {
                names << name;
            }
        }
    }
    names.sort();
    return names;
}

void GroupPolicyDialog::refill()
{
    if (m_filling || !m_tree) {
        return;
    }
    m_filling = true;
    m_tree->clear();

    foreach (const QString &name, knownGroups()) {
        QTreeWidgetItem *item = new QTreeWidgetItem(m_tree);
        item->setText(0, name);
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        item->setSizeHint(0, QSize(220, 28));

        QCheckBox *hideBox = new QCheckBox(tr("Hide"));
        QCheckBox *muteBox = new QCheckBox(tr("Mute"));
        if (Global::groupPolicy) {
            hideBox->setChecked(Global::groupPolicy->isHidden(name));
            muteBox->setChecked(Global::groupPolicy->isMuted(name));
        }
        hideBox->setToolTip(tr("Do not announce presence to peers in this group"));
        muteBox->setToolTip(tr("Keep history, but do not raise or notify"));
        m_tree->setItemWidget(item, 1, hideBox);
        m_tree->setItemWidget(item, 2, muteBox);

        connect(hideBox, &QCheckBox::toggled, this, [this, name](bool on) {
            if (m_filling || !Global::groupPolicy) {
                return;
            }
            if (Global::groupPolicy->setHidden(name, on) && Global::userManager) {
                Global::userManager->refreshGroupVisibility(name);
            }
        });
        connect(muteBox, &QCheckBox::toggled, this, [this, name](bool on) {
            if (m_filling || !Global::groupPolicy) {
                return;
            }
            Global::groupPolicy->setMuted(name, on);
        });
    }
    m_filling = false;
}
