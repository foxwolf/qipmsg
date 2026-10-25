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

#include "doodleCanvas.h"

#include <QHBoxLayout>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>
#include <QVBoxLayout>

static const int kPenWidth = 3;

class DoodleBoard : public QWidget
{
public:
    explicit DoodleBoard(QWidget *parent = 0)
        : QWidget(parent), m_color(Qt::black)
    {
        setMinimumSize(480, 280);
        setAutoFillBackground(true);
        QPalette pal = palette();
        pal.setColor(QPalette::Window, Qt::white);
        setPalette(pal);
    }

    void setColor(const QColor &color) { m_color = color; }

    void clearInk()
    {
        m_strokes.clear();
        update();
    }

    bool empty() const { return m_strokes.isEmpty(); }

    QImage image() const
    {
        return DoodleCanvas::render(size(), m_strokes);
    }

protected:
    void paintEvent(QPaintEvent *)
    {
        QPainter painter(this);
        painter.drawImage(rect(), image());
    }

    void mousePressEvent(QMouseEvent *event)
    {
        if (event->button() != Qt::LeftButton) {
            return;
        }
        DoodleStroke stroke;
        stroke.color = m_color;
        stroke.points << event->pos();
        m_strokes.append(stroke);
        update();
    }

    void mouseMoveEvent(QMouseEvent *event)
    {
        if (!(event->buttons() & Qt::LeftButton) || m_strokes.isEmpty()) {
            return;
        }
        const QPoint pos = event->pos();
        const QVector<QPoint> &points = m_strokes.last().points;
        if (!points.isEmpty() && (points.last() - pos).manhattanLength() < 2) {
            return;
        }
        m_strokes.last().points << pos;
        update();
    }

private:
    QColor m_color;
    QVector<DoodleStroke> m_strokes;
};

DoodleCanvas::DoodleCanvas(QWidget *parent)
    : QDialog(parent), m_board(0)
{
    setWindowTitle(tr("Doodle"));
    setModal(true);

    m_board = new DoodleBoard(this);

    QPushButton *black = new QPushButton(tr("Black"));
    QPushButton *red = new QPushButton(tr("Red"));
    QPushButton *blue = new QPushButton(tr("Blue"));
    QPushButton *green = new QPushButton(tr("Green"));
    QPushButton *clear = new QPushButton(tr("Clear"));
    connect(black, SIGNAL(clicked()), this, SLOT(chooseBlack()));
    connect(red, SIGNAL(clicked()), this, SLOT(chooseRed()));
    connect(blue, SIGNAL(clicked()), this, SLOT(chooseBlue()));
    connect(green, SIGNAL(clicked()), this, SLOT(chooseGreen()));
    connect(clear, SIGNAL(clicked()), this, SLOT(clearInk()));

    QHBoxLayout *colors = new QHBoxLayout;
    colors->addWidget(black);
    colors->addWidget(red);
    colors->addWidget(blue);
    colors->addWidget(green);
    colors->addWidget(clear);
    colors->addStretch();

    QPushButton *send = new QPushButton(tr("Send"));
    QPushButton *cancel = new QPushButton(tr("Cancel"));
    send->setDefault(true);
    connect(send, SIGNAL(clicked()), this, SLOT(acceptDoodle()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

    QHBoxLayout *actions = new QHBoxLayout;
    actions->addStretch();
    actions->addWidget(send);
    actions->addWidget(cancel);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addLayout(colors);
    layout->addWidget(m_board, 1);
    layout->addLayout(actions);
    resize(560, 420);
}

QPixmap DoodleCanvas::edit(QWidget *parent)
{
    DoodleCanvas dialog(parent);
    if (dialog.exec() != QDialog::Accepted) {
        return QPixmap();
    }
    const QImage image = dialog.image();
    if (image.isNull()) {
        return QPixmap();
    }
    return QPixmap::fromImage(image);
}

QImage DoodleCanvas::render(const QSize &size, const QVector<DoodleStroke> &strokes)
{
    const QSize canvas = size.isValid() ? size : QSize(480, 280);
    QImage image(canvas, QImage::Format_RGB32);
    image.fill(Qt::white);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    for (int i = 0; i < strokes.size(); ++i) {
        const DoodleStroke &stroke = strokes.at(i);
        if (stroke.points.isEmpty()) {
            continue;
        }
        QPen pen(stroke.color, kPenWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter.setPen(pen);
        if (stroke.points.size() == 1) {
            painter.drawPoint(stroke.points.at(0));
            continue;
        }
        painter.drawPolyline(stroke.points.constData(), stroke.points.size());
    }
    return image;
}

QImage DoodleCanvas::image() const
{
    if (!m_board) {
        return QImage();
    }
    return m_board->image();
}

void DoodleCanvas::setInk(const QColor &color)
{
    if (m_board) {
        m_board->setColor(color);
    }
}

void DoodleCanvas::chooseBlack() { setInk(Qt::black); }
void DoodleCanvas::chooseRed() { setInk(QColor(198, 40, 40)); }
void DoodleCanvas::chooseBlue() { setInk(QColor(21, 101, 192)); }
void DoodleCanvas::chooseGreen() { setInk(QColor(46, 125, 50)); }

void DoodleCanvas::clearInk()
{
    if (m_board) {
        m_board->clearInk();
    }
}

void DoodleCanvas::acceptDoodle()
{
    if (!m_board || m_board->empty()) {
        QMessageBox::information(this, tr("Doodle"),
                                 tr("Draw something first."));
        return;
    }
    accept();
}
