// SPDX-License-Identifier: MIT
#include "capture/PortalCaptureBackend.h"

#include "common/AppInfo.h"
#include "common/Logging.h"
#include "common/Platform.h"
#include "common/PortalRequest.h"
#include "common/ScreenGeometry.h"
#include "capture/CursorCompositor.h"

#include <QCursor>
#include <QFileInfo>
#include <QGuiApplication>
#include <QScreen>
#include <QUrl>

namespace imagen {
namespace {

/// The portal returns the whole virtual desktop for non interactive captures.
bool requiresWholeDesktop(CaptureKind kind)
{
    switch (kind) {
    case CaptureKind::FullScreen:
    case CaptureKind::Monitor:
    case CaptureKind::FixedRegion:
    case CaptureKind::LastCapture:
        return true;
    default:
        return false;
    }
}

} // namespace

PortalCaptureBackend::PortalCaptureBackend(QObject *parent)
    : CaptureBackend(parent)
    , m_portal(new PortalRequest(this))
{
}

PortalCaptureBackend::~PortalCaptureBackend() = default;

QString PortalCaptureBackend::id() const
{
    return QStringLiteral("portal");
}

QString PortalCaptureBackend::displayName() const
{
    return QStringLiteral("XDG desktop portal");
}

bool PortalCaptureBackend::isAvailable() const
{
    return Platform::isPortalInterfaceAvailable(QStringLiteral("org.freedesktop.portal.Screenshot"));
}

bool PortalCaptureBackend::supports(CaptureKind kind) const
{
    Q_UNUSED(kind)
    return isAvailable();
}

void PortalCaptureBackend::requestCapture(const CaptureRequest &request)
{
    if (m_pending) {
        fail(tr("A capture is already in progress."));
        return;
    }

    if (!m_portal->connection().isConnected()) {
        fail(tr("The session bus is not available."));
        return;
    }

    m_pending = true;
    m_handled = false;
    m_request = request;

    QVariantMap options;
    options.insert(QStringLiteral("interactive"), !requiresWholeDesktop(request.kind));
    options.insert(QStringLiteral("modal"), true);
    options.insert(QStringLiteral("permission_store"), QLatin1String(AppInfo::AppId));

    qCInfo(lcCapture) << "requesting portal screenshot, interactive:"
                      << options.value(QStringLiteral("interactive"));

    m_portal->call(QStringLiteral("org.freedesktop.portal.Screenshot"),
                   QStringLiteral("Screenshot"),
                   {QString()},
                   options,
                   [this](quint32 code, const QVariantMap &results) { handleResponse(code, results); });
}

void PortalCaptureBackend::handleResponse(quint32 code, const QVariantMap &results)
{
    if (m_handled)
        return;

    if (code == 1) {
        m_handled = true;
        m_pending = false;
        qCInfo(lcCapture) << "capture cancelled by the user";
        Q_EMIT captureCancelled();
        return;
    }

    if (code != 0) {
        fail(tr("The screenshot portal returned error code %1.").arg(code));
        return;
    }

    const QString uri = results.value(QStringLiteral("uri")).toString();
    if (uri.isEmpty()) {
        fail(tr("The portal did not return an image."));
        return;
    }

    const QString path = QUrl(uri).toLocalFile();
    QImage image;
    if (!path.isEmpty() && QFileInfo::exists(path))
        image.load(path);

    if (image.isNull()) {
        fail(tr("The captured image could not be read (%1).").arg(path));
        return;
    }

    m_handled = true;
    m_pending = false;

    CapturedFrame frame;
    frame.image = image;
    frame.backendName = id();
    frame.devicePixelRatio = 1.0;
    frame.sourceGeometry = QRect(QPoint(0, 0), image.size());
    frame.cursorPosition = QCursor::pos();

    Q_EMIT frameReady(cropToScreen(frame, m_request));
}

CapturedFrame PortalCaptureBackend::cropToScreen(const CapturedFrame &frame, const CaptureRequest &request) const
{
    if (!requiresWholeDesktop(request.kind))
        return frame;

    const QList<QScreen *> screens = QGuiApplication::screens();
    if (screens.isEmpty())
        return frame;

    // Absolute logical bounds of the whole desktop.
    QRect logicalBounds;
    for (QScreen *screen : screens)
        logicalBounds = logicalBounds.united(screen->geometry());

    CapturedFrame result = frame;
    QRect crop;

    switch (request.kind) {
    case CaptureKind::Monitor: {
        if (!request.screen)
            return frame;
        crop = mapLogicalRectToImage(request.screen->geometry(), logicalBounds, frame.image.size());
        result.sourceGeometry = request.screen->geometry();
        result.devicePixelRatio = request.screen->devicePixelRatio();
        break;
    }
    case CaptureKind::FixedRegion:
    case CaptureKind::LastCapture: {
        if (request.region.isEmpty())
            return frame;
        crop = mapLogicalRectToImage(request.region, logicalBounds, frame.image.size());
        result.sourceGeometry = request.region;
        result.devicePixelRatio = 1.0;
        break;
    }
    case CaptureKind::FullScreen:
    default:
        result.sourceGeometry = logicalBounds;
        result.devicePixelRatio = 1.0;
        return result;
    }

    if (crop.isEmpty()) {
        qCWarning(lcCapture) << "crop rectangle is empty, using the full image";
        return frame;
    }

    result.image = frame.image.copy(crop);
    if (result.image.isNull())
        result.image = frame.image;

    // The portal does not composite the pointer, so it is drawn from the
    // cursor theme at the position it had when the frame was taken.
    if (m_request.includeCursor) {
        const qreal scaleX = qreal(frame.image.width()) / qreal(logicalBounds.width());
        const qreal scaleY = qreal(frame.image.height()) / qreal(logicalBounds.height());
        const QPoint imagePos(qRound((frame.cursorPosition.x() - logicalBounds.left()) * scaleX),
                              qRound((frame.cursorPosition.y() - logicalBounds.top()) * scaleY));
        result.cursorIncluded = compositeCursor(result.image, imagePos - crop.topLeft());
    }

    qCDebug(lcCapture) << "cropped desktop image" << frame.image.size() << "->" << result.image.size()
                        << "crop" << crop;
    return result;
}

void PortalCaptureBackend::fail(const QString &message)
{
    m_handled = true;
    m_pending = false;
    qCWarning(lcCapture) << "capture failed:" << message;
    Q_EMIT captureFailed(message);
}

void PortalCaptureBackend::cancel()
{
    if (!m_pending)
        return;
    m_handled = true;
    m_pending = false;
    Q_EMIT captureCancelled();
}

} // namespace imagen
