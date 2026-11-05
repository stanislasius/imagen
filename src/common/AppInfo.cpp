// SPDX-License-Identifier: MIT
#include "common/AppInfo.h"

#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>

namespace imagen::AppInfo {

QString version()
{
    return QStringLiteral(IMAGEN_VERSION);
}

QString settingsFileName()
{
    return QStringLiteral("imagen.ini");
}

QString configDirPath()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    if (path.isEmpty()) {
        path = QDir::homePath() + QStringLiteral("/.config");
    }
    path += QLatin1Char('/') + QLatin1String(ApplicationName);
    QDir().mkpath(path);
    return path;
}

QString settingsFilePath()
{
    return configDirPath() + QLatin1Char('/') + settingsFileName();
}

QString cacheDirPath()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation);
    if (path.isEmpty()) {
        path = QDir::homePath() + QStringLiteral("/.cache");
    }
    path += QLatin1Char('/') + QLatin1String(ApplicationName);
    QDir().mkpath(path);
    return path;
}

QString soundDirPath()
{
    // Installed layout first, then a source-tree fallback for development.
    const QString installed = QCoreApplication::applicationDirPath()
            + QStringLiteral("/../share/imagen/sounds");
    if (QDir(installed).exists()) {
        return QDir(installed).absolutePath();
    }

    QDir dir(QCoreApplication::applicationDirPath());
    for (int i = 0; i < 4; ++i) {
        const QString candidate = dir.absoluteFilePath(QStringLiteral("assets/sounds"));
        if (QDir(candidate).exists()) {
            return candidate;
        }
        if (!dir.cdUp()) {
            break;
        }
    }
    return {};
}

} // namespace imagen::AppInfo
