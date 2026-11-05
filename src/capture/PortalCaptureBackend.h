// SPDX-License-Identifier: MIT
#pragma once

#include "capture/CaptureBackend.h"

#include <QVariantMap>

namespace imagen {

class PortalRequest;

/**
 * Capture backend for Wayland sessions.
 *
 * Uses the XDG desktop portal `org.freedesktop.portal.Screenshot`:
 *
 *  - Interactive kinds (region, window, freehand) let the compositor show
 *    its own selection UI and hand back the already cropped image.
 *  - Non interactive kinds (full screen, single monitor) request the whole
 *    virtual desktop and crop it client side using the screen geometry,
 *    taking the device pixel ratio into account.
 *  - Repeated captures of a fixed region are produced by requesting the
 *    whole desktop again and cropping the stored rectangle.
 */
class PortalCaptureBackend : public CaptureBackend
{
    Q_OBJECT

public:
    explicit PortalCaptureBackend(QObject *parent = nullptr);
    ~PortalCaptureBackend() override;

    QString id() const override;
    QString displayName() const override;
    bool isAvailable() const override;
    bool supports(CaptureKind kind) const override;
    void requestCapture(const CaptureRequest &request) override;
    void cancel() override;

private:
    void handleResponse(quint32 code, const QVariantMap &results);
    void fail(const QString &message);

    /// Crops the full desktop image down to the requested screen or region.
    CapturedFrame cropToScreen(const CapturedFrame &frame, const CaptureRequest &request) const;

    PortalRequest *m_portal = nullptr;
    bool m_pending = false;
    bool m_handled = false;
    CaptureRequest m_request;
};

} // namespace imagen
