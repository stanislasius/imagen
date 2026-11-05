// SPDX-License-Identifier: MIT
#include "common/Platform.h"

#include "common/Logging.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusReply>
#include <QGuiApplication>
#include <QProcessEnvironment>

namespace imagen::Platform {
namespace {

const char *kPortalService = "org.freedesktop.portal.Desktop";
const char *kPortalPath = "/org/freedesktop/portal/desktop";

bool interfaceExists(const QString &service, const QString &path, const QString &interface)
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected())
        return false;

    QDBusMessage msg = QDBusMessage::createMethodCall(service, path,
                                                      QStringLiteral("org.freedesktop.DBus.Introspectable"),
                                                      QStringLiteral("Introspect"));
    const QDBusMessage reply = bus.call(msg, QDBus::Block, 2000);
    if (reply.type() != QDBusMessage::ReplyMessage)
        return false;
    return reply.arguments().value(0).toString().contains(interface);
}

} // namespace

SessionType sessionType()
{
    const QString platform = QGuiApplication::platformName().toLower();
    if (platform.contains(QStringLiteral("wayland")))
        return SessionType::Wayland;
    if (platform.contains(QStringLiteral("xcb")) || platform.contains(QStringLiteral("x11")))
        return SessionType::X11;
    return SessionType::Unknown;
}

bool isWayland()
{
    return sessionType() == SessionType::Wayland;
}

bool isX11()
{
    return sessionType() == SessionType::X11;
}

bool isPortalAvailable()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected())
        return false;
    return bus.interface()
            && bus.interface()->isServiceRegistered(QLatin1String(kPortalService))
            .value();
}

bool isPortalInterfaceAvailable(const QString &interfaceName)
{
    if (!isPortalAvailable())
        return false;
    return interfaceExists(QLatin1String(kPortalService), QLatin1String(kPortalPath), interfaceName);
}

bool isKGlobalAccelAvailable()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected())
        return false;
    return bus.interface()
            && bus.interface()->isServiceRegistered(QStringLiteral("org.kde.kglobalaccel")).value();
}

bool isLayerShellAvailable()
{
#if IMAGEN_HAVE_LAYER_SHELL
    return true;
#else
    // Without the library we can only use the frameless always-on-top
    // fallback, which works but positions overlays less precisely.
    return false;
#endif
}

QString sessionTypeName()
{
    switch (sessionType()) {
    case SessionType::Wayland:
        return QStringLiteral("Wayland");
    case SessionType::X11:
        return QStringLiteral("X11");
    case SessionType::Unknown:
        break;
    }
    return QStringLiteral("Unknown");
}

QList<CapabilityStatus> capabilityReport()
{
    QList<CapabilityStatus> report;

    CapabilityStatus session;
    session.name = QStringLiteral("Display server");
    session.available = sessionType() != SessionType::Unknown;
    session.details = sessionTypeName() + QStringLiteral(" (") + QGuiApplication::platformName() + QLatin1Char(')');
    report.append(session);

    CapabilityStatus portal;
    portal.name = QStringLiteral("XDG screenshot portal");
    portal.available = isPortalInterfaceAvailable(QStringLiteral("org.freedesktop.portal.Screenshot"));
    portal.details = portal.available ? QStringLiteral("available") : QStringLiteral("not implemented");
    report.append(portal);

    CapabilityStatus shortcuts;
    shortcuts.name = QStringLiteral("Global shortcuts portal");
    shortcuts.available = isPortalInterfaceAvailable(QStringLiteral("org.freedesktop.portal.GlobalShortcuts"));
    shortcuts.details = shortcuts.available ? QStringLiteral("available") : QStringLiteral("not implemented");
    report.append(shortcuts);

    CapabilityStatus kglobalaccel;
    kglobalaccel.name = QStringLiteral("KDE global accelerators");
    kglobalaccel.available = isKGlobalAccelAvailable();
    kglobalaccel.details = kglobalaccel.available ? QStringLiteral("available") : QStringLiteral("not available");
    report.append(kglobalaccel);

    CapabilityStatus layerShell;
    layerShell.name = QStringLiteral("Layer-shell overlays");
    layerShell.available = isLayerShellAvailable();
    layerShell.details = layerShell.available
            ? QStringLiteral("available")
            : QStringLiteral("using frameless always-on-top fallback");
    report.append(layerShell);

    return report;
}

} // namespace imagen::Platform
