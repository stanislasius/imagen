// SPDX-License-Identifier: MIT
#include "overlays/SelectionOverlay.h"

#include "common/Logging.h"

#include <QGuiApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>

namespace imagen {
namespace {

/// Union of all screen geometries in logical coordinates.
QRect desktopGeometry()
{
    QRect bounds;
    const QList<QScreen *> screens = QGuiApplication::screens();
    for (QScreen *screen : screens)
        bounds = bounds.united(screen->geometry());
    return bounds;
}

} // namespace

SelectionOverlay::SelectionOverlay(QWidget *parent)
    : OverlayWindow(parent)
{
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
    setFocusPolicy(Qt::StrongFocus);
}

void SelectionOverlay::beginSelection()
{
    const QRect geometry = desktopGeometry();
    setGeometry(geometry);
    m_selection = QRect();
    m_dragging = false;

    setWindowFlag(Qt::WindowStaysOnTopHint, true);
    show();
    raise();
    activateWindow();
    setFocus(Qt::ActiveWindowFocusReason);
    update();
}

void SelectionOverlay::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        OverlayWindow::mousePressEvent(event);
        return;
    }
    m_dragging = true;
    m_selection = QRect(event->position().toPoint(), event->position().toPoint());
    update();
}

void SelectionOverlay::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_dragging) {
        OverlayWindow::mouseMoveEvent(event);
        return;
    }
    m_selection = QRect(m_selection.topLeft(), event->position().toPoint()).normalized();
    update();
}

void SelectionOverlay::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_dragging || event->button() != Qt::LeftButton) {
        OverlayWindow::mouseReleaseEvent(event);
        return;
    }
    m_dragging = false;
    const QRect selection = m_selection.normalized();
    hide();

    if (selection.width() < 2 || selection.height() < 2) {
        Q_EMIT selectionCancelled();
        return;
    }
    Q_EMIT regionSelected(selection);
}

void SelectionOverlay::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        m_dragging = false;
        hide();
        Q_EMIT selectionCancelled();
        return;
    }
    OverlayWindow::keyPressEvent(event);
}

void SelectionOverlay::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor(0, 0, 0, 90));

    if (m_selection.isNull())
        return;

    const QRect selection = m_selection.normalized();
    painter.fillRect(selection, QColor(0, 0, 0, 0));
    painter.setPen(QPen(QColor(64, 160, 255), 1));
    painter.drawRect(selection.adjusted(0, 0, -1, -1));
}

} // namespace imagen
