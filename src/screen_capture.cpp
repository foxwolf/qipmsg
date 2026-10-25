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

#include "screen_capture.h"
#include "helper.h"
#include "windowSnap.h"

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QEventLoop>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>

ScreenCapture::ScreenCapture(const QPixmap &desktop, QWidget *parent)
    : QWidget(parent), m_desktop(desktop), m_dragging(false), m_accepted(false),
      m_snapOff(false)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint
                   | Qt::Tool);
    setCursor(Qt::CrossCursor);
    setMouseTracking(true);
    setAttribute(Qt::WA_DeleteOnClose, false);
    QRect screenRect;
    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        screenRect = screen->geometry();
        setGeometry(screenRect);
    }
    m_snapTargets = WindowSnap::topLevelRects(screenRect, 0);
}

QPixmap ScreenCapture::capturePrimaryScreen()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        return QPixmap();
    }
    return screen->grabWindow(0);
}

QPixmap ScreenCapture::captureRegion()
{
    const QPixmap desktop = capturePrimaryScreen();
    if (desktop.isNull()) {
        return QPixmap();
    }

    ScreenCapture overlay(desktop);
    QEventLoop loop;
    QObject::connect(&overlay, SIGNAL(finished()), &loop, SLOT(quit()));
    overlay.showFullScreen();
    overlay.raise();
    overlay.activateWindow();
    loop.exec();

    if (!overlay.m_accepted) {
        return QPixmap();
    }
    return overlay.selectedPixmap();
}

QString ScreenCapture::savePixmap(const QPixmap &pixmap, const QString &prefix)
{
    if (pixmap.isNull()) {
        return QString();
    }
    QDir dir(Helper::screenshotDir());
    dir.mkpath(".");
    const QString name = QString("%1-%2.png")
            .arg(prefix)
            .arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss"));
    const QString path = dir.filePath(name);
    if (!pixmap.save(path, "PNG")) {
        return QString();
    }
    return path;
}

QPixmap ScreenCapture::selectedPixmap() const
{
    QRect rect = m_rect.normalized();
    if (rect.width() < 2 || rect.height() < 2) {
        return QPixmap();
    }
    const qreal dpr = m_desktop.devicePixelRatio();
    QRect source(qRound(rect.x() * dpr), qRound(rect.y() * dpr),
                 qRound(rect.width() * dpr), qRound(rect.height() * dpr));
    source = source.intersected(m_desktop.rect());
    return m_desktop.copy(source);
}

void ScreenCapture::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(rect(), m_desktop);
    painter.fillRect(rect(), QColor(0, 0, 0, 90));
    if (m_rect.isValid()) {
        const QRect box = m_rect.normalized();
        painter.drawPixmap(box, m_desktop, box);
        painter.setPen(QPen(QColor(0, 180, 255), 1));
        painter.drawRect(box.adjusted(0, 0, -1, -1));
        painter.setPen(Qt::white);
        painter.drawText(10, 20,
                         m_snapOff
                         ? tr("Snap is off. Release Alt to snap again. "
                              "Enter/double-click captures, Esc cancels.")
                         : tr("Drag to select. Snaps to nearby windows. "
                              "Hold Alt to turn snap off. Enter/double-click "
                              "captures, Esc cancels."));
    } else {
        painter.setPen(Qt::white);
        painter.drawText(10, 20,
                         tr("Drag to select a region. It snaps to nearby "
                            "windows. Hold Alt to turn snap off. Esc cancels."));
    }
}

bool ScreenCapture::snapDisabled(Qt::KeyboardModifiers mods) const
{
    return m_snapOff || (mods & Qt::AltModifier) != 0;
}

void ScreenCapture::updateSelection(const QPoint &pos, Qt::KeyboardModifiers mods)
{
    m_cursor = pos;
    if (m_dragging) {
        const QRect raw(m_origin, pos);
        m_rect = WindowSnap::snapSelection(raw, m_snapTargets, 12, snapDisabled(mods));
    }
    update();
}

void ScreenCapture::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_origin = event->pos();
        m_cursor = event->pos();
        m_rect = QRect(m_origin, QSize());
        update();
    }
}

void ScreenCapture::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging) {
        updateSelection(event->pos(), event->modifiers());
    }
}

void ScreenCapture::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_dragging) {
        updateSelection(event->pos(), event->modifiers());
        m_dragging = false;
    }
}

void ScreenCapture::mouseDoubleClickEvent(QMouseEvent *)
{
    finish(m_rect.normalized().width() > 2);
}

void ScreenCapture::finish(bool accepted)
{
    m_accepted = accepted;
    hide();
    emit finished();
}

void ScreenCapture::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        finish(false);
    } else if (event->key() == Qt::Key_Return
               || event->key() == Qt::Key_Enter) {
        finish(m_rect.normalized().width() > 2);
    } else if (event->key() == Qt::Key_Alt) {
        m_snapOff = true;
        updateSelection(m_cursor, event->modifiers());
    }
}

void ScreenCapture::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Alt) {
        m_snapOff = false;
        updateSelection(m_cursor, event->modifiers() & ~Qt::AltModifier);
    }
}
