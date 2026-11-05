// SPDX-License-Identifier: MIT
#include "capture/X11CaptureBackend.h"

#include "capture/CursorCompositor.h"
#include "common/Logging.h"
#include "common/Platform.h"
#include "overlays/SelectionOverlay.h"

#include <QCoreApplication>
#include <QCursor>
#include <QEventLoop>
#include <QGuiApplication>
#include <QScreen>
#include <QTimer>

#if IMAGEN_HAVE_X11
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/extensions/XShm.h>

#include <cstring>
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

namespace imagen {

X11CaptureBackend::X11CaptureBackend(QObject *parent)
    : CaptureBackend(parent)
{
}

X11CaptureBackend::~X11CaptureBackend()
{
    closeDisplay();
}

QString X11CaptureBackend::id() const
{
    return QStringLiteral("x11");
}

QString X11CaptureBackend::displayName() const
{
    return QStringLiteral("X11");
}

bool X11CaptureBackend::isAvailable() const
{
#if IMAGEN_HAVE_X11
    return Platform::isX11() && ensureDisplay();
#else
    return false;
#endif
}

bool X11CaptureBackend::supports(CaptureKind kind) const
{
    Q_UNUSED(kind)
    return isAvailable();
}

#if IMAGEN_HAVE_X11

bool X11CaptureBackend::ensureDisplay() const
{
    if (m_display)
        return true;

    Display *display = XOpenDisplay(nullptr);
    if (!display) {
        qCWarning(lcCapture) << "cannot open the X display";
        return false;
    }

    m_display = display;
    m_screenIndex = DefaultScreen(display);
    m_rootWindow = RootWindow(display, m_screenIndex);

    int major = 0;
    int minor = 0;
    int opcode = 0;
    m_shmAvailable = XShmQueryExtension(display) && XShmQueryVersion(display, &major, &minor, &opcode);
    qCInfo(lcCapture) << "opened X display, shm:" << m_shmAvailable;
    return true;
}

void X11CaptureBackend::closeDisplay()
{
    if (!m_display)
        return;
    XCloseDisplay(static_cast<Display *>(m_display));
    m_display = nullptr;
    m_rootWindow = 0;
}

QRect X11CaptureBackend::screenDeviceRect(QScreen *screen) const
{
    if (!screen)
        return {};
    const qreal ratio = screen->devicePixelRatio() > 0 ? screen->devicePixelRatio() : 1.0;
    const QRect geometry = screen->geometry();
    return QRect(qRound(geometry.x() * ratio), qRound(geometry.y() * ratio),
                  qRound(geometry.width() * ratio), qRound(geometry.height() * ratio));
}

qreal X11CaptureBackend::ratioAt(const QPoint &logicalPosition) const
{
    const QList<QScreen *> screens = QGuiApplication::screens();
    for (QScreen *screen : screens) {
        if (screen->geometry().contains(logicalPosition))
            return screen->devicePixelRatio();
    }
    if (!screens.isEmpty())
        return screens.first()->devicePixelRatio();
    return 1.0;
}

QImage X11CaptureBackend::grabRoot(const QRect &deviceRect)
{
    if (!ensureDisplay())
        return {};

    auto *display = static_cast<Display *>(m_display);
    const int width = qMax(1, deviceRect.width());
    const int height = qMax(1, deviceRect.height());
    const int depth = DefaultDepth(display, m_screenIndex);
    Visual *visual = DefaultVisual(display, m_screenIndex);

    XImage *xImage = nullptr;
    XShmSegmentInfo shmInfo;
    bool shared = false;
    std::memset(&shmInfo, 0, sizeof(shmInfo));

    if (m_shmAvailable) {
        shmInfo.shmid = shmget(IPC_PRIVATE, static_cast<size_t>(width) * height * 4, IPC_CREAT | 0600);
        if (shmInfo.shmid >= 0) {
            shmInfo.shmaddr = static_cast<char *>(shmat(shmInfo.shmid, nullptr, 0));
            if (shmInfo.shmaddr != reinterpret_cast<char *>(-1)) {
                // The image has to use the shared segment, otherwise the
                // server cannot write into it.
                xImage = XShmCreateImage(display, visual, depth, ZPixmap, shmInfo.shmaddr,
                                          &shmInfo, width, height);
                if (xImage) {
                    if (XShmAttach(display, &shmInfo)
                        && XShmGetImage(display, m_rootWindow, xImage, deviceRect.x(), deviceRect.y(),
                                        AllPlanes)) {
                        shared = true;
                    } else {
                        qCDebug(lcCapture) << "the shared memory path failed, using XGetImage";
                        XDestroyImage(xImage);
                        xImage = nullptr;
                        shmdt(shmInfo.shmaddr);
                        shmctl(shmInfo.shmid, IPC_RMID, nullptr);
                    }
                } else {
                    shmdt(shmInfo.shmaddr);
                    shmctl(shmInfo.shmid, IPC_RMID, nullptr);
                }
            } else {
                shmctl(shmInfo.shmid, IPC_RMID, nullptr);
            }
        }
    }

    if (!xImage) {
        xImage = XGetImage(display, m_rootWindow, deviceRect.x(), deviceRect.y(),
                           static_cast<unsigned int>(width), static_cast<unsigned int>(height),
                           AllPlanes, ZPixmap);
    }

    if (!xImage) {
        qCWarning(lcCapture) << "XGetImage failed";
        if (shared) {
            shmdt(shmInfo.shmaddr);
            shmctl(shmInfo.shmid, IPC_RMID, nullptr);
        }
        return {};
    }

    QImage image;
    if (xImage->depth == 24 && xImage->bits_per_pixel == 32) {
        image = QImage(xImage->width, xImage->height, QImage::Format_RGB32);
        for (int y = 0; y < xImage->height; ++y) {
            std::memcpy(image.scanLine(y),
                        xImage->data + static_cast<qsizetype>(y) * xImage->bytes_per_line,
                        static_cast<size_t>(xImage->width) * 4);
        }
    } else if (xImage->depth == 16) {
        image = QImage(xImage->width, xImage->height, QImage::Format_RGB16);
        for (int y = 0; y < xImage->height; ++y) {
            auto *target = reinterpret_cast<quint16 *>(image.scanLine(y));
            const auto *source = reinterpret_cast<const quint16 *>(
                    xImage->data + static_cast<qsizetype>(y) * xImage->bytes_per_line);
            for (int x = 0; x < xImage->width; ++x)
                target[x] = source[x];
        }
    } else {
        qCWarning(lcCapture) << "unsupported image depth" << xImage->depth;
    }

    if (shared) {
        xImage->data = nullptr; // the buffer belongs to the shared segment
        XShmDetach(display, &shmInfo);
        XDestroyImage(xImage);
        shmdt(shmInfo.shmaddr);
        shmctl(shmInfo.shmid, IPC_RMID, nullptr);
    } else {
        XDestroyImage(xImage);
    }
    return image;
}

unsigned long X11CaptureBackend::activeWindow()
{
    if (!ensureDisplay())
        return 0;

    auto *display = static_cast<Display *>(m_display);
    const Atom atom = XInternAtom(display, "_NET_ACTIVE_WINDOW", True);
    if (atom == None)
        return 0;

    Atom actualType = None;
    int actualFormat = 0;
    unsigned long items = 0;
    unsigned long bytesAfter = 0;
    unsigned char *property = nullptr;

    if (XGetWindowProperty(display, m_rootWindow, atom, 0, 1, False, XA_WINDOW,
                           &actualType, &actualFormat, &items, &bytesAfter, &property)
            != Success) {
        return 0;
    }

    unsigned long window = 0;
    if (property) {
        if (items > 0)
            window = *reinterpret_cast<unsigned long *>(property);
        XFree(property);
    }
    return window;
}

bool X11CaptureBackend::windowDeviceRect(unsigned long window, QRect *deviceRect) const
{
    auto *display = static_cast<Display *>(m_display);

    XWindowAttributes attributes;
    if (!XGetWindowAttributes(display, window, &attributes))
        return false;

    int absoluteX = 0;
    int absoluteY = 0;
    Window child = 0;
    if (!XTranslateCoordinates(display, window, m_rootWindow, 0, 0, &absoluteX, &absoluteY, &child))
        return false;

    *deviceRect = QRect(absoluteX, absoluteY, attributes.width, attributes.height);
    return true;
}

bool X11CaptureBackend::pickWindowGeometry(QRect *deviceRect)
{
    if (!ensureDisplay())
        return false;

    auto *display = static_cast<Display *>(m_display);
    Window root = m_rootWindow;
    Window returned = 0;
    int x = 0;
    int y = 0;
    int winX = 0;
    int winY = 0;
    unsigned int mask = 0;

    const Cursor cursor = XCreateFontCursor(display, XC_crosshair);
    const int grabbed = XGrabPointer(display, root, False, ButtonPressMask | ButtonReleaseMask,
                                     GrabModeAsync, GrabModeAsync, None, cursor, CurrentTime);
    if (grabbed != GrabSuccess) {
        XFreeCursor(display, cursor);
        qCWarning(lcCapture) << "cannot grab the pointer";
        return false;
    }

    XEvent event;
    bool picked = false;
    for (;;) {
        XWindowEvent(display, root, ButtonPressMask | ButtonReleaseMask, &event);
        if (event.type == ButtonPress) {
            XQueryPointer(display, root, &returned, &event.xbutton.subwindow,
                          &x, &y, &winX, &winY, &mask);
            picked = true;
            break;
        }
        if (event.type == ButtonRelease)
            break;
    }

    XUngrabPointer(display, CurrentTime);
    XFreeCursor(display, cursor);

    if (!picked || returned == 0)
        return false;

    // Walk up the window tree until the top level window is reached.
    Window window = returned;
    while (true) {
        Window parent = 0;
        Window rootReturn = 0;
        if (!XQueryTree(display, window, &rootReturn, &parent, nullptr, nullptr))
            break;
        if (parent == 0 || parent == rootReturn)
            break;
        window = parent;
    }

    return windowDeviceRect(window, deviceRect);
}

bool X11CaptureBackend::pickRegion(QRect *logicalRect)
{
    auto *overlay = new SelectionOverlay();
    m_selectionOverlay = overlay;

    QRect result;
    bool selected = false;

    QEventLoop loop;
    connect(overlay, &SelectionOverlay::regionSelected, &loop, [&](const QRect &region) {
        result = region;
        selected = true;
        loop.quit();
    });
    connect(overlay, &SelectionOverlay::selectionCancelled, &loop, &QEventLoop::quit);
    QTimer::singleShot(120000, &loop, &QEventLoop::quit);

    overlay->beginSelection();
    loop.exec();

    overlay->deleteLater();
    m_selectionOverlay.clear();

    if (!selected)
        return false;

    *logicalRect = result;
    return true;
}

void X11CaptureBackend::requestCapture(const CaptureRequest &request)
{
    if (m_pending) {
        Q_EMIT captureFailed(tr("A capture is already in progress."));
        return;
    }
    if (!ensureDisplay()) {
        Q_EMIT captureFailed(tr("The X display could not be opened."));
        return;
    }

    m_pending = true;
    m_handled = false;
    m_request = request;

    switch (request.kind) {
    case CaptureKind::FullScreen: {
        QRect bounds;
        for (QScreen *screen : QGuiApplication::screens())
            bounds = bounds.united(screenDeviceRect(screen));
        const QImage image = grabRoot(bounds);
        if (image.isNull())
            break;
        deliver(image, bounds, 1.0);
        return;
    }
    case CaptureKind::Monitor: {
        if (!request.screen) {
            Q_EMIT captureFailed(tr("No monitor was selected."));
            return;
        }
        const QRect deviceRect = screenDeviceRect(request.screen);
        const QImage image = grabRoot(deviceRect);
        if (image.isNull())
            break;
        deliver(image, request.screen->geometry(), request.screen->devicePixelRatio());
        return;
    }
    case CaptureKind::ActiveWindow: {
        const unsigned long window = activeWindow();
        if (window == 0) {
            Q_EMIT captureFailed(tr("No active window was found."));
            return;
        }
        QRect deviceRect;
        if (!windowDeviceRect(window, &deviceRect)) {
            Q_EMIT captureFailed(tr("The active window could not be inspected."));
            return;
        }
        const QImage image = grabRoot(deviceRect);
        if (image.isNull())
            break;
        deliver(image, deviceRect, 1.0);
        return;
    }
    case CaptureKind::Window: {
        QRect deviceRect;
        if (!pickWindowGeometry(&deviceRect)) {
            m_pending = false;
            Q_EMIT captureCancelled();
            return;
        }
        const QImage image = grabRoot(deviceRect);
        if (image.isNull())
            break;
        deliver(image, deviceRect, 1.0);
        return;
    }
    case CaptureKind::Region:
    case CaptureKind::FreehandRegion: {
        QRect logicalRect;
        if (!pickRegion(&logicalRect)) {
            m_pending = false;
            Q_EMIT captureCancelled();
            return;
        }
        const qreal ratio = ratioAt(logicalRect.center());
        const QRect deviceRect(qRound(logicalRect.x() * ratio), qRound(logicalRect.y() * ratio),
                               qRound(logicalRect.width() * ratio), qRound(logicalRect.height() * ratio));
        const QImage image = grabRoot(deviceRect);
        if (image.isNull())
            break;
        deliver(image, logicalRect, ratio);
        return;
    }
    case CaptureKind::FixedRegion:
    case CaptureKind::LastCapture: {
        if (request.region.isEmpty()) {
            Q_EMIT captureFailed(tr("No region was remembered yet."));
            return;
        }
        const qreal ratio = ratioAt(request.region.center());
        const QRect deviceRect(qRound(request.region.x() * ratio), qRound(request.region.y() * ratio),
                               qRound(request.region.width() * ratio), qRound(request.region.height() * ratio));
        const QImage image = grabRoot(deviceRect);
        if (image.isNull())
            break;
        deliver(image, request.region, ratio);
        return;
    }
    }

    m_pending = false;
    Q_EMIT captureFailed(tr("The screen could not be captured."));
}

#else // !IMAGEN_HAVE_X11

bool X11CaptureBackend::ensureDisplay()
{
    return false;
}

void X11CaptureBackend::closeDisplay() {}

QRect X11CaptureBackend::screenDeviceRect(QScreen *) const
{
    return {};
}

qreal X11CaptureBackend::ratioAt(const QPoint &) const
{
    return 1.0;
}

QImage X11CaptureBackend::grabRoot(const QRect &)
{
    return {};
}

unsigned long X11CaptureBackend::activeWindow()
{
    return 0;
}

bool X11CaptureBackend::windowDeviceRect(unsigned long, QRect *) const
{
    return false;
}

bool X11CaptureBackend::pickWindowGeometry(QRect *)
{
    return false;
}

bool X11CaptureBackend::pickRegion(QRect *)
{
    return false;
}

void X11CaptureBackend::requestCapture(const CaptureRequest &request)
{
    Q_UNUSED(request)
    Q_EMIT captureFailed(tr("The application was built without X11 support."));
}

#endif // IMAGEN_HAVE_X11

void X11CaptureBackend::deliver(const QImage &image, const QRect &sourceGeometry, qreal devicePixelRatio)
{
    m_pending = false;
    m_handled = true;

    CapturedFrame frame;
    frame.image = image;
    frame.sourceGeometry = sourceGeometry;
    frame.devicePixelRatio = devicePixelRatio > 0 ? devicePixelRatio : 1.0;
    frame.backendName = id();
    frame.cursorPosition = QCursor::pos();

    if (m_request.includeCursor) {
        const QPoint devicePos(qRound(frame.cursorPosition.x() * frame.devicePixelRatio),
                               qRound(frame.cursorPosition.y() * frame.devicePixelRatio));
        frame.cursorIncluded = compositeCursor(frame.image, devicePos - sourceGeometry.topLeft());
    }

    Q_EMIT frameReady(frame);
}

void X11CaptureBackend::cancel()
{
    if (!m_pending)
        return;
    m_pending = false;
    m_handled = true;
    if (m_selectionOverlay)
        m_selectionOverlay->hide();
    Q_EMIT captureCancelled();
}

} // namespace imagen
