// SPDX-License-Identifier: MIT
#pragma once

#include <QImage>
#include <QObject>

namespace imagen {

/**
 * Copies images to the clipboard.
 *
 * Both PNG and BMP are offered, because applications differ in the image
 * format they accept from a clipboard.
 */
class ClipboardOutput : public QObject
{
    Q_OBJECT

public:
    explicit ClipboardOutput(QObject *parent = nullptr);

    /// Copies the image to the clipboard.
    bool copy(const QImage &image);

    /// Whether the clipboard currently holds an image.
    bool hasImage() const;
};

} // namespace imagen
