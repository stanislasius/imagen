// SPDX-License-Identifier: MIT
#pragma once

#include "hotkeys/HotkeyProvider.h"

#include <QHash>
#include <QTimer>

namespace imagen {

/**
 * Global shortcut provider for X11 sessions.
 *
 * The shortcuts are grabbed on the root window with XGrabKey. Because the
 * application talks to the X server through its own connection, the
 * grabbed keys are polled with a short timer.
 */
class X11HotkeysBackend : public HotkeyProvider
{
    Q_OBJECT

public:
    explicit X11HotkeysBackend(QObject *parent = nullptr);
    ~X11HotkeysBackend() override;

    QString id() const override;
    QString displayName() const override;
    bool isAvailable() const override;
    bool registerShortcuts(const QHash<QString, HotkeySequence> &shortcuts) override;
    void unregisterShortcuts() override;

private:
    struct Grab {
        QString actionId;
        unsigned int keycode = 0;
        unsigned int modifiers = 0;
    };

    bool ensureDisplay() const;
    void closeDisplay();
    void pollEvents();
    void grab(const QString &actionId, const HotkeySequence &sequence);

    mutable void *m_display = nullptr;
    mutable unsigned long m_rootWindow = 0;
    QList<Grab> m_grabs;
    QTimer *m_timer = nullptr;
};

} // namespace imagen
