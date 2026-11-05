# Architecture

```
imagen
├── src/
│   ├── app/         main(), single instance, wiring of all subsystems
│   ├── capture/     capture backends, history, countdown
│   ├── common/      logging, platform detection, geometry, portal client
│   ├── editor/      image window (M2)
│   ├── hotkeys/     global shortcut providers and the action registry
│   ├── outputs/     clipboard, file, auto-save, external program
│   ├── overlays/    transient windows (selection overlay, base class)
│   ├── settings/    configuration store and settings dialog
│   ├── system/      notifications and sound
│   └── tray/        tray icon and main menu
├── assets/          own icons and the generated shutter sound
├── data/            desktop entry and AppStream metadata
├── docs/            this documentation
└── tests/           Qt Test based unit tests
```

The code is split into a static library `imagen_core` and a thin
executable `imagen`. Everything except `main()` lives in the library, which
keeps the unit tests independent of the application.

## Capture flow

```
hotkey / tray menu / --capture
        │
        ▼
CaptureManager::requestCapture(kind)
        │  resolves "repeat last" against the CaptureHistory
        │  starts the delay countdown when a delay is configured
        ▼
CaptureBackend::requestCapture(request)
        │  portal (Wayland) or X11
        ▼
CapturedFrame  ──►  Application::onFrameReady
                        ├─► clipboard (when "always copy" is on)
                        ├─► OutputRouter  (editor, file, auto-save, …)
                        ├─► sound and notification
                        └─► CaptureHistory (for "repeat last")
```

### Backends

`CaptureBackend` is a one-shot, asynchronous producer of images. It
receives a `CaptureRequest` and answers with exactly one of `frameReady()`,
`captureCancelled()` or `captureFailed()`.

* `PortalCaptureBackend` — Wayland, uses `org.freedesktop.portal.Screenshot`.
* `X11CaptureBackend` — X11, uses `XShm`/`XGetImage` on the root window.

`CaptureManager` picks the first backend that is available and supports the
requested kind. The decision is based on `QGuiApplication::platformName()`
and on the portal interfaces that are actually implemented.

### Coordinate systems

Three coordinate systems meet in the capture code:

| System | Used by |
|---|---|
| logical (DIP) | `QScreen::geometry()`, the selection overlay, the settings |
| device pixels | the X11 backend, the crop rectangles |
| image pixels | the image that comes back from a backend |

`src/common/ScreenGeometry.h` converts between them. Two functions matter
most:

* `mapLogicalRectToImage()` places a logical rectangle on an image that
  covers the whole desktop (the portal case).
* `mapDeviceRectToImage()` places a device pixel rectangle on an image that
  may be smaller than the real desktop resolution.

All conversions are covered by `tests/tst_geometry.cpp`.

## Portal client

`src/common/PortalRequest.h` implements the request/response dance of the
XDG desktop portal:

1. a unique `handle_token` is generated and put into the options,
2. the path of the request object is predicted from the token and the
   unique name of the bus connection,
3. the `Response` signal is subscribed **before** the call is issued, so a
   fast response cannot be missed,
4. the result arrives through `org.freedesktop.portal.Request::Response`.

Two details are worth remembering:

* the token must only contain `[A-Za-z0-9_]`, because it becomes the last
  element of an object path. A dot or a dash makes the portal crash.
* the bus connection handle has to be kept alive for the lifetime of the
  client, otherwise the connection is closed while calls are in flight.

## Hotkeys

`HotkeyManager` resolves the configured shortcuts through the first
available provider:

1. `PortalShortcutsBackend` — `org.freedesktop.portal.GlobalShortcuts`,
2. `X11HotkeysBackend` — `XGrabKey` on the root window.

The action registry (`HotkeyAction`) maps stable identifiers such as
`capture.fullscreen` to their default shortcut. The bindings themselves
live in the configuration file and can be changed in the settings dialog.

The portal describes a shortcut as a list of key names in the XKB naming
scheme (`CTRL`, `ALT`, `SHIFT`, `LOGO`, `Print`, `space`, …); the mapping
from a Qt key sequence to those names lives in `src/hotkeys/KeyNames.h`
and is shared with the X11 backend.

## Outputs

`OutputRouter` sends a captured frame to the destination from the settings.
Every output reports its result through a signal so the application can
show a notification.

## Configuration

`Config` is a thin, typed wrapper around `QSettings` with an ini file in
`~/.config/imagen/imagen.ini`. The dialog writes through the same class,
so the file stays the single source of truth.
