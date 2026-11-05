// SPDX-License-Identifier: MIT
#pragma once

#include "capture/CaptureTypes.h"
#include "settings/Config.h"

#include <QObject>
#include <QString>

namespace imagen {

class AutoSaveOutput;
class ClipboardOutput;
class ExternalProgramOutput;
class FileOutput;

/**
 * Sends a captured frame to the configured destination.
 */
class OutputRouter : public QObject
{
    Q_OBJECT

public:
    explicit OutputRouter(QObject *parent = nullptr);

    /// Routes the frame to the destination configured in the settings.
    void route(const CapturedFrame &frame);

    /// Routes the frame to an explicit destination.
    void routeTo(OutputTarget target, const CapturedFrame &frame);

    /// Opens the auto-save folder in the file manager.
    void openAutoSaveFolder();

Q_SIGNALS:
    /// Emitted when the image was written to a file.
    void fileSaved(const QString &fileName);
    /// Emitted when the image was copied to the clipboard.
    void copiedToClipboard();
    /// Emitted when the image was handed over to an external program.
    void launchedExternalProgram();
    /// Emitted when the image should be shown in the editor.
    void editorRequested(const imagen::CapturedFrame &frame);
    /// Emitted when something went wrong.
    void failed(const QString &message);

private:
    ClipboardOutput *m_clipboard = nullptr;
    FileOutput *m_file = nullptr;
    AutoSaveOutput *m_autoSave = nullptr;
    ExternalProgramOutput *m_external = nullptr;
};

} // namespace imagen
