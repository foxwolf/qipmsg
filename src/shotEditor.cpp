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

#include "shotEditor.h"

#include <QButtonGroup>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineF>
#include <QMouseEvent>
#include <QPainter>
#include <QPolygon>
#include <QPushButton>
#include <QScreen>
#include <QShortcut>
#include <QVBoxLayout>

namespace {

void paintArrow(QPainter *painter, const QPoint &anchor, const QPoint &tip,
                const QColor &color, int width)
{
    painter->setPen(QPen(color, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->setBrush(color);
    painter->drawLine(anchor, tip);
    const QLineF shaft(anchor, tip);
    const qreal head = qMax(12.0, width * 4.0);
    QLineF left = QLineF::fromPolar(head, shaft.angle() + 180.0 - 28.0);
    QLineF right = QLineF::fromPolar(head, shaft.angle() + 180.0 + 28.0);
    left.translate(tip);
    right.translate(tip);
    QPolygon headPoly;
    headPoly << tip << left.p2().toPoint() << right.p2().toPoint();
    painter->drawPolygon(headPoly);
}

} // namespace

ShotCanvas::ShotCanvas(const QPixmap &source, QWidget *parent)
    : QWidget(parent), m_scale(1.0), m_tool(ShotToolPen),
      m_color(QColor(220, 38, 38)), m_width(4), m_drawing(false)
{
    QImage image = source.toImage();
    m_base = QPixmap::fromImage(image);
    m_base.setDevicePixelRatio(1.0);
    if (m_base.isNull()) {
        m_base = QPixmap(32, 32);
        m_base.fill(Qt::white);
    }

    QSize avail(1100, 680);
    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        avail = screen->availableGeometry().size() - QSize(80, 200);
        if (avail.width() < 320) {
            avail.setWidth(320);
        }
        if (avail.height() < 240) {
            avail.setHeight(240);
        }
    }
    if (m_base.width() > avail.width() || m_base.height() > avail.height()) {
        m_scale = qMin(double(avail.width()) / double(m_base.width()),
                       double(avail.height()) / double(m_base.height()));
    }
    if (m_scale < 0.05) {
        m_scale = 0.05;
    }
    setFixedSize(qMax(1, qRound(m_base.width() * m_scale)),
                 qMax(1, qRound(m_base.height() * m_scale)));
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
}

QPoint ShotCanvas::toImage(const QPoint &widgetPos) const
{
    const int x = qBound(0, qRound(widgetPos.x() / m_scale), m_base.width() - 1);
    const int y = qBound(0, qRound(widgetPos.y() / m_scale), m_base.height() - 1);
    return QPoint(x, y);
}

void ShotCanvas::setTool(int tool)
{
    m_tool = tool;
    m_drawing = false;
}

bool ShotCanvas::canUndo() const
{
    return !m_strokes.isEmpty();
}

void ShotCanvas::setColor(const QColor &color)
{
    if (color.isValid()) {
        m_color = color;
    }
}

void ShotCanvas::setWidth(int width)
{
    m_width = qBound(1, width, 32);
}

void ShotCanvas::undo()
{
    if (m_strokes.isEmpty()) {
        return;
    }
    m_strokes.removeLast();
    m_drawing = false;
    update();
    emit changed();
}

void ShotCanvas::commitStroke(const Stroke &stroke)
{
    m_strokes.append(stroke);
    if (m_strokes.size() > 200) {
        m_strokes.removeFirst();
    }
    emit changed();
}

void ShotCanvas::paintStroke(QPainter *painter, const Stroke &stroke) const
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    QPen pen(stroke.color, stroke.width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    if (stroke.tool == ShotToolPen) {
        if (stroke.points.size() == 1) {
            painter->drawPoint(stroke.points.first());
        } else if (stroke.points.size() > 1) {
            painter->drawPolyline(stroke.points.constData(), stroke.points.size());
        }
    } else if (stroke.tool == ShotToolRect) {
        painter->drawRect(QRect(stroke.anchor, stroke.tip).normalized());
    } else if (stroke.tool == ShotToolArrow) {
        paintArrow(painter, stroke.anchor, stroke.tip, stroke.color, stroke.width);
    } else if (stroke.tool == ShotToolText && !stroke.text.isEmpty()) {
        QFont font = painter->font();
        font.setBold(true);
        font.setPixelSize(qMax(16, stroke.width * 5));
        painter->setFont(font);
        painter->drawText(stroke.anchor + QPoint(0, font.pixelSize()), stroke.text);
    }
}

void ShotCanvas::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.scale(m_scale, m_scale);
    painter.drawPixmap(0, 0, m_base);
    for (int i = 0; i < m_strokes.size(); ++i) {
        paintStroke(&painter, m_strokes.at(i));
    }
    if (m_drawing) {
        paintStroke(&painter, m_active);
    }
}

void ShotCanvas::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }
    const QPoint at = toImage(event->pos());
    if (m_tool == ShotToolText) {
        bool ok = false;
        const QString text = QInputDialog::getText(
                    this, tr("Text"), tr("Annotation text:"),
                    QLineEdit::Normal, QString(), &ok);
        if (ok && !text.trimmed().isEmpty()) {
            Stroke stroke;
            stroke.tool = ShotToolText;
            stroke.color = m_color;
            stroke.width = m_width;
            stroke.anchor = at;
            stroke.tip = at;
            stroke.text = text.trimmed();
            commitStroke(stroke);
            update();
        }
        return;
    }

    m_drawing = true;
    m_active = Stroke();
    m_active.tool = m_tool;
    m_active.color = m_color;
    m_active.width = m_width;
    m_active.anchor = at;
    m_active.tip = at;
    m_active.points.append(at);
    update();
}

void ShotCanvas::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_drawing) {
        return;
    }
    const QPoint at = toImage(event->pos());
    m_active.tip = at;
    if (m_tool == ShotToolPen) {
        if (m_active.points.isEmpty() || m_active.points.last() != at) {
            m_active.points.append(at);
        }
    }
    update();
}

void ShotCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_drawing || event->button() != Qt::LeftButton) {
        return;
    }
    m_active.tip = toImage(event->pos());
    if (m_tool == ShotToolPen && (m_active.points.isEmpty()
                                  || m_active.points.last() != m_active.tip)) {
        m_active.points.append(m_active.tip);
    }
    m_drawing = false;
    commitStroke(m_active);
    update();
}

QPixmap ShotCanvas::render() const
{
    QPixmap out(m_base.size());
    out.fill(Qt::white);
    QPainter painter(&out);
    painter.drawPixmap(0, 0, m_base);
    for (int i = 0; i < m_strokes.size(); ++i) {
        paintStroke(&painter, m_strokes.at(i));
    }
    return out;
}

ShotEditor::ShotEditor(const QPixmap &source, QWidget *parent)
    : QDialog(parent), m_canvas(0), m_undoButton(0), m_colorBox(0), m_widthBox(0)
{
    setWindowTitle(tr("Annotate screenshot"));
    setWindowModality(Qt::ApplicationModal);

    m_canvas = new ShotCanvas(source, this);

    QPushButton *penButton = new QPushButton(tr("Pen"), this);
    QPushButton *rectButton = new QPushButton(tr("Rectangle"), this);
    QPushButton *arrowButton = new QPushButton(tr("Arrow"), this);
    QPushButton *textButton = new QPushButton(tr("Text"), this);
    penButton->setCheckable(true);
    rectButton->setCheckable(true);
    arrowButton->setCheckable(true);
    textButton->setCheckable(true);
    penButton->setChecked(true);
    QButtonGroup *tools = new QButtonGroup(this);
    tools->addButton(penButton, ShotToolPen);
    tools->addButton(rectButton, ShotToolRect);
    tools->addButton(arrowButton, ShotToolArrow);
    tools->addButton(textButton, ShotToolText);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(tools, &QButtonGroup::idClicked, m_canvas, &ShotCanvas::setTool);
#else
    connect(tools, SIGNAL(buttonClicked(int)), m_canvas, SLOT(setTool(int)));
#endif

    m_colorBox = new QComboBox(this);
    m_colorBox->addItem(tr("Red"), QColor(220, 38, 38));
    m_colorBox->addItem(tr("Orange"), QColor(234, 88, 12));
    m_colorBox->addItem(tr("Yellow"), QColor(202, 138, 4));
    m_colorBox->addItem(tr("Green"), QColor(22, 163, 74));
    m_colorBox->addItem(tr("Blue"), QColor(37, 99, 235));
    m_colorBox->addItem(tr("White"), QColor(255, 255, 255));
    m_colorBox->addItem(tr("Black"), QColor(17, 24, 39));
    m_widthBox = new QComboBox(this);
    m_widthBox->addItem(tr("Thin"), 2);
    m_widthBox->addItem(tr("Medium"), 4);
    m_widthBox->addItem(tr("Thick"), 8);
    m_widthBox->setCurrentIndex(1);

    m_undoButton = new QPushButton(tr("Undo"), this);
    m_undoButton->setEnabled(false);
    connect(m_undoButton, SIGNAL(clicked()), m_canvas, SLOT(undo()));
    connect(m_canvas, SIGNAL(changed()), this, SLOT(syncUndo()));
    connect(new QShortcut(QKeySequence::Undo, this), SIGNAL(activated()),
            m_canvas, SLOT(undo()));

    QHBoxLayout *toolsLayout = new QHBoxLayout;
    toolsLayout->addWidget(penButton);
    toolsLayout->addWidget(rectButton);
    toolsLayout->addWidget(arrowButton);
    toolsLayout->addWidget(textButton);
    toolsLayout->addWidget(m_colorBox);
    toolsLayout->addWidget(m_widthBox);
    toolsLayout->addWidget(m_undoButton);
    toolsLayout->addStretch();

    QLabel *hint = new QLabel(tr("Draw on the capture, then Send. Esc cancels. "
                                 "The picture is still sent as a PNG attachment."));
    hint->setWordWrap(true);

    QDialogButtonBox *buttons = new QDialogButtonBox(
                QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("Send"));
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addLayout(toolsLayout);
    layout->addWidget(hint);
    layout->addWidget(m_canvas, 0, Qt::AlignCenter);
    layout->addWidget(buttons);

    connect(m_colorBox, SIGNAL(currentIndexChanged(int)), this, SLOT(syncUndo()));
    connect(m_widthBox, SIGNAL(currentIndexChanged(int)), this, SLOT(syncUndo()));
}

void ShotEditor::syncUndo()
{
    if (!m_canvas) {
        return;
    }
    if (m_colorBox) {
        m_canvas->setColor(m_colorBox->currentData().value<QColor>());
    }
    if (m_widthBox) {
        m_canvas->setWidth(m_widthBox->currentData().toInt());
    }
    if (m_undoButton) {
        m_undoButton->setEnabled(m_canvas->canUndo());
    }
}

QPixmap ShotEditor::resultPixmap() const
{
    return m_canvas ? m_canvas->render() : QPixmap();
}

QPixmap ShotEditor::edit(const QPixmap &source, bool *cancelled, QWidget *parent)
{
    if (cancelled) {
        *cancelled = false;
    }
    if (source.isNull()) {
        return QPixmap();
    }
    ShotEditor dialog(source, parent);
    if (dialog.exec() != QDialog::Accepted) {
        if (cancelled) {
            *cancelled = true;
        }
        return QPixmap();
    }
    const QPixmap result = dialog.resultPixmap();
    if (result.isNull() && cancelled) {
        *cancelled = false;
    }
    return result;
}
