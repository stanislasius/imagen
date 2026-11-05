// SPDX-License-Identifier: MIT
#include "settings/Config.h"

#include "common/AppInfo.h"
#include "common/Logging.h"

#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>

namespace imagen {
namespace {

QString key(const char *name)
{
    return QLatin1String(name);
}

} // namespace

QString outputTargetName(OutputTarget target)
{
    switch (target) {
    case OutputTarget::Editor:
        return QCoreApplication::translate("OutputTarget", "Editor");
    case OutputTarget::Clipboard:
        return QCoreApplication::translate("OutputTarget", "Clipboard");
    case OutputTarget::File:
        return QCoreApplication::translate("OutputTarget", "File");
    case OutputTarget::AutoSave:
        return QCoreApplication::translate("OutputTarget", "Auto-save folder");
    case OutputTarget::ExternalProgram:
        return QCoreApplication::translate("OutputTarget", "External program");
    case OutputTarget::OpenFolder:
        return QCoreApplication::translate("OutputTarget", "Open folder");
    }
    return {};
}

QString themeModeName(ThemeMode mode)
{
    switch (mode) {
    case ThemeMode::System:
        return QCoreApplication::translate("ThemeMode", "System");
    case ThemeMode::Light:
        return QCoreApplication::translate("ThemeMode", "Light");
    case ThemeMode::Dark:
        return QCoreApplication::translate("ThemeMode", "Dark");
    }
    return {};
}

Config::Config()
    : QObject(nullptr)
    , m_settingsPath(AppInfo::settingsFilePath())
    , m_settings(std::make_unique<QSettings>(m_settingsPath, QSettings::IniFormat))
{
}

Config::~Config()
{
    m_settings->sync();
}

Config &Config::instance()
{
    static Config config;
    return config;
}

void Config::setSettingsFilePath(const QString &path)
{
    Config &config = instance();
    if (config.m_settingsPath == path)
        return;

    config.m_settings->sync();
    config.m_settingsPath = path;
    config.m_settings = std::make_unique<QSettings>(path, QSettings::IniFormat);
}

// -- capture ---------------------------------------------------------------

int Config::captureDelay() const
{
    return m_settings->value(key("capture/delay"), 0).toInt();
}

void Config::setCaptureDelay(int seconds)
{
    m_settings->setValue(key("capture/delay"), qMax(0, seconds));
    Q_EMIT changed();
}

bool Config::includeCursor() const
{
    return m_settings->value(key("capture/includeCursor"), false).toBool();
}

void Config::setIncludeCursor(bool enabled)
{
    m_settings->setValue(key("capture/includeCursor"), enabled);
    Q_EMIT changed();
}

bool Config::playShutterSound() const
{
    return m_settings->value(key("capture/playSound"), true).toBool();
}

void Config::setPlayShutterSound(bool enabled)
{
    m_settings->setValue(key("capture/playSound"), enabled);
    Q_EMIT changed();
}

bool Config::showNotification() const
{
    return m_settings->value(key("capture/showNotification"), true).toBool();
}

void Config::setShowNotification(bool enabled)
{
    m_settings->setValue(key("capture/showNotification"), enabled);
    Q_EMIT changed();
}

bool Config::alwaysCopyToClipboard() const
{
    return m_settings->value(key("capture/alwaysCopyToClipboard"), true).toBool();
}

void Config::setAlwaysCopyToClipboard(bool enabled)
{
    m_settings->setValue(key("capture/alwaysCopyToClipboard"), enabled);
    Q_EMIT changed();
}

bool Config::hideEditorBeforeCapture() const
{
    return m_settings->value(key("capture/hideEditorBeforeCapture"), false).toBool();
}

void Config::setHideEditorBeforeCapture(bool enabled)
{
    m_settings->setValue(key("capture/hideEditorBeforeCapture"), enabled);
    Q_EMIT changed();
}

// -- output ----------------------------------------------------------------

OutputTarget Config::outputTarget() const
{
    const QString stored = m_settings->value(key("output/target"), QStringLiteral("editor")).toString();
    if (stored == QLatin1String("clipboard"))
        return OutputTarget::Clipboard;
    if (stored == QLatin1String("file"))
        return OutputTarget::File;
    if (stored == QLatin1String("autosave"))
        return OutputTarget::AutoSave;
    if (stored == QLatin1String("external"))
        return OutputTarget::ExternalProgram;
    if (stored == QLatin1String("folder"))
        return OutputTarget::OpenFolder;
    return OutputTarget::Editor;
}

void Config::setOutputTarget(OutputTarget target)
{
    QString stored = QStringLiteral("editor");
    switch (target) {
    case OutputTarget::Editor:
        stored = QStringLiteral("editor");
        break;
    case OutputTarget::Clipboard:
        stored = QStringLiteral("clipboard");
        break;
    case OutputTarget::File:
        stored = QStringLiteral("file");
        break;
    case OutputTarget::AutoSave:
        stored = QStringLiteral("autosave");
        break;
    case OutputTarget::ExternalProgram:
        stored = QStringLiteral("external");
        break;
    case OutputTarget::OpenFolder:
        stored = QStringLiteral("folder");
        break;
    }
    m_settings->setValue(key("output/target"), stored);
    Q_EMIT changed();
}

QString Config::autoSaveDirectory() const
{
    const QString stored = m_settings->value(key("output/autoSaveDirectory")).toString();
    if (!stored.isEmpty())
        return stored;
    return QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
}

void Config::setAutoSaveDirectory(const QString &path)
{
    m_settings->setValue(key("output/autoSaveDirectory"), path);
    Q_EMIT changed();
}

QString Config::autoSavePattern() const
{
    return m_settings->value(key("output/autoSavePattern"), QStringLiteral("screenshot_%Y%m%d_%H%M%S")).toString();
}

void Config::setAutoSavePattern(const QString &pattern)
{
    m_settings->setValue(key("output/autoSavePattern"), pattern);
    Q_EMIT changed();
}

QString Config::autoSaveFormat() const
{
    return m_settings->value(key("output/autoSaveFormat"), QStringLiteral("png")).toString();
}

void Config::setAutoSaveFormat(const QString &format)
{
    m_settings->setValue(key("output/autoSaveFormat"), format.toLower());
    Q_EMIT changed();
}

int Config::autoSaveQuality() const
{
    return m_settings->value(key("output/autoSaveQuality"), 90).toInt();
}

void Config::setAutoSaveQuality(int quality)
{
    m_settings->setValue(key("output/autoSaveQuality"), qBound(1, quality, 100));
    Q_EMIT changed();
}

QString Config::externalProgram() const
{
    return m_settings->value(key("output/externalProgram"), QString()).toString();
}

void Config::setExternalProgram(const QString &command)
{
    m_settings->setValue(key("output/externalProgram"), command);
    Q_EMIT changed();
}

// -- shell -----------------------------------------------------------------

bool Config::showTrayIcon() const
{
    return m_settings->value(key("shell/showTrayIcon"), true).toBool();
}

void Config::setShowTrayIcon(bool enabled)
{
    m_settings->setValue(key("shell/showTrayIcon"), enabled);
    Q_EMIT changed();
}

bool Config::launchAtStartup() const
{
    return m_settings->value(key("shell/launchAtStartup"), false).toBool();
}

void Config::setLaunchAtStartup(bool enabled)
{
    m_settings->setValue(key("shell/launchAtStartup"), enabled);
    Q_EMIT changed();
}

ThemeMode Config::themeMode() const
{
    const QString stored = m_settings->value(key("shell/theme"), QStringLiteral("system")).toString();
    if (stored == QLatin1String("light"))
        return ThemeMode::Light;
    if (stored == QLatin1String("dark"))
        return ThemeMode::Dark;
    return ThemeMode::System;
}

void Config::setThemeMode(ThemeMode mode)
{
    QString stored = QStringLiteral("system");
    if (mode == ThemeMode::Light)
        stored = QStringLiteral("light");
    else if (mode == ThemeMode::Dark)
        stored = QStringLiteral("dark");
    m_settings->setValue(key("shell/theme"), stored);
    Q_EMIT changed();
}

QString Config::language() const
{
    return m_settings->value(key("shell/language"), QStringLiteral("system")).toString();
}

void Config::setLanguage(const QString &language)
{
    m_settings->setValue(key("shell/language"), language);
    Q_EMIT changed();
}

void Config::sync()
{
    m_settings->sync();
}

// -- hotkeys ---------------------------------------------------------------

QString Config::hotkeySequence(const QString &actionId) const
{
    return m_settings->value(QStringLiteral("hotkeys/") + actionId).toString();
}

void Config::setHotkeySequence(const QString &actionId, const QString &sequence)
{
    if (sequence.isEmpty())
        m_settings->remove(QStringLiteral("hotkeys/") + actionId);
    else
        m_settings->setValue(QStringLiteral("hotkeys/") + actionId, sequence);
    Q_EMIT changed();
}

} // namespace imagen
