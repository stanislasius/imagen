// SPDX-License-Identifier: MIT
#include "outputs/OutputRouter.h"

#include "common/Logging.h"
#include "outputs/AutoSaveOutput.h"
#include "outputs/ClipboardOutput.h"
#include "outputs/ExternalProgramOutput.h"
#include "outputs/FileOutput.h"

#include <QDesktopServices>
#include <QDir>
#include <QUrl>

namespace imagen {

OutputRouter::OutputRouter(QObject *parent)
    : QObject(parent)
    , m_clipboard(new ClipboardOutput(this))
    , m_file(new FileOutput(this))
    , m_autoSave(new AutoSaveOutput(this))
    , m_external(new ExternalProgramOutput(this))
{
}

void OutputRouter::route(const CapturedFrame &frame)
{
    routeTo(Config::instance().outputTarget(), frame);
}

void OutputRouter::routeTo(OutputTarget target, const CapturedFrame &frame)
{
    if (!frame.isValid()) {
        Q_EMIT failed(tr("The captured image is empty."));
        return;
    }

    switch (target) {
    case OutputTarget::Clipboard:
        if (m_clipboard->copy(frame.image))
            Q_EMIT copiedToClipboard();
        else
            Q_EMIT failed(tr("The image could not be copied to the clipboard."));
        return;

    case OutputTarget::File: {
        const QString fileName = m_file->save(frame.image);
        if (fileName.isEmpty())
            return;
        Q_EMIT fileSaved(fileName);
        return;
    }

    case OutputTarget::AutoSave: {
        const QString fileName = m_autoSave->save(frame.image);
        if (fileName.isEmpty()) {
            Q_EMIT failed(tr("The image could not be saved automatically."));
            return;
        }
        Q_EMIT fileSaved(fileName);
        return;
    }

    case OutputTarget::ExternalProgram:
        if (m_external->launch(frame.image))
            Q_EMIT launchedExternalProgram();
        else
            Q_EMIT failed(tr("The external program could not be started."));
        return;

    case OutputTarget::OpenFolder:
        openAutoSaveFolder();
        return;

    case OutputTarget::Editor:
        Q_EMIT editorRequested(frame);
        return;
    }
}

void OutputRouter::openAutoSaveFolder()
{
    const QString directory = m_autoSave->directory();
    if (!QDir(directory).exists()) {
        Q_EMIT failed(tr("The auto-save folder %1 does not exist.").arg(directory));
        return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(directory));
}

} // namespace imagen
