// SPDX-License-Identifier: MIT
#include "hotkeys/PortalShortcutsBackend.h"

#include "common/Logging.h"
#include "common/Platform.h"
#include "common/PortalRequest.h"
#include "hotkeys/KeyNames.h"

#include <QDBusConnection>
#include <QDBusMetaType>
#include <QDBusObjectPath>
#include <QPair>
#include <QVariantMap>

namespace imagen {
namespace {

/// The shortcut list type the GlobalShortcuts portal expects. The
/// portal documents it as QList<QPair<QString, QVariantMap>>, which is
/// marshalled as the D-Bus type a(sa{sv}).
using PortalShortcutList = QList<std::pair<QString, QVariantMap>>;

/**
 * Registers the shortcut types with the D-Bus marshaller.
 */
void registerPortalShortcutTypes()
{
    static bool registered = false;
    if (registered)
        return;
    registered = true;

    qDBusRegisterMetaType<std::pair<QString, QVariantMap>>();
    qDBusRegisterMetaType<PortalShortcutList>();
}

/**
 * Converts a shortcut into the key names understood by the portal.
 *
 * The portal describes a shortcut as an array of strings: first the
 * modifiers, then the key. Modifier and key names follow the XKB
 * naming scheme (the names the KDE portal implementation understands are
 * CTRL, ALT, SHIFT, CAPS, NUM and LOGO).
 */
QStringList portalKeyNames(const HotkeySequence &sequence)
{
    QStringList names;

    const QKeyCombination combination = sequence.toKeySequence()[0];
    const Qt::KeyboardModifiers modifiers = combination.keyboardModifiers();

    if (modifiers & Qt::ShiftModifier)
        names << QStringLiteral("SHIFT");
    if (modifiers & Qt::ControlModifier)
        names << QStringLiteral("CTRL");
    if (modifiers & Qt::AltModifier)
        names << QStringLiteral("ALT");
    if (modifiers & Qt::MetaModifier)
        names << QStringLiteral("LOGO");

    const QString keyName = KeyNames::forKey(sequence.toKeySequence());
    if (keyName.isEmpty())
        return {};
    names << keyName;

    return names;
}

} // namespace

PortalShortcutsBackend::PortalShortcutsBackend(QObject *parent)
    : HotkeyProvider(parent)
    , m_portal(new PortalRequest(this))
{
}

PortalShortcutsBackend::~PortalShortcutsBackend() = default;

QString PortalShortcutsBackend::id() const
{
    return QStringLiteral("portal");
}

QString PortalShortcutsBackend::displayName() const
{
    return QStringLiteral("XDG GlobalShortcuts portal");
}

bool PortalShortcutsBackend::isAvailable() const
{
    return Platform::isPortalInterfaceAvailable(QStringLiteral("org.freedesktop.portal.GlobalShortcuts"));
}

bool PortalShortcutsBackend::registerShortcuts(const QHash<QString, HotkeySequence> &shortcuts)
{
    if (!isAvailable())
        return false;

    m_pendingShortcuts = shortcuts;

    if (!m_sessionRequested) {
        m_sessionRequested = true;
        createSession();
        return true;
    }

    if (m_sessionHandle.isEmpty())
        return false;

    bindShortcuts(m_pendingShortcuts);
    return true;
}

void PortalShortcutsBackend::createSession()
{
    QVariantMap options;
    options.insert(QStringLiteral("session_handle_token"), QStringLiteral("imagen_session"));

    m_portal->call(QStringLiteral("org.freedesktop.portal.GlobalShortcuts"),
                   QStringLiteral("CreateSession"),
                   {},
                   options,
                   [this](quint32 code, const QVariantMap &results) {
                       if (code != 0) {
                           qCWarning(lcHotkeys) << "cannot create a portal shortcut session, code" << code;
                           return;
                       }
                       m_sessionHandle = results.value(QStringLiteral("session_handle")).toString();
                       if (m_sessionHandle.isEmpty()) {
                           qCWarning(lcHotkeys) << "the portal did not return a session handle";
                           return;
                       }
                       qCInfo(lcHotkeys) << "portal shortcut session" << m_sessionHandle;

                       QDBusConnection bus = m_portal->connection();
                       bus.connect(QLatin1String("org.freedesktop.portal.Desktop"),
                                   m_sessionHandle,
                                   QStringLiteral("org.freedesktop.portal.GlobalShortcuts"),
                                   QStringLiteral("Activated"),
                                   this,
                                   SLOT(onActivated(QDBusObjectPath, QString, qulonglong, QVariantMap)));

                        // The session exists now, so the shortcuts can be bound.
                        bindShortcuts(m_pendingShortcuts);
                   });
}

void PortalShortcutsBackend::bindShortcuts(const QHash<QString, HotkeySequence> &shortcuts)
{
    registerPortalShortcutTypes();

    PortalShortcutList entries;
    for (auto it = shortcuts.constBegin(); it != shortcuts.constEnd(); ++it) {
        const QStringList names = portalKeyNames(it.value());
        if (names.isEmpty()) {
            qCWarning(lcHotkeys) << "no portal representation for shortcut" << it.value().toString();
            continue;
        }

        QVariantMap description;
        description.insert(QStringLiteral("description"), it.key());
        description.insert(QStringLiteral("value"), names);

        entries.append(qMakePair(it.key(), description));
    }

    if (entries.isEmpty()) {
        unregisterShortcuts();
        return;
    }

    QVariantMap options;

    m_portal->call(QStringLiteral("org.freedesktop.portal.GlobalShortcuts"),
                   QStringLiteral("BindShortcuts"),
                   {QVariant::fromValue(QDBusObjectPath(m_sessionHandle)), QVariant::fromValue(entries), QString()},
                   options,
                   [this](quint32 code, const QVariantMap &results) {
                       if (code != 0) {
                           qCWarning(lcHotkeys) << "cannot bind the portal shortcuts, code" << code;
                           return;
                       }
                       qCInfo(lcHotkeys) << "bound portal shortcuts:" << results;
                   });
}

void PortalShortcutsBackend::onActivated(const QDBusObjectPath &session,
                                        const QString &shortcutId,
                                        qulonglong timestamp,
                                        const QVariantMap &options)
{
    Q_UNUSED(session)
    Q_UNUSED(timestamp)
    Q_UNUSED(options)

    qCInfo(lcHotkeys) << "portal shortcut activated:" << shortcutId;
    Q_EMIT activated(shortcutId);
}

void PortalShortcutsBackend::unregisterShortcuts()
{
    if (m_sessionHandle.isEmpty())
        return;

    registerPortalShortcutTypes();

    QVariantMap options;

    m_portal->call(QStringLiteral("org.freedesktop.portal.GlobalShortcuts"),
                   QStringLiteral("BindShortcuts"),
                   {QVariant::fromValue(QDBusObjectPath(m_sessionHandle)), QVariant::fromValue(PortalShortcutList()), QString()},
                   options,
                   [](quint32 code, const QVariantMap &) {
                       if (code != 0)
                           qCWarning(lcHotkeys) << "cannot unbind the portal shortcuts, code" << code;
                   });
}

} // namespace imagen
