// SPDX-License-Identifier: MIT
#pragma once

#include "capture/CaptureTypes.h"

#include <QObject>
#include <QString>

namespace imagen {

class CaptureManager;
class HotkeyManager;
class Notifier;
class OutputRouter;
class SettingsDialog;
class SoundPlayer;
class TrayIcon;
class EditorWindow;

/**
 * The application object that wires the subsystems together.
 */
class Application : public QObject
{
    Q_OBJECT

public:
    explicit Application(QObject *parent = nullptr);
    ~Application() override;

    /// Creates the subsystems and shows the tray icon.
    void start();

    /// Executes an action by its identifier.
    void handleAction(const QString &actionId);

    /// Shows the settings dialog.
    void showSettings();

private:
    void onFrameReady(const imagen::CapturedFrame &frame);
    void onCaptureCancelled();
    void onCaptureFailed(const QString &message);
    void onOutputFileSaved(const QString &fileName);
    void onOutputFailed(const QString &message);

    void applyTheme();
    void applyLanguage();
    void showEditor(const imagen::CapturedFrame &frame);

    CaptureManager *m_capture = nullptr;
    HotkeyManager *m_hotkeys = nullptr;
    OutputRouter *m_outputs = nullptr;
    Notifier *m_notifier = nullptr;
    SoundPlayer *m_sound = nullptr;
    TrayIcon *m_tray = nullptr;
    SettingsDialog *m_settings = nullptr;
    EditorWindow *m_editor = nullptr;
};

} // namespace imagen
