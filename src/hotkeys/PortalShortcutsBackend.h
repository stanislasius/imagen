// SPDX-License-Identifier: MIT
#pragma once

#include "hotkeys/HotkeyProvider.h"

#include <QDBusObjectPath>
#include <QHash>
#include <QVariantMap>

namespace imagen {

class PortalRequest;

/**
 * Global shortcut provider based on the XDG GlobalShortcuts portal.
 *
 * The provider creates a portal session and binds the configured
 * shortcuts to it. Activations are reported through the portal's
 * `Activated` signal.
 */
class PortalShortcutsBackend : public HotkeyProvider
{
    Q_OBJECT

public:
    explicit PortalShortcutsBackend(QObject *parent = nullptr);
    ~PortalShortcutsBackend() override;

    QString id() const override;
    QString displayName() const override;
    bool isAvailable() const override;
    bool registerShortcuts(const QHash<QString, HotkeySequence> &shortcuts) override;
    void unregisterShortcuts() override;

    /// Whether a portal session was created successfully.
    bool hasSession() const { return !m_sessionHandle.isEmpty(); }

private Q_SLOTS:
    void onActivated(const QDBusObjectPath &session,
                     const QString &shortcutId,
                     qulonglong timestamp,
                     const QVariantMap &options);

private:
    void createSession();
    void bindShortcuts(const QHash<QString, HotkeySequence> &shortcuts);

    PortalRequest *m_portal = nullptr;
    QString m_sessionHandle;
    bool m_sessionRequested = false;
    QHash<QString, HotkeySequence> m_pendingShortcuts;
};

} // namespace imagen
