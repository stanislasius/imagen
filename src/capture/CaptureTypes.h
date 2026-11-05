// SPDX-License-Identifier: MIT
#pragma once

#include <QImage>
#include <QMetaType>
#include <QPoint>
#include <QRect>
#include <QString>

class QScreen;

namespace imagen {

/**
 * The kind of content the user asked to capture.
 */
enum class CaptureKind {
    FullScreen,     ///< every monitor, stitched into one image
    Monitor,        ///< a single monitor
    ActiveWindow,   ///< the window that currently has the focus
    Window,         ///< a window picked interactively
    Region,         ///< a rectangle picked interactively
    FixedRegion,    ///< the rectangle used by the previous region capture
    FreehandRegion, ///< an arbitrary shape picked interactively
    LastCapture,    ///< repeat of the most recent capture
};

/// Human readable, translatable name of a capture kind.
QString captureKindName(CaptureKind kind);

/// Parses a capture kind from its stable string identifier.
CaptureKind captureKindFromId(const QString &id);

/// Stable string identifier of a capture kind (used in settings and hotkeys).
QString captureKindId(CaptureKind kind);

/**
 * A capture request handed to a backend.
 */
struct CaptureRequest {
    CaptureKind kind = CaptureKind::FullScreen;

    /// Screen to capture for CaptureKind::Monitor. May be null for "all".
    QScreen *screen = nullptr;

    /// Region to capture for CaptureKind::FixedRegion, in logical desktop
    /// relative coordinates (origin is the top-left of the virtual desktop).
    QRect region;

    /// Whether the backend may show its own selection user interface.
    bool interactive = true;

    /// Whether the mouse cursor should be part of the result.
    bool includeCursor = false;
};

/**
 * The result of a successful capture.
 */
struct CapturedFrame {
    QImage image;

    /// Logical rectangle (desktop relative) the image was taken from.
    QRect sourceGeometry;

    /// Device pixel ratio of the captured surface.
    qreal devicePixelRatio = 1.0;

    /// Cursor position in global logical coordinates at capture time.
    QPoint cursorPosition;

    /// Whether the cursor is actually drawn into @c image.
    bool cursorIncluded = false;

    /// Name of the backend that produced the frame (for diagnostics).
    QString backendName;

    bool isValid() const { return !image.isNull(); }
};

} // namespace imagen

Q_DECLARE_METATYPE(imagen::CaptureKind)
Q_DECLARE_METATYPE(imagen::CapturedFrame)
