// SPDX-License-Identifier: MIT
#include "capture/CaptureTypes.h"

#include <QCoreApplication>

namespace imagen {

QString captureKindName(CaptureKind kind)
{
    switch (kind) {
    case CaptureKind::FullScreen:
        return QCoreApplication::translate("CaptureKind", "Full screen");
    case CaptureKind::Monitor:
        return QCoreApplication::translate("CaptureKind", "Single monitor");
    case CaptureKind::ActiveWindow:
        return QCoreApplication::translate("CaptureKind", "Active window");
    case CaptureKind::Window:
        return QCoreApplication::translate("CaptureKind", "Window");
    case CaptureKind::Region:
        return QCoreApplication::translate("CaptureKind", "Region");
    case CaptureKind::FixedRegion:
        return QCoreApplication::translate("CaptureKind", "Fixed region");
    case CaptureKind::FreehandRegion:
        return QCoreApplication::translate("CaptureKind", "Freehand region");
    case CaptureKind::LastCapture:
        return QCoreApplication::translate("CaptureKind", "Repeat last capture");
    }
    return {};
}

QString captureKindId(CaptureKind kind)
{
    switch (kind) {
    case CaptureKind::FullScreen:
        return QStringLiteral("fullscreen");
    case CaptureKind::Monitor:
        return QStringLiteral("monitor");
    case CaptureKind::ActiveWindow:
        return QStringLiteral("activewindow");
    case CaptureKind::Window:
        return QStringLiteral("window");
    case CaptureKind::Region:
        return QStringLiteral("region");
    case CaptureKind::FixedRegion:
        return QStringLiteral("fixedregion");
    case CaptureKind::FreehandRegion:
        return QStringLiteral("freehandregion");
    case CaptureKind::LastCapture:
        return QStringLiteral("lastcapture");
    }
    return {};
}

CaptureKind captureKindFromId(const QString &id)
{
    const QString normalized = id.trimmed().toLower();
    if (normalized == QLatin1String("fullscreen"))
        return CaptureKind::FullScreen;
    if (normalized == QLatin1String("monitor"))
        return CaptureKind::Monitor;
    if (normalized == QLatin1String("activewindow"))
        return CaptureKind::ActiveWindow;
    if (normalized == QLatin1String("window"))
        return CaptureKind::Window;
    if (normalized == QLatin1String("region"))
        return CaptureKind::Region;
    if (normalized == QLatin1String("fixedregion"))
        return CaptureKind::FixedRegion;
    if (normalized == QLatin1String("freehandregion"))
        return CaptureKind::FreehandRegion;
    if (normalized == QLatin1String("lastcapture"))
        return CaptureKind::LastCapture;
    return CaptureKind::FullScreen;
}

} // namespace imagen
