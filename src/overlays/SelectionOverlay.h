// SPDX-License-Identifier: MIT
#pragma once

#include <QRect>

#include "overlays/OverlayWindow.h"

namespace imagen {

/**
 * Translucent fullscreen helper that lets the user drag a rectangle.
 *
 * Used by the X11 backend, where the client may pick a screen region on
 * its own instead of delegating to the compositor.
 */
class SelectionOverlay : public OverlayWindow
{
    Q_OBJECT

public:
    explicit SelectionOverlay(QWidget *parent = nullptr);

    /// Starts an interactive selection covering the whole virtual desktop.
    void beginSelection();

Q_SIGNALS:
    /// Emitted with the selected rectangle in logical global coordinates.
    void regionSelected(const QRect &region);
    /// Emitted when the user aborted the selection.
    void selectionCancelled();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QRect m_selection;
    bool m_dragging = false;
};

} // namespace imagen
