// SPDX-License-Identifier: MIT
#pragma once

#include <QHash>
#include <QObject>
#include <QString>

#include "hotkeys/HotkeySequence.h"

namespace imagen {

/**
 * Base class of the global shortcut providers.
 *
 * A provider binds a set of action identifiers to keyboard shortcuts and
 * reports activations. The HotkeyManager picks the first provider that is
 * available in the current session.
 */
class HotkeyProvider : public QObject
{
    Q_OBJECT

public:
    explicit HotkeyProvider(QObject *parent = nullptr)
        : QObject(parent)
    {
    }
    ~HotkeyProvider() override = default;

    /// Stable identifier of the provider.
    virtual QString id() const = 0;

    /// Human readable name shown in the diagnostics dialog.
    virtual QString displayName() const = 0;

    /// Whether the provider can be used in the current session.
    virtual bool isAvailable() const = 0;

    /// Binds the given shortcuts, replacing previously bound ones.
    virtual bool registerShortcuts(const QHash<QString, HotkeySequence> &shortcuts) = 0;

    /// Removes all bindings.
    virtual void unregisterShortcuts() = 0;

Q_SIGNALS:
    /// Emitted with the identifier of the activated action.
    void activated(const QString &actionId);
};

} // namespace imagen
