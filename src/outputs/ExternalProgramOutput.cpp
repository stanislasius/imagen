// SPDX-License-Identifier: MIT
#include "outputs/ExternalProgramOutput.h"

#include "common/AppInfo.h"
#include "common/Logging.h"
#include "outputs/FileOutput.h"
#include "settings/Config.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QProcess>

namespace imagen {

ExternalProgramOutput::ExternalProgramOutput(QObject *parent)
    : QObject(parent)
    , m_command(Config::instance().externalProgram())
{
}

QString ExternalProgramOutput::command() const
{
    return m_command;
}

bool ExternalProgramOutput::launch(const QImage &image, const QString &command)
{
    const QString template_ = command.isEmpty() ? m_command : command;
    if (image.isNull() || template_.isEmpty()) {
        qCWarning(lcOutputs) << "no external program configured";
        return false;
    }

    const QString directory = AppInfo::cacheDirPath() + QStringLiteral("/handover");
    QDir().mkpath(directory);

    const QString fileName = QDir(directory).filePath(QStringLiteral("imagen_%1.png")
                                                             .arg(QDateTime::currentMSecsSinceEpoch()));
    if (!FileOutput::write(image, fileName)) {
        qCWarning(lcOutputs) << "cannot write the handover file" << fileName;
        return false;
    }

    QStringList parts = QProcess::splitCommand(template_);
    if (parts.isEmpty())
        return false;

    // %f is replaced by the file name, every other argument is passed on.
    for (QString &part : parts) {
        if (part.contains(QLatin1String("%f"))) {
            part.replace(QLatin1String("%f"), fileName);
        }
    }

    const QString program = parts.takeFirst();
    if (!QProcess::startDetached(program, parts))
        return false;

    qCInfo(lcOutputs) << "handed the image over to" << program;
    return true;
}

} // namespace imagen
