# Wayland notes

Everything on Wayland goes through the XDG desktop portal. This document
records what works, what does not and why.

## Screenshot portal

`org.freedesktop.portal.Screenshot` (version 2) is the only portable way
to read the screen on a Wayland compositor.

| Capture kind | Portal call | Result |
|---|---|---|
| full screen | `interactive=false` | image of the whole virtual desktop, cropped client side |
| single monitor | `interactive=false` | image of the whole virtual desktop, cropped to the monitor |
| fixed region | `interactive=false` | image of the whole virtual desktop, cropped to the stored rectangle |
| repeat last | `interactive=false` | image of the whole virtual desktop, cropped to the stored rectangle |
| region | `interactive=true` | the selection of the compositor, already cropped |
| window | `interactive=true` | the selection of the compositor, already cropped |
| freehand | `interactive=true` | the selection of the compositor, already cropped |

### Why the whole desktop is requested for full screen captures

The portal offers no way to ask for a single monitor. Requesting the whole
desktop and cropping it is the standard approach. The client side crop uses
the logical geometry of the screen and the device pixel ratio of the
result, which is exact for a uniform scaling.

### The cursor

The portal does not composite the pointer. When "include the mouse cursor"
is enabled, `imagen` draws the cursor from the current cursor theme at the
position the pointer had when the frame was taken. On X11 the pointer is
part of the captured image itself.

### The active window

There is no portal interface that reports the focused window. On Wayland
`imagen` therefore treats "active window" like "window": the compositor
shows its selection interface. The X11 backend can determine the focused
window through `_NET_ACTIVE_WINDOW`.

## Global shortcuts

`org.freedesktop.portal.GlobalShortcuts` (version 1) is implemented by
Plasma 6 and by some other compositors.

The flow is:

1. `CreateSession` with a `session_handle_token`,
2. `BindShortcuts` with the list of shortcuts,
3. the compositor reports an activation with the `Activated` signal on the
   session object.

The shortcut list has the D-Bus type `a(sa{sv})`. Qt marshals it from
`QList<std::pair<QString, QVariantMap>>`, which is the type the portal
documents in its introspection data. Both the pair and the list have to be
registered with `qDBusRegisterMetaType`, otherwise the marshaller refuses
the message and closes the connection.

The session object is owned by the portal frontend
(`org.freedesktop.portal.Desktop`), so the signal subscription names that
service.

## Permissions

The first screenshot of a session may ask for permission. Plasma remembers
the decision for the application, other compositors ask again. The
`permission_store` option is set to the application id so the portal can
remember it.

## Overlays

Overlays (region picker, magnifier, ruler, whiteboard) are placed on the
layer-shell overlay layer when LayerShellQt is available. Without it a
frameless always-on-top window is used, which works everywhere but is
placed less precisely.

## Known limitations

| Limitation | Reason |
|---|---|
| no pixel access without a snapshot | the compositor does not allow reading the screen |
| the active window cannot be determined | no portal interface for it |
| the cursor is approximated | the portal does not composite it |
| a window capture is a selection capture | the portal has no window mode |
| overlays may be placed imprecisely without layer-shell | see above |
