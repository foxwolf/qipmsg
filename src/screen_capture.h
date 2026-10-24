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

#ifndef SCREEN_CAPTURE_H
#define SCREEN_CAPTURE_H

#include <QPixmap>
#include <QPoint>
#include <QRect>
#include <QString>
#include <QWidget>

class ScreenCapture : public QWidget
{
    Q_OBJECT

public:
    static QPixmap capturePrimaryScreen();
    static QPixmap captureRegion();
    static QString savePixmap(const QPixmap &pixmap, const QString &prefix);

signals:
    void finished();

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

    void finish(bool accepted);

private:
    explicit ScreenCapture(const QPixmap &desktop, QWidget *parent = 0);
    QPixmap selectedPixmap() const;

    QPixmap m_desktop;
    QPoint m_origin;
    QRect m_rect;
    bool m_dragging;
    bool m_accepted;
};

#endif // !SCREEN_CAPTURE_H
