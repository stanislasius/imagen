// SPDX-License-Identifier: MIT
#pragma once

#include <QString>
#include <QStringList>

namespace imagen {

/**
 * The display server family the application is running on.
 */
enum class SessionType {
    Unknown,
    Wayland,
    X11,
};

/**
 * One line describing the state of a platform capability. Used by the
 * diagnostics dialog so users can see *why* a feature is unavailable.
 */
struct CapabilityStatus {
    QString name;
    bool available = false;
    QString details;
};

namespace Platform {

/// The session type the application is currently running under.
SessionType sessionType();

/// Convenience: true when running under a Wayland compositor.
bool isWayland();

/// Convenience: true when running under X11.
bool isX11();

/// True when an xdg-desktop-portal implementation is reachable on the bus.
bool isPortalAvailable();

/// True when the given portal interface (e.g. "Screenshot") is implemented.
bool isPortalInterfaceAvailable(const QString &interfaceName);

/// True when the KDE global accelerator service is reachable.
bool isKGlobalAccelAvailable();

/// True when a compositor implementing the layer-shell protocol was detected.
bool isLayerShellAvailable();

/// Human readable name of the detected session.
QString sessionTypeName();

/// Full capability report, ordered by relevance for the user.
QList<CapabilityStatus> capabilityReport();

} // namespace Platform
} // namespace imagen
