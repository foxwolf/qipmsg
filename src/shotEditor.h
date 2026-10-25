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

#ifndef SHOTEDITOR_H
#define SHOTEDITOR_H

#include <QColor>
#include <QDialog>
#include <QPixmap>
#include <QPoint>
#include <QVector>

class QComboBox;
class QMouseEvent;
class QPainter;
class QPaintEvent;
class QPushButton;

enum ShotTool {
    ShotToolPen = 0,
    ShotToolRect = 1,
    ShotToolArrow = 2,
    ShotToolText = 3
};

class ShotCanvas : public QWidget
{
    Q_OBJECT

public:
    explicit ShotCanvas(const QPixmap &source, QWidget *parent = 0);

    QPixmap render() const;
    bool canUndo() const;
    void setColor(const QColor &color);
    void setWidth(int width);

public slots:
    void undo();
    void setTool(int tool);

signals:
    void changed();

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    struct Stroke {
        int tool;
        QColor color;
        int width;
        QVector<QPoint> points;
        QPoint anchor;
        QPoint tip;
        QString text;
    };

    QPoint toImage(const QPoint &widgetPos) const;
    void paintStroke(QPainter *painter, const Stroke &stroke) const;
    void commitStroke(const Stroke &stroke);

    QPixmap m_base;
    qreal m_scale;
    int m_tool;
    QColor m_color;
    int m_width;
    bool m_drawing;
    Stroke m_active;
    QVector<Stroke> m_strokes;
};

class ShotEditor : public QDialog
{
    Q_OBJECT

public:
    static QPixmap edit(const QPixmap &source, bool *cancelled, QWidget *parent);

private slots:
    void syncUndo();

private:
    explicit ShotEditor(const QPixmap &source, QWidget *parent = 0);
    QPixmap resultPixmap() const;

    ShotCanvas *m_canvas;
    QPushButton *m_undoButton;
    QComboBox *m_colorBox;
    QComboBox *m_widthBox;
};

#endif // !SHOTEDITOR_H
