// SPDX-License-Identifier: MIT
#include "capture/CaptureManager.h"

#include "capture/CaptureHistory.h"
#include "capture/DelayCountdown.h"
#include "capture/PortalCaptureBackend.h"
#include "capture/X11CaptureBackend.h"
#include "common/Logging.h"
#include "common/Platform.h"
#include "settings/Config.h"

#include <QCursor>
#include <QGuiApplication>
#include <QScreen>

namespace imagen {

CaptureManager::CaptureManager(QObject *parent)
    : QObject(parent)
    , m_history(new CaptureHistory(this))
{
#if IMAGEN_HAVE_PORTAL
    auto *portal = new PortalCaptureBackend(this);
    m_backends.append(portal);
#endif
#if IMAGEN_HAVE_X11
    auto *x11 = new X11CaptureBackend(this);
    m_backends.append(x11);
#endif

    for (CaptureBackend *backend : m_backends) {
        connect(backend, &CaptureBackend::frameReady, this, &CaptureManager::onBackendFrame);
        connect(backend, &CaptureBackend::captureCancelled, this, &CaptureManager::onBackendCancelled);
        connect(backend, &CaptureBackend::captureFailed, this, &CaptureManager::onBackendFailed);
    }
}

CaptureManager::~CaptureManager() = default;

bool CaptureManager::hasBackend() const
{
    for (CaptureBackend *backend : preferredBackends()) {
        if (backend->isAvailable())
            return true;
    }
    return false;
}

QString CaptureManager::backendName() const
{
    for (CaptureBackend *backend : preferredBackends()) {
        if (backend->isAvailable())
            return backend->displayName();
    }
    return {};
}

QList<CaptureBackend *> CaptureManager::backends() const
{
    return m_backends;
}

void CaptureManager::requestCapture(CaptureKind kind)
{
    CaptureRequest request;
    request.kind = kind;

    if (kind == CaptureKind::LastCapture) {
        request.kind = m_history->lastKind();
        request.region = m_history->lastRegion();
        if (request.kind == CaptureKind::Region)
            request.kind = CaptureKind::FixedRegion;
    }

    requestCapture(request);
}

void CaptureManager::requestCapture(const CaptureRequest &request)
{
    m_pendingRequest = request;

    // A monitor capture without a screen would have nothing to capture.
    if (m_pendingRequest.kind == CaptureKind::Monitor && !m_pendingRequest.screen) {
        const QPoint cursor = QCursor::pos();
        for (QScreen *screen : QGuiApplication::screens()) {
            if (screen->geometry().contains(cursor)) {
                m_pendingRequest.screen = screen;
                break;
            }
        }
        if (!m_pendingRequest.screen)
            m_pendingRequest.screen = QGuiApplication::primaryScreen();
    }

    const int delay = Config::instance().captureDelay();
    if (delay > 0) {
        if (!m_countdown) {
            m_countdown = new DelayCountdown();
            connect(m_countdown, &DelayCountdown::finished, this, [this]() {
                startBackendCapture(m_pendingRequest);
            });
            connect(m_countdown, &DelayCountdown::cancelled, this, [this]() {
                Q_EMIT captureCancelled();
            });
        }
        m_countdown->start(delay);
        return;
    }

    startBackendCapture(m_pendingRequest);
}

void CaptureManager::startBackendCapture(const CaptureRequest &request)
{
    CaptureBackend *backend = nullptr;
    for (CaptureBackend *candidate : preferredBackends()) {
        if (candidate->isAvailable() && candidate->supports(request.kind)) {
            backend = candidate;
            break;
        }
    }

    if (!backend) {
        Q_EMIT captureFailed(tr("No capture backend is available in this session."));
        return;
    }

    m_activeBackend = backend;
    qCInfo(lcCapture) << "starting capture with backend" << backend->id();
    backend->requestCapture(request);
}

/**
 * The backends in the order they are tried.
 *
 * A Wayland session prefers the portal, because it is the only way to
 * read the screen. An X11 session talks to the server directly, which is
 * faster and offers the pointer and a client side region picker. The other
 * backend stays available as a fallback.
 */
QList<CaptureBackend *> CaptureManager::preferredBackends() const
{
    QList<CaptureBackend *> ordered;

    const bool wayland = Platform::isWayland();

    for (CaptureBackend *backend : m_backends) {
        const bool isPortal = backend->id() == QLatin1String("portal");
        if (wayland == isPortal)
            ordered.append(backend);
    }
    for (CaptureBackend *backend : m_backends) {
        const bool isPortal = backend->id() == QLatin1String("portal");
        if (wayland != isPortal)
            ordered.append(backend);
    }

    return ordered;
}

void CaptureManager::cancel()
{
    if (m_countdown && m_countdown->isRunning()) {
        m_countdown->stop();
        Q_EMIT captureCancelled();
        return;
    }
    if (m_activeBackend)
        m_activeBackend->cancel();
}

void CaptureManager::onBackendFrame(const imagen::CapturedFrame &frame)
{
    m_history->remember(m_pendingRequest);
    Q_EMIT frameReady(frame);
}

void CaptureManager::onBackendCancelled()
{
    Q_EMIT captureCancelled();
}

void CaptureManager::onBackendFailed(const QString &message)
{
    Q_EMIT captureFailed(message);
}

} // namespace imagen
