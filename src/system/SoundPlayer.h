// SPDX-License-Identifier: MIT
#pragma once

#include <QObject>
#include <QString>

namespace imagen {

/**
 * Plays the shutter sound.
 *
 * Qt's multimedia module is not a dependency of the application, so the
 * sound is handed to whichever command line player is installed. The
 * bundled sound is a plain uncompressed wav file, which every player
 * understands.
 */
class SoundPlayer : public QObject
{
    Q_OBJECT

public:
    explicit SoundPlayer(QObject *parent = nullptr);

    /// Plays the shutter sound. Returns false when no player is available.
    bool playShutter();

    /// The file that is played.
    QString shutterSoundFile() const;

private:
    QString m_soundFile;
};

} // namespace imagen
