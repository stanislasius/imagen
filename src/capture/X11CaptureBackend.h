// SPDX-License-Identifier: MIT
#pragma once

#include "capture/CaptureBackend.h"

#include <QPointer>
#include <QRect>

class QScreen;

namespace imagen {

class SelectionOverlay;

/**
 * Capture backend for X11 sessions.
 *
 * The backend talks to the X server directly and can therefore implement
 * every capture mode of the plan, including a client side region picker
 * and the inclusion of the mouse cursor.
 */
class X11CaptureBackend : public CaptureBackend
{
    Q_OBJECT

public:
    explicit X11CaptureBackend(QObject *parent = nullptr);
    ~X11CaptureBackend() override;

    QString id() const override;
    QString displayName() const override;
    bool isAvailable() const override;
    bool supports(CaptureKind kind) const override;
    void requestCapture(const CaptureRequest &request) override;
    void cancel() override;

private:
    bool ensureDisplay() const;
    void closeDisplay();

    /// Grabs a rectangle of the root window, given in device pixels.
    QImage grabRoot(const QRect &deviceRect);

    /// Window id of the currently focused window, or 0.
    unsigned long activeWindow();

    /// Device pixel rectangle of an X window on the root window.
    bool windowDeviceRect(unsigned long window, QRect *deviceRect) const;

    /// Device pixel ratio of the screen the given logical position is on.
    qreal ratioAt(const QPoint &logicalPosition) const;

    /// Lets the user pick a window; returns its geometry in device pixels.
    bool pickWindowGeometry(QRect *deviceRect);

    /// Lets the user drag a rectangle; returns it in logical coordinates.
    bool pickRegion(QRect *logicalRect);

    /// Device pixel rectangle of a screen inside the root window.
    QRect screenDeviceRect(QScreen *screen) const;

    void deliver(const QImage &image, const QRect &sourceGeometry, qreal devicePixelRatio);

    bool m_pending = false;
    bool m_handled = false;
    CaptureRequest m_request;
    QPointer<SelectionOverlay> m_selectionOverlay;

    mutable void *m_display = nullptr;
    mutable unsigned long m_rootWindow = 0;
    mutable int m_screenIndex = 0;
    mutable bool m_shmAvailable = false;
};

} // namespace imagen
