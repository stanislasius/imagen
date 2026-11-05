// SPDX-License-Identifier: MIT
#pragma once

#include <QList>
#include <QRect>
#include <QSize>

class QScreen;

namespace imagen {

/**
 * Plain description of a screen, used for the geometry calculations.
 *
 * Keeping the calculations independent of QScreen makes them testable
 * without a running display server.
 */
struct ScreenDescriptor {
    QRect geometry;                 ///< logical geometry, may be negative
    qreal devicePixelRatio = 1.0;   ///< physical pixels per logical pixel
};

/**
 * Mapping between one screen's logical (DIP) and physical (device pixel)
 * coordinates, both relative to the origin of the virtual desktop.
 */
struct ScreenMapping {
    QRect logicalRect;   ///< logical rectangle relative to the desktop origin
    QRect deviceRect;    ///< physical rectangle relative to the desktop origin
    qreal devicePixelRatio = 1.0;
};

/**
 * The virtual desktop: every connected screen plus the aggregate sizes.
 *
 * The logical layout is normalised so that its origin is (0, 0) even when
 * the left-most or top-most monitor sits at negative coordinates.
 *
 * The physical layout is built by walking the screens in order and scaling
 * each one with its own device pixel ratio. This keeps the physical layout
 * contiguous, which is what capture backends hand back (one image covering
 * the whole desktop).
 */
struct VirtualDesktop {
    QList<ScreenMapping> screens;

    /// Physical size of the whole desktop in device pixels.
    QSize devicePixelSize() const;

    /// Logical size of the whole desktop in DIP.
    QSize logicalSize() const;

    /// Union of all logical rectangles.
    QRect logicalBounds() const;

    /// Mapping for a screen with the given logical geometry.
    ScreenMapping mappingForGeometry(const QRect &geometry) const;

    /// Mapping for a screen, or an invalid mapping when unknown.
    ScreenMapping mappingFor(const QScreen *screen) const;
};

/// Builds the virtual desktop description for the given screens.
VirtualDesktop buildVirtualDesktop(const QList<ScreenDescriptor> &screens);

/// Builds the virtual desktop description for the connected screens.
VirtualDesktop buildVirtualDesktop(const QList<QScreen *> &screens);

/// Maps a logical rectangle (desktop relative) into device pixels.
QRect mapLogicalToDevice(const VirtualDesktop &desktop, const QRect &logicalRect);

/// Maps a device pixel rectangle (desktop relative) into logical coordinates.
QRect mapDeviceToLogical(const VirtualDesktop &desktop, const QRect &deviceRect);

/**
 * Maps a device pixel rectangle onto a captured image.
 *
 * Capture backends are allowed to return an image that is smaller than the
 * real desktop resolution (the XDG portal does exactly that for very large
 * desktops). The rectangle is therefore scaled from the virtual desktop
 * resolution onto the actual image resolution.
 */
QRect mapDeviceRectToImage(const QRect &deviceRect,
                           const QSize &virtualDevicePixelSize,
                           const QSize &imageSize);

/**
 * Maps a logical rectangle onto a captured image of the whole desktop.
 *
 * The XDG portal returns one image covering the entire virtual desktop.
 * Its pixel layout mirrors the logical layout, so the rectangle is scaled
 * by the ratio between the image size and the logical desktop size. This is
 * exact for a uniform device pixel ratio and a good approximation for
 * mixed-DPI setups.
 */
QRect mapLogicalRectToImage(const QRect &logicalRect,
                            const QRect &virtualLogicalBounds,
                            const QSize &imageSize);

/// The device pixel rectangle of a single screen, relative to the desktop.
QRect screenDeviceRect(const VirtualDesktop &desktop, const QScreen *screen);

/// The device pixel rectangle of a single screen, relative to the desktop.
QRect screenDeviceRect(const VirtualDesktop &desktop, const QRect &geometry);

} // namespace imagen
