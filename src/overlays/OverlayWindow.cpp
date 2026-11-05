// SPDX-License-Identifier: MIT
#include "overlays/OverlayWindow.h"

#include "common/Logging.h"

#include <QShowEvent>
#include <QWindow>

#if IMAGEN_HAVE_LAYER_SHELL
#include <LayerShellQt/Shell>
#include <LayerShellQt/Window>
#endif

namespace imagen {

OverlayWindow::OverlayWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
}

OverlayWindow::~OverlayWindow() = default;

void OverlayWindow::setClickThrough(bool enabled)
{
    m_clickThrough = enabled;
    if (enabled)
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
    else
        setAttribute(Qt::WA_TransparentForMouseEvents, false);
}

void OverlayWindow::setOverlayLayer()
{
#if IMAGEN_HAVE_LAYER_SHELL
    if (QWindow *handle = windowHandle()) {
        if (auto *shell = LayerShellQt::Window::get(handle)) {
            shell->setLayer(LayerShellQt::Window::Layer::Overlay);
            shell->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivity::None);
        }
    }
#else
    // Without layer-shell support the window flags already keep it on top.
    raise();
    activateWindow();
#endif
}

void OverlayWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    setOverlayLayer();
}

} // namespace imagen
