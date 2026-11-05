// SPDX-License-Identifier: MIT
#pragma once

#include <QImage>
#include <QObject>
#include <QString>

namespace imagen {

/**
 * Hands a captured image over to an external program.
 *
 * The command is a template such as `gimp %f`. The placeholder is replaced
 * by the name of a temporary file that holds the image.
 */
class ExternalProgramOutput : public QObject
{
    Q_OBJECT

public:
    explicit ExternalProgramOutput(QObject *parent = nullptr);

    /// Writes the image to a temporary file and starts the program.
    bool launch(const QImage &image, const QString &command = QString());

    /// The command template that is used when none is given.
    QString command() const;

private:
    QString m_command;
};

} // namespace imagen
