// SPDX-License-Identifier: MIT
#include "capture/CursorCompositor.h"

#include "common/Logging.h"

#include <QCursor>
#include <QPainter>

namespace imagen {

bool compositeCursor(QImage &image, const QPoint &devicePos)
{
    if (image.isNull())
        return false;

    const QCursor cursor;
    const QPixmap cursorPixmap = cursor.pixmap();
    if (cursorPixmap.isNull())
        return false;

    QPainter painter(&image);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.drawPixmap(devicePos, cursorPixmap);
    painter.end();

    qCDebug(lcCapture) << "composited cursor at" << devicePos;
    return true;
}

} // namespace imagen
