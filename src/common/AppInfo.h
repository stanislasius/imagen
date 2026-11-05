// SPDX-License-Identifier: MIT
#pragma once

#include <QString>

namespace imagen {

/**
 * Compile-time identity of the application.
 *
 * The identifiers here end up in D-Bus service names, the desktop entry,
 * the AppStream metadata and the on-disk configuration directory, so they
 * are part of the public contract of the project.
 */
namespace AppInfo {

/// Human readable application name.
inline constexpr const char *ApplicationName = "imagen";

/// Reverse-DNS application identifier (desktop entry, AppStream, Flatpak).
inline constexpr const char *AppId = "org.imagen.Imagen";

/// Desktop file base name (without the .desktop suffix).
inline constexpr const char *DesktopFileName = "org.imagen.Imagen";

/// D-Bus well-known bus name prefix used for portal handle tokens.
inline constexpr const char *DbusBusPrefix = "org.imagen";

/// Version string reported by --version.
QString version();

/// Name of the QSettings ini file inside the configuration directory.
QString settingsFileName();

/// Absolute path of the user configuration directory (created on demand).
QString configDirPath();

/// Absolute path of the settings ini file.
QString settingsFilePath();

/// Absolute path of the cache directory (created on demand).
QString cacheDirPath();

/// Directory holding bundled sound effects.
QString soundDirPath();

} // namespace AppInfo
} // namespace imagen
