// SPDX-License-Identifier: MIT
#include "outputs/ClipboardOutput.h"

#include "common/Logging.h"

#include <QBuffer>
#include <QClipboard>
#include <QGuiApplication>
#include <QMimeData>

namespace imagen {

ClipboardOutput::ClipboardOutput(QObject *parent)
    : QObject(parent)
{
}

bool ClipboardOutput::copy(const QImage &image)
{
    if (image.isNull())
        return false;

    QClipboard *clipboard = QGuiApplication::clipboard();
    if (!clipboard)
        return false;

    auto *mimeData = new QMimeData();
    mimeData->setImageData(image);

    // Offer the common raster formats explicitly so that applications
    // asking for a specific format do not have to rely on conversions.
    QByteArray png;
    QBuffer pngBuffer(&png);
    pngBuffer.open(QIODevice::WriteOnly);
    image.save(&pngBuffer, "PNG");
    mimeData->setData(QStringLiteral("image/png"), png);

    QByteArray bmp;
    QBuffer bmpBuffer(&bmp);
    bmpBuffer.open(QIODevice::WriteOnly);
    image.save(&bmpBuffer, "BMP");
    mimeData->setData(QStringLiteral("image/bmp"), bmp);

    clipboard->setMimeData(mimeData);
    qCInfo(lcOutputs) << "image copied to the clipboard";
    return true;
}

bool ClipboardOutput::hasImage() const
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    return clipboard && clipboard->mimeData() && clipboard->mimeData()->hasImage();
}

} // namespace imagen
