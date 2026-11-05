// SPDX-License-Identifier: MIT
#pragma once

#include <QImage>
#include <QObject>
#include <QString>

namespace imagen {

/**
 * Saves images automatically into a folder using a file name pattern.
 *
 * The pattern is a strftime-like string, `screenshot_%Y%m%d_%H%M%S` by
 * default.
 */
class AutoSaveOutput : public QObject
{
    Q_OBJECT

public:
    explicit AutoSaveOutput(QObject *parent = nullptr);

    /// Writes the image into the configured folder. Returns the file name.
    QString save(const QImage &image);

    /// The folder the images are written to.
    QString directory() const;

    /// The file name of the next image.
    QString nextFileName() const;

private:
    QString m_directory;
    QString m_pattern;
    QString m_format;
};

} // namespace imagen
