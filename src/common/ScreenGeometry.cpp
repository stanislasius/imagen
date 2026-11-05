// SPDX-License-Identifier: MIT
#include "common/ScreenGeometry.h"

#include <QScreen>
#include <QtGlobal>

#include <algorithm>
#include <cmath>

namespace imagen {
namespace {

int scaledSize(qreal value, qreal ratio)
{
    return std::max(1, static_cast<int>(std::lround(value * ratio)));
}

} // namespace

QSize VirtualDesktop::devicePixelSize() const
{
    int width = 0;
    int height = 0;
    for (const ScreenMapping &mapping : screens) {
        width = std::max(width, mapping.deviceRect.right() + 1);
        height = std::max(height, mapping.deviceRect.bottom() + 1);
    }
    return QSize(width, height);
}

QSize VirtualDesktop::logicalSize() const
{
    return logicalBounds().size();
}

QRect VirtualDesktop::logicalBounds() const
{
    QRect bounds;
    for (const ScreenMapping &mapping : screens)
        bounds = bounds.united(mapping.logicalRect);
    return bounds;
}

ScreenMapping VirtualDesktop::mappingForGeometry(const QRect &geometry) const
{
    for (const ScreenMapping &mapping : screens) {
        if (mapping.logicalRect == geometry)
            return mapping;
    }
    return {};
}

ScreenMapping VirtualDesktop::mappingFor(const QScreen *screen) const
{
    if (!screen)
        return {};
    const ScreenMapping mapping = mappingForGeometry(screen->geometry());
    if (!mapping.logicalRect.isNull())
        return mapping;

    // Fall back to a size match for screens that share their origin.
    for (const ScreenMapping &candidate : screens) {
        if (candidate.logicalRect.size() == screen->geometry().size())
            return candidate;
    }
    return {};
}

VirtualDesktop buildVirtualDesktop(const QList<ScreenDescriptor> &screens)
{
    VirtualDesktop desktop;
    if (screens.isEmpty())
        return desktop;

    // Normalise the logical layout: the desktop origin becomes (0, 0).
    QRect logicalUnion;
    for (const ScreenDescriptor &screen : screens)
        logicalUnion = logicalUnion.united(screen.geometry);
    const QPoint origin = logicalUnion.topLeft();

    // The physical layout follows the geometric order of the screens, not
    // the order in which they were reported.
    QList<ScreenDescriptor> ordered = screens;
    std::sort(ordered.begin(), ordered.end(), [](const ScreenDescriptor &left, const ScreenDescriptor &right) {
        if (left.geometry.left() != right.geometry.left())
            return left.geometry.left() < right.geometry.left();
        return left.geometry.top() < right.geometry.top();
    });

    // Build the physical layout, scaling every screen with its own ratio.
    int deviceCursorX = 0;
    int deviceHeight = 0;
    for (const ScreenDescriptor &screen : ordered) {
        const qreal ratio = screen.devicePixelRatio > 0 ? screen.devicePixelRatio : 1.0;

        ScreenMapping mapping;
        mapping.devicePixelRatio = ratio;
        mapping.logicalRect = screen.geometry.translated(-origin);
        mapping.deviceRect = QRect(deviceCursorX, 0,
                                   scaledSize(screen.geometry.width(), ratio),
                                   scaledSize(screen.geometry.height(), ratio));

        desktop.screens.append(mapping);
        deviceCursorX += mapping.deviceRect.width();
        deviceHeight = std::max(deviceHeight, mapping.deviceRect.height());
    }

    return desktop;
}

VirtualDesktop buildVirtualDesktop(const QList<QScreen *> &screens)
{
    QList<ScreenDescriptor> descriptors;
    descriptors.reserve(screens.size());
    for (QScreen *screen : screens) {
        if (!screen)
            continue;
        ScreenDescriptor descriptor;
        descriptor.geometry = screen->geometry();
        descriptor.devicePixelRatio = screen->devicePixelRatio();
        descriptors.append(descriptor);
    }
    return buildVirtualDesktop(descriptors);
}

QRect mapLogicalToDevice(const VirtualDesktop &desktop, const QRect &logicalRect)
{
    QRect result;
    for (const ScreenMapping &mapping : desktop.screens) {
        const QRect overlap = mapping.logicalRect.intersected(logicalRect);
        if (overlap.isEmpty())
            continue;
        const QRect device = mapping.deviceRect;
        const qreal sx = qreal(device.width()) / qreal(mapping.logicalRect.width());
        const qreal sy = qreal(device.height()) / qreal(mapping.logicalRect.height());

        QRect mapped(overlap.left() - mapping.logicalRect.left(),
                     overlap.top() - mapping.logicalRect.top(),
                     overlap.width(),
                     overlap.height());
        mapped = QRect(device.left() + static_cast<int>(std::lround(mapped.left() * sx)),
                       device.top() + static_cast<int>(std::lround(mapped.top() * sy)),
                       std::max(1, static_cast<int>(std::lround(mapped.width() * sx))),
                       std::max(1, static_cast<int>(std::lround(mapped.height() * sy))));
        result = result.isNull() ? mapped : result.united(mapped);
    }
    return result;
}

QRect mapDeviceToLogical(const VirtualDesktop &desktop, const QRect &deviceRect)
{
    QRect result;
    for (const ScreenMapping &mapping : desktop.screens) {
        const QRect overlap = mapping.deviceRect.intersected(deviceRect);
        if (overlap.isEmpty())
            continue;
        const qreal sx = qreal(mapping.logicalRect.width()) / qreal(mapping.deviceRect.width());
        const qreal sy = qreal(mapping.logicalRect.height()) / qreal(mapping.deviceRect.height());

        QRect mapped(overlap.left() - mapping.deviceRect.left(),
                     overlap.top() - mapping.deviceRect.top(),
                     overlap.width(),
                     overlap.height());
        mapped = QRect(mapping.logicalRect.left() + static_cast<int>(std::lround(mapped.left() * sx)),
                       mapping.logicalRect.top() + static_cast<int>(std::lround(mapped.top() * sy)),
                       std::max(1, static_cast<int>(std::lround(mapped.width() * sx))),
                       std::max(1, static_cast<int>(std::lround(mapped.height() * sy))));
        result = result.isNull() ? mapped : result.united(mapped);
    }
    return result;
}

QRect mapDeviceRectToImage(const QRect &deviceRect,
                           const QSize &virtualDevicePixelSize,
                           const QSize &imageSize)
{
    if (virtualDevicePixelSize.isEmpty() || imageSize.isEmpty())
        return {};

    const qreal scaleX = qreal(imageSize.width()) / qreal(virtualDevicePixelSize.width());
    const qreal scaleY = qreal(imageSize.height()) / qreal(virtualDevicePixelSize.height());

    const int left = static_cast<int>(std::lround(deviceRect.left() * scaleX));
    const int top = static_cast<int>(std::lround(deviceRect.top() * scaleY));
    const int right = static_cast<int>(std::lround((deviceRect.right() + 1) * scaleX));
    const int bottom = static_cast<int>(std::lround((deviceRect.bottom() + 1) * scaleY));

    QRect mapped(left, top, std::max(1, right - left), std::max(1, bottom - top));
    return mapped.intersected(QRect(QPoint(0, 0), imageSize));
}

QRect mapLogicalRectToImage(const QRect &logicalRect,
                            const QRect &virtualLogicalBounds,
                            const QSize &imageSize)
{
    if (virtualLogicalBounds.isEmpty() || imageSize.isEmpty())
        return {};

    const qreal scaleX = qreal(imageSize.width()) / qreal(virtualLogicalBounds.width());
    const qreal scaleY = qreal(imageSize.height()) / qreal(virtualLogicalBounds.height());

    const int left = static_cast<int>(std::lround((logicalRect.left() - virtualLogicalBounds.left()) * scaleX));
    const int top = static_cast<int>(std::lround((logicalRect.top() - virtualLogicalBounds.top()) * scaleY));
    const int right = static_cast<int>(std::lround((logicalRect.right() + 1 - virtualLogicalBounds.left()) * scaleX));
    const int bottom = static_cast<int>(std::lround((logicalRect.bottom() + 1 - virtualLogicalBounds.top()) * scaleY));

    QRect mapped(left, top, std::max(1, right - left), std::max(1, bottom - top));
    return mapped.intersected(QRect(QPoint(0, 0), imageSize));
}

QRect screenDeviceRect(const VirtualDesktop &desktop, const QScreen *screen)
{
    return desktop.mappingFor(screen).deviceRect;
}

QRect screenDeviceRect(const VirtualDesktop &desktop, const QRect &geometry)
{
    return desktop.mappingForGeometry(geometry).deviceRect;
}

} // namespace imagen
