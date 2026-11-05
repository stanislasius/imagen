// SPDX-License-Identifier: MIT
#include "hotkeys/HotkeyAction.h"

#include "hotkeys/HotkeySequence.h"

#include <QCoreApplication>
#include <QHash>

namespace imagen {
namespace {

struct ActionDescriptor {
    HotkeyAction action;
    const char *id;
    const char *defaultShortcut;
};

const ActionDescriptor kActions[] = {
    { HotkeyAction::CaptureFullScreen, "capture.fullscreen", "Print" },
    { HotkeyAction::CaptureMonitor, "capture.monitor", "Ctrl+Alt+Print" },
    { HotkeyAction::CaptureActiveWindow, "capture.activewindow", "Alt+Print" },
    { HotkeyAction::CaptureWindow, "capture.window", "Alt+Shift+Print" },
    { HotkeyAction::CaptureRegion, "capture.region", "Ctrl+Print" },
    { HotkeyAction::CaptureFixedRegion, "capture.fixedregion", "Ctrl+Alt+Shift+Print" },
    { HotkeyAction::CaptureFreehandRegion, "capture.freehand", "Ctrl+Shift+Alt+Print" },
    { HotkeyAction::CaptureLast, "capture.last", "Ctrl+Shift+Print" },
    { HotkeyAction::OpenEditor, "editor.open", "Ctrl+O" },
    { HotkeyAction::ColorPicker, "tools.colorpicker", "" },
    { HotkeyAction::Magnifier, "tools.magnifier", "" },
    { HotkeyAction::ShowTrayMenu, "shell.traymenu", "" },
    { HotkeyAction::Settings, "shell.settings", "Ctrl+," },
};

} // namespace

QString hotkeyActionId(HotkeyAction action)
{
    for (const ActionDescriptor &descriptor : kActions) {
        if (descriptor.action == action)
            return QString::fromLatin1(descriptor.id);
    }
    return {};
}

HotkeyAction hotkeyActionFromId(const QString &id)
{
    for (const ActionDescriptor &descriptor : kActions) {
        if (id == QLatin1String(descriptor.id))
            return descriptor.action;
    }
    return HotkeyAction::CaptureFullScreen;
}

QString hotkeyActionName(HotkeyAction action)
{
    switch (action) {
    case HotkeyAction::CaptureFullScreen:
        return QCoreApplication::translate("HotkeyAction", "Capture full screen");
    case HotkeyAction::CaptureMonitor:
        return QCoreApplication::translate("HotkeyAction", "Capture a single monitor");
    case HotkeyAction::CaptureActiveWindow:
        return QCoreApplication::translate("HotkeyAction", "Capture the active window");
    case HotkeyAction::CaptureWindow:
        return QCoreApplication::translate("HotkeyAction", "Capture a window");
    case HotkeyAction::CaptureRegion:
        return QCoreApplication::translate("HotkeyAction", "Capture a region");
    case HotkeyAction::CaptureFixedRegion:
        return QCoreApplication::translate("HotkeyAction", "Capture the fixed region");
    case HotkeyAction::CaptureFreehandRegion:
        return QCoreApplication::translate("HotkeyAction", "Capture a freehand region");
    case HotkeyAction::CaptureLast:
        return QCoreApplication::translate("HotkeyAction", "Repeat the last capture");
    case HotkeyAction::OpenEditor:
        return QCoreApplication::translate("HotkeyAction", "Open the editor");
    case HotkeyAction::ColorPicker:
        return QCoreApplication::translate("HotkeyAction", "Colour picker");
    case HotkeyAction::Magnifier:
        return QCoreApplication::translate("HotkeyAction", "Magnifier");
    case HotkeyAction::ShowTrayMenu:
        return QCoreApplication::translate("HotkeyAction", "Show the main menu");
    case HotkeyAction::Settings:
        return QCoreApplication::translate("HotkeyAction", "Open the settings");
    }
    return {};
}

QList<HotkeyAction> allHotkeyActions()
{
    QList<HotkeyAction> actions;
    actions.reserve(static_cast<int>(std::size(kActions)));
    for (const ActionDescriptor &descriptor : kActions)
        actions.append(descriptor.action);
    return actions;
}

HotkeySequence defaultHotkeySequence(HotkeyAction action)
{
    for (const ActionDescriptor &descriptor : kActions) {
        if (descriptor.action == action)
            return HotkeySequence::fromString(QString::fromLatin1(descriptor.defaultShortcut));
    }
    return {};
}

} // namespace imagen
