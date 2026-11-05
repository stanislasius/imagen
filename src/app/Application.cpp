// SPDX-License-Identifier: MIT
#include "app/Application.h"

#include "capture/CaptureManager.h"
#include "capture/CaptureTypes.h"
#include "common/AppInfo.h"
#include "common/Logging.h"
#include "common/Platform.h"
#include "editor/EditorWindow.h"
#include "hotkeys/HotkeyAction.h"
#include "hotkeys/HotkeyManager.h"
#include "outputs/OutputRouter.h"
#include "settings/Config.h"
#include "settings/SettingsDialog.h"
#include "system/Notifier.h"
#include "system/SoundPlayer.h"
#include "tray/TrayIcon.h"

#include <QApplication>
#include <QDir>
#include <QIcon>
#include <QLibraryInfo>
#include <QLocale>
#include <QMessageBox>
#include <QTranslator>

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
#include <QStyleHints>
#endif

namespace imagen {

Application::Application(QObject *parent)
    : QObject(parent)
{
}

Application::~Application() = default;

void Application::start()
{
    applyLanguage();
    applyTheme();

    m_capture = new CaptureManager(this);
    m_hotkeys = new HotkeyManager(this);
    m_outputs = new OutputRouter(this);
    m_notifier = new Notifier(this);
    m_sound = new SoundPlayer(this);
    m_tray = new TrayIcon(this);

    connect(m_capture, &CaptureManager::frameReady, this, &Application::onFrameReady);
    connect(m_capture, &CaptureManager::captureCancelled, this, &Application::onCaptureCancelled);
    connect(m_capture, &CaptureManager::captureFailed, this, &Application::onCaptureFailed);
    connect(m_hotkeys, &HotkeyManager::activated, this, &Application::handleAction);
    connect(m_tray, &TrayIcon::actionTriggered, this, &Application::handleAction);
    connect(m_outputs, &OutputRouter::fileSaved, this, &Application::onOutputFileSaved);
    connect(m_outputs, &OutputRouter::failed, this, &Application::onOutputFailed);
    connect(m_outputs, &OutputRouter::editorRequested, this,
            [this](const imagen::CapturedFrame &frame) { showEditor(frame); });

    if (Config::instance().showTrayIcon() && TrayIcon::isSystemTrayAvailable())
        m_tray->setVisible(true);

    qCInfo(lcApp) << "imagen started, capture backend:" << m_capture->backendName()
                  << "hotkeys:" << m_hotkeys->providerName();

    if (!m_capture->hasBackend()) {
        m_notifier->notify(tr("imagen"),
                           tr("No capture backend is available in this session."));
        qCWarning(lcApp) << "no capture backend is available";
    }
}

void Application::handleAction(const QString &actionId)
{
    qCDebug(lcApp) << "action:" << actionId;

    if (actionId == QLatin1String("app.quit")) {
        QCoreApplication::quit();
        return;
    }

    if (actionId == QLatin1String("shell.settings")) {
        showSettings();
        return;
    }

    if (actionId == QLatin1String("editor.open")) {
        showEditor(CapturedFrame());
        return;
    }

    if (actionId == QLatin1String("shell.traymenu")) {
        if (m_tray && m_tray->isVisible()) {
            // The menu is opened by the tray icon itself; nothing to do.
            return;
        }
        return;
    }

    const HotkeyAction action = hotkeyActionFromId(actionId);
    switch (action) {
    case HotkeyAction::CaptureFullScreen:
        m_capture->requestCapture(CaptureKind::FullScreen);
        return;
    case HotkeyAction::CaptureMonitor:
        m_capture->requestCapture(CaptureKind::Monitor);
        return;
    case HotkeyAction::CaptureActiveWindow:
        m_capture->requestCapture(CaptureKind::ActiveWindow);
        return;
    case HotkeyAction::CaptureWindow:
        m_capture->requestCapture(CaptureKind::Window);
        return;
    case HotkeyAction::CaptureRegion:
        m_capture->requestCapture(CaptureKind::Region);
        return;
    case HotkeyAction::CaptureFixedRegion:
        m_capture->requestCapture(CaptureKind::FixedRegion);
        return;
    case HotkeyAction::CaptureFreehandRegion:
        m_capture->requestCapture(CaptureKind::FreehandRegion);
        return;
    case HotkeyAction::CaptureLast:
        m_capture->requestCapture(CaptureKind::LastCapture);
        return;
    case HotkeyAction::ColorPicker:
    case HotkeyAction::Magnifier:
    case HotkeyAction::ShowTrayMenu:
    case HotkeyAction::OpenEditor:
    case HotkeyAction::Settings:
        break;
    }
}

void Application::onFrameReady(const imagen::CapturedFrame &frame)
{
    const Config &config = Config::instance();

    if (config.playShutterSound())
        m_sound->playShutter();

    if (config.alwaysCopyToClipboard())
        m_outputs->routeTo(OutputTarget::Clipboard, frame);

    m_outputs->route(frame);

    if (config.showNotification()) {
        if (config.outputTarget() == OutputTarget::AutoSave)
            m_notifier->notifyCapture(tr("Screen captured"), tr("Saved to the auto-save folder"));
        else
            m_notifier->notifyCapture(tr("Screen captured"), tr("%1 x %2 pixels")
                                                                  .arg(frame.image.width())
                                                                  .arg(frame.image.height()));
    }
}

void Application::onCaptureCancelled()
{
    qCInfo(lcApp) << "the capture was cancelled";
}

void Application::onCaptureFailed(const QString &message)
{
    qCWarning(lcApp) << "the capture failed:" << message;
    m_notifier->notify(tr("imagen"), tr("The capture failed: %1").arg(message));
}

void Application::onOutputFileSaved(const QString &fileName)
{
    qCInfo(lcApp) << "capture saved to" << fileName;
}

void Application::onOutputFailed(const QString &message)
{
    qCWarning(lcApp) << "the output failed:" << message;
    m_notifier->notify(tr("imagen"), message);
}

void Application::showSettings()
{
    if (!m_settings) {
        m_settings = new SettingsDialog();
        connect(m_settings, &SettingsDialog::settingsChanged, this, [this]() {
            applyTheme();
            applyLanguage();
            m_hotkeys->reload();
        });
    }

    m_settings->reload();
    m_settings->show();
    m_settings->raise();
    m_settings->activateWindow();
}

void Application::showEditor(const imagen::CapturedFrame &frame)
{
    if (!m_editor)
        m_editor = new EditorWindow();

    if (frame.isValid())
        m_editor->setImage(frame.image);

    m_editor->show();
    m_editor->raise();
    m_editor->activateWindow();
}

void Application::applyTheme()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    QStyleHints *hints = QGuiApplication::styleHints();
    switch (Config::instance().themeMode()) {
    case ThemeMode::Light:
        hints->setColorScheme(Qt::ColorScheme::Light);
        break;
    case ThemeMode::Dark:
        hints->setColorScheme(Qt::ColorScheme::Dark);
        break;
    case ThemeMode::System:
        hints->setColorScheme(Qt::ColorScheme::Unknown);
        break;
    }
#endif
}

void Application::applyLanguage()
{
    static QTranslator *translator = nullptr;

    const QString language = Config::instance().language();
    if (language.isEmpty() || language == QLatin1String("system")) {
        if (translator) {
            QCoreApplication::removeTranslator(translator);
            delete translator;
            translator = nullptr;
        }
        return;
    }

    if (!translator) {
        translator = new QTranslator(this);
    } else {
        QCoreApplication::removeTranslator(translator);
    }

    const QString directory = QDir(AppInfo::configDirPath()).filePath(QStringLiteral("translations"));
    if (translator->load(QStringLiteral("imagen_") + language, directory)) {
        QCoreApplication::installTranslator(translator);
    } else if (translator->load(QStringLiteral("imagen_") + language,
                               QStringLiteral(":/i18n"))) {
        QCoreApplication::installTranslator(translator);
    } else {
        qCWarning(lcApp) << "no translation found for" << language;
    }
}

} // namespace imagen
