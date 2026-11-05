// SPDX-License-Identifier: MIT
#pragma once

#include <QList>
#include <QString>

namespace imagen {

class HotkeySequence;

/**
 * Every action that can be bound to a global shortcut.
 */
enum class HotkeyAction {
    CaptureFullScreen,
    CaptureMonitor,
    CaptureActiveWindow,
    CaptureWindow,
    CaptureRegion,
    CaptureFixedRegion,
    CaptureFreehandRegion,
    CaptureLast,
    OpenEditor,
    ColorPicker,
    Magnifier,
    ShowTrayMenu,
    Settings,
};

/// Stable identifier of an action, e.g. "capture.fullscreen".
QString hotkeyActionId(HotkeyAction action);

/// Reverse lookup for hotkeyActionId().
HotkeyAction hotkeyActionFromId(const QString &id);

/// Translatable, human readable name of an action.
QString hotkeyActionName(HotkeyAction action);

/// All actions known to the application.
QList<HotkeyAction> allHotkeyActions();

/// The shortcut the action is bound to by default.
HotkeySequence defaultHotkeySequence(HotkeyAction action);

} // namespace imagen
