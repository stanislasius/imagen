// SPDX-License-Identifier: MIT
#pragma once

#include <QImage>
#include <QObject>
#include <QString>

namespace imagen {

/**
 * Saves images to a file chosen by the user.
 */
class FileOutput : public QObject
{
    Q_OBJECT

public:
    explicit FileOutput(QObject *parent = nullptr);

    /// Opens a save dialog and writes the image. Returns the file name.
    QString save(const QImage &image, const QString &suggestedName = QString());

    /// Writes the image to the given file, deriving the format from the
    /// file suffix.
    static bool write(const QImage &image, const QString &fileName);

    /// The file name filters offered in the save dialog.
    static QString fileDialogFilter();
};

} // namespace imagen
