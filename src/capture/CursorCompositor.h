// SPDX-License-Identifier: MIT
#pragma once

#include <QImage>
#include <QPoint>

namespace imagen {

/**
 * Draws the mouse cursor into a captured image.
 *
 * Neither the XDG portal nor XGetImage include the pointer, so it is
 * composited from the cursor theme instead. The cursor bitmap is taken from
 * the platform theme, which is why the result matches the on-screen shape.
 *
 * @param image    image to draw into (modified in place)
 * @param devicePos cursor position in the image's pixel coordinates
 * @return true when a cursor was drawn
 */
bool compositeCursor(QImage &image, const QPoint &devicePos);

} // namespace imagen
