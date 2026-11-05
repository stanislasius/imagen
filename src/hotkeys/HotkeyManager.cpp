// SPDX-License-Identifier: MIT
#include "hotkeys/HotkeyManager.h"

#include "common/Logging.h"
#include "hotkeys/HotkeyAction.h"
#include "hotkeys/PortalShortcutsBackend.h"
#include "hotkeys/X11HotkeysBackend.h"
#include "settings/Config.h"

namespace imagen {

HotkeyManager::HotkeyManager(QObject *parent)
    : QObject(parent)
{
    auto *portal = new PortalShortcutsBackend(this);
    m_providers.append(portal);

    auto *x11 = new X11HotkeysBackend(this);
    m_providers.append(x11);

    for (HotkeyProvider *provider : m_providers) {
        connect(provider, &HotkeyProvider::activated, this, &HotkeyManager::activated);
    }

    reload();
}

HotkeyManager::~HotkeyManager() = default;

bool HotkeyManager::isActive() const
{
    return m_activeProvider != nullptr;
}

QString HotkeyManager::providerName() const
{
    return m_activeProvider ? m_activeProvider->displayName() : QString();
}

QString HotkeyManager::providerId() const
{
    return m_activeProvider ? m_activeProvider->id() : QString();
}

QHash<QString, HotkeySequence> HotkeyManager::shortcuts() const
{
    return m_shortcuts;
}

void HotkeyManager::reload()
{
    m_shortcuts.clear();

    const Config &config = Config::instance();
    for (HotkeyAction action : allHotkeyActions()) {
        const QString actionId = hotkeyActionId(action);
        const QString stored = config.hotkeySequence(actionId);
        HotkeySequence sequence = stored.isEmpty() ? defaultHotkeySequence(action)
                                                   : HotkeySequence::fromString(stored);
        if (sequence.isValid())
            m_shortcuts.insert(actionId, sequence);
    }

    bindProvider();
}

void HotkeyManager::bindProvider()
{
    for (HotkeyProvider *provider : m_providers) {
        if (!provider->isAvailable())
            continue;

        if (!provider->registerShortcuts(m_shortcuts)) {
            qCWarning(lcHotkeys) << "provider" << provider->id() << "could not bind the shortcuts";
            continue;
        }

        m_activeProvider = provider;
        qCInfo(lcHotkeys) << "shortcuts handled by" << provider->id();
        return;
    }

    m_activeProvider = nullptr;
    qCWarning(lcHotkeys) << "no hotkey provider is available";
}

} // namespace imagen
