// SPDX-License-Identifier: MIT
#pragma once

#include <QWidget>

class QShowEvent;

namespace imagen {

/**
 * Base class for the transient windows that float above the desktop.
 *
 * On compositors that implement the layer-shell protocol the window is
 * anchored to the overlay layer, which keeps it visually on top of
 * fullscreen applications. Everywhere else it falls back to a frameless,
 * always-on-top, input-transparent window.
 */
class OverlayWindow : public QWidget
{
    Q_OBJECT

public:
    explicit OverlayWindow(QWidget *parent = nullptr);
    ~OverlayWindow() override;

    /// When enabled the window does not receive mouse or keyboard events.
    void setClickThrough(bool enabled);
    bool isClickThrough() const { return m_clickThrough; }

    /// Applies the layer-shell configuration for the given layer.
    void setOverlayLayer();

protected:
    void showEvent(QShowEvent *event) override;

private:
    bool m_clickThrough = false;
};

} // namespace imagen
