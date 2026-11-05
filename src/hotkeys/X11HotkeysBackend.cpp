// SPDX-License-Identifier: MIT
#include "hotkeys/X11HotkeysBackend.h"

#include "common/Logging.h"
#include "common/Platform.h"
#include "hotkeys/KeyNames.h"

#include <QKeySequence>

#if IMAGEN_HAVE_X11
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#endif

namespace imagen {
namespace {

/// Modifier bits that are ignored while matching a key event.
#if IMAGEN_HAVE_X11
constexpr unsigned int kLockMask = LockMask | Mod2Mask | Mod5Mask;
#else
constexpr unsigned int kLockMask = 0;
#endif

} // namespace

X11HotkeysBackend::X11HotkeysBackend(QObject *parent)
    : HotkeyProvider(parent)
    , m_timer(new QTimer(this))
{
    m_timer->setInterval(20);
    connect(m_timer, &QTimer::timeout, this, &X11HotkeysBackend::pollEvents);
}

X11HotkeysBackend::~X11HotkeysBackend()
{
    unregisterShortcuts();
    closeDisplay();
}

QString X11HotkeysBackend::id() const
{
    return QStringLiteral("x11");
}

QString X11HotkeysBackend::displayName() const
{
    return QStringLiteral("X11 keyboard grab");
}

bool X11HotkeysBackend::isAvailable() const
{
#if IMAGEN_HAVE_X11
    return Platform::isX11() && ensureDisplay();
#else
    return false;
#endif
}

bool X11HotkeysBackend::registerShortcuts(const QHash<QString, HotkeySequence> &shortcuts)
{
    if (!isAvailable())
        return false;

    unregisterShortcuts();

    for (auto it = shortcuts.constBegin(); it != shortcuts.constEnd(); ++it)
        grab(it.key(), it.value());

    if (!m_timer->isActive())
        m_timer->start();

    qCInfo(lcHotkeys) << "grabbed" << m_grabs.size() << "shortcuts";
    return true;
}

void X11HotkeysBackend::unregisterShortcuts()
{
#if IMAGEN_HAVE_X11
    if (!m_display)
        return;

    auto *display = static_cast<Display *>(m_display);
    for (const Grab &grab : m_grabs) {
        for (unsigned int variant = 0; variant < 8; ++variant) {
            const unsigned int lockBits = (variant & 1 ? LockMask : 0)
                    | (variant & 2 ? Mod2Mask : 0)
                    | (variant & 4 ? Mod5Mask : 0);
            XUngrabKey(display, grab.keycode, grab.modifiers | lockBits, m_rootWindow);
        }
    }
#endif
    m_grabs.clear();
    if (m_timer->isActive())
        m_timer->stop();
}

void X11HotkeysBackend::grab(const QString &actionId, const HotkeySequence &sequence)
{
#if IMAGEN_HAVE_X11
    if (!m_display)
        return;

    auto *display = static_cast<Display *>(m_display);

    const QString keyName = KeyNames::forKey(sequence.toKeySequence());
    if (keyName.isEmpty()) {
        qCWarning(lcHotkeys) << "no X11 representation for shortcut" << sequence.toString();
        return;
    }

    const KeySym keysym = XStringToKeysym(keyName.toLatin1().constData());
    if (keysym == NoSymbol) {
        qCWarning(lcHotkeys) << "unknown key" << keyName;
        return;
    }

    const KeyCode keycode = XKeysymToKeycode(display, keysym);
    if (keycode == 0) {
        qCWarning(lcHotkeys) << "key" << keyName << "is not present on the keyboard";
        return;
    }

    const QKeyCombination combination = sequence.toKeySequence()[0];
    const Qt::KeyboardModifiers modifiers = combination.keyboardModifiers();

    unsigned int modifierMask = 0;
    if (modifiers & Qt::ShiftModifier)
        modifierMask |= ShiftMask;
    if (modifiers & Qt::ControlModifier)
        modifierMask |= ControlMask;
    if (modifiers & Qt::AltModifier)
        modifierMask |= Mod1Mask;
    if (modifiers & Qt::MetaModifier)
        modifierMask |= Mod4Mask;

    for (unsigned int variant = 0; variant < 8; ++variant) {
        const unsigned int lockBits = (variant & 1 ? LockMask : 0)
                | (variant & 2 ? Mod2Mask : 0)
                | (variant & 4 ? Mod5Mask : 0);
        XGrabKey(display, keycode, modifierMask | lockBits, m_rootWindow,
                 False, GrabModeAsync, GrabModeAsync);
    }

    m_grabs.append(Grab{actionId, keycode, modifierMask});
#endif
}

void X11HotkeysBackend::pollEvents()
{
#if IMAGEN_HAVE_X11
    if (!m_display)
        return;

    auto *display = static_cast<Display *>(m_display);

    while (XPending(display) > 0) {
        XEvent event;
        XNextEvent(display, &event);
        if (event.type != KeyPress)
            continue;

        const unsigned int state = event.xkey.state & ~kLockMask;
        for (const Grab &grab : m_grabs) {
            if (grab.keycode == event.xkey.keycode && grab.modifiers == state) {
                qCInfo(lcHotkeys) << "shortcut activated:" << grab.actionId;
                Q_EMIT activated(grab.actionId);
                break;
            }
        }
    }
#endif
}

#if IMAGEN_HAVE_X11

bool X11HotkeysBackend::ensureDisplay() const
{
    if (m_display)
        return true;

    Display *display = XOpenDisplay(nullptr);
    if (!display)
        return false;

    m_display = display;
    m_rootWindow = DefaultRootWindow(display);
    return true;
}

void X11HotkeysBackend::closeDisplay()
{
    if (!m_display)
        return;
    XCloseDisplay(static_cast<Display *>(m_display));
    m_display = nullptr;
    m_rootWindow = 0;
}

#else

bool X11HotkeysBackend::ensureDisplay()
{
    return false;
}

void X11HotkeysBackend::closeDisplay() {}

#endif

} // namespace imagen
