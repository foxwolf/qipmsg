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

#ifndef DOODLECANVAS_H
#define DOODLECANVAS_H

#include <QColor>
#include <QDialog>
#include <QImage>
#include <QPixmap>
#include <QPoint>
#include <QVector>

class DoodleBoard;

struct DoodleStroke
{
    QColor color;
    QVector<QPoint> points;
};

// Pen, a few colors, and clear. The chat window saves the PNG and sends
// it on the existing image attachment path. No tablet pressure.

class DoodleCanvas : public QDialog
{
    Q_OBJECT

public:
    explicit DoodleCanvas(QWidget *parent = 0);

    static QPixmap edit(QWidget *parent);
    static QImage render(const QSize &size, const QVector<DoodleStroke> &strokes);

    QImage image() const;

private slots:
    void chooseBlack();
    void chooseRed();
    void chooseBlue();
    void chooseGreen();
    void clearInk();
    void acceptDoodle();

private:
    void setInk(const QColor &color);

    DoodleBoard *m_board;
};

#endif // !DOODLECANVAS_H
