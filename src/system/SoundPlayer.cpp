// SPDX-License-Identifier: MIT
#include "system/SoundPlayer.h"

#include "common/AppInfo.h"
#include "common/Logging.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>

namespace imagen {
namespace {

/// Command line players, in the order they are tried.
struct Player {
    const char *program;
    const char *arguments;
};

const Player kPlayers[] = {
    { "pw-play", "" },
    { "paplay", "" },
    { "aplay", "-q" },
};

} // namespace

SoundPlayer::SoundPlayer(QObject *parent)
    : QObject(parent)
{
    m_soundFile = QDir(AppInfo::soundDirPath()).filePath(QStringLiteral("shutter.wav"));
}

QString SoundPlayer::shutterSoundFile() const
{
    return m_soundFile;
}

bool SoundPlayer::playShutter()
{
    if (!QFileInfo::exists(m_soundFile)) {
        qCDebug(lcApp) << "the shutter sound is missing:" << m_soundFile;
        return false;
    }

    for (const Player &player : kPlayers) {
        const QString program = QString::fromLatin1(player.program);
        QStringList arguments;
        if (player.arguments[0] != '\0')
            arguments << QString::fromLatin1(player.arguments);
        arguments << m_soundFile;

        if (QProcess::startDetached(program, arguments)) {
            qCDebug(lcApp) << "playing the shutter sound with" << program;
            return true;
        }
    }

    qCDebug(lcApp) << "no audio player is available for the shutter sound";
    return false;
}

} // namespace imagen
