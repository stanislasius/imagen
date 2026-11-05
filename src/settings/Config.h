// SPDX-License-Identifier: MIT
#pragma once

#include <QObject>
#include <QSettings>
#include <QString>

#include <memory>

namespace imagen {

/// Where a captured image is sent after the capture finished.
enum class OutputTarget {
    Editor,           ///< open in the built-in editor
    Clipboard,        ///< copy to the clipboard only
    File,             ///< ask for a file name and save
    AutoSave,         ///< save into the auto-save folder
    ExternalProgram,  ///< hand over to a configured program
    OpenFolder,       ///< open the auto-save folder
};

/// Human readable, translatable name of an output target.
QString outputTargetName(OutputTarget target);

/// Theme preference.
enum class ThemeMode {
    System,
    Light,
    Dark,
};

/// Human readable, translatable name of a theme mode.
QString themeModeName(ThemeMode mode);

/**
 * Central configuration store.
 *
 * Backed by an ini file inside the user configuration directory, which
 * keeps the settings portable and easy to inspect.
 */
class Config : public QObject
{
    Q_OBJECT

public:
    static Config &instance();

    /// Overrides the location of the settings file. Used by the tests and
    /// by the portable mode.
    static void setSettingsFilePath(const QString &path);

    /// The underlying settings object. Use it for rarely accessed keys.
    QSettings &settings() { return *m_settings; }

    // -- capture ------------------------------------------------------------
    int captureDelay() const;
    void setCaptureDelay(int seconds);

    bool includeCursor() const;
    void setIncludeCursor(bool enabled);

    bool playShutterSound() const;
    void setPlayShutterSound(bool enabled);

    bool showNotification() const;
    void setShowNotification(bool enabled);

    bool alwaysCopyToClipboard() const;
    void setAlwaysCopyToClipboard(bool enabled);

    bool hideEditorBeforeCapture() const;
    void setHideEditorBeforeCapture(bool enabled);

    // -- output -------------------------------------------------------------
    OutputTarget outputTarget() const;
    void setOutputTarget(OutputTarget target);

    QString autoSaveDirectory() const;
    void setAutoSaveDirectory(const QString &path);

    QString autoSavePattern() const;
    void setAutoSavePattern(const QString &pattern);

    QString autoSaveFormat() const;
    void setAutoSaveFormat(const QString &format);

    int autoSaveQuality() const;
    void setAutoSaveQuality(int quality);

    QString externalProgram() const;
    void setExternalProgram(const QString &command);

    // -- shell --------------------------------------------------------------
    bool showTrayIcon() const;
    void setShowTrayIcon(bool enabled);

    bool launchAtStartup() const;
    void setLaunchAtStartup(bool enabled);

    ThemeMode themeMode() const;
    void setThemeMode(ThemeMode mode);

    QString language() const;
    void setLanguage(const QString &language);

    // -- hotkeys ------------------------------------------------------------
    /// The shortcut bound to an action, empty when it is not bound.
    QString hotkeySequence(const QString &actionId) const;
    void setHotkeySequence(const QString &actionId, const QString &sequence);

    /// Writes pending changes to disk.
    void sync();

Q_SIGNALS:
    /// Emitted after a setting was changed through this class.
    void changed();

private:
    Config();
    ~Config() override;

    QString m_settingsPath;
    std::unique_ptr<QSettings> m_settings;
};

} // namespace imagen
