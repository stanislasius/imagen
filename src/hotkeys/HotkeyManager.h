// SPDX-License-Identifier: MIT
#pragma once

#include "hotkeys/HotkeyProvider.h"
#include "hotkeys/HotkeySequence.h"

#include <QHash>
#include <QObject>

namespace imagen {

/**
 * Resolves the configured shortcuts through the best available provider.
 *
 * The providers are tried in order: the GlobalShortcuts portal first,
 * the X11 keyboard grab second. The provider that is used is reported so
 * the settings dialog can explain the situation to the user.
 */
class HotkeyManager : public QObject
{
    Q_OBJECT

public:
    explicit HotkeyManager(QObject *parent = nullptr);
    ~HotkeyManager() override;

    /// Whether a provider is available and the shortcuts are bound.
    bool isActive() const;

    /// Name of the provider that handles the shortcuts.
    QString providerName() const;

    /// Identifier of the provider that handles the shortcuts.
    QString providerId() const;

    /// The shortcuts that are currently bound.
    QHash<QString, HotkeySequence> shortcuts() const;

    /// Binds the shortcuts again, picking up configuration changes.
    void reload();

Q_SIGNALS:
    /// Emitted with the identifier of the activated action.
    void activated(const QString &actionId);

private:
    void bindProvider();

    QList<HotkeyProvider *> m_providers;
    HotkeyProvider *m_activeProvider = nullptr;
    QHash<QString, HotkeySequence> m_shortcuts;
};

} // namespace imagen
