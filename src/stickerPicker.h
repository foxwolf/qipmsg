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

#ifndef STICKER_PICKER_H
#define STICKER_PICKER_H

#include <QDialog>

class QComboBox;
class QGridLayout;
class QLabel;
class QPushButton;
class QWidget;

class StickerPicker : public QDialog
{
    Q_OBJECT

public:
    explicit StickerPicker(QWidget *parent = 0);

signals:
    void unicodeChosen(QString glyph);
    void stickerChosen(QString path, QString title);

private slots:
    void rebuildGrid();
    void importStickers();
    void createGroup();
    void renameCurrentGroup();
    void deleteCurrentGroup();

private:
    void refillGroups(const QString &selectId);
    bool showingBuiltin() const;
    QString currentGroupId() const;
    void updateGroupButtons();

    QComboBox *m_groupCombo;
    QWidget *m_gridHost;
    QGridLayout *m_grid;
    QLabel *m_status;
    QPushButton *m_renameGroupButton;
    QPushButton *m_deleteGroupButton;
    bool m_filling;
};

#endif // !STICKER_PICKER_H
