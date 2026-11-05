# imagen

Screenshot capture and annotation for Linux.

`imagen` is a clean-room reimplementation of the feature set of PicPick for
Linux desktops. It targets **KDE Plasma 6 on Wayland** first and supports
**X11** as well.

The implementation follows [PLAN.md](PLAN.md). The project ships its own
icons, sounds and translations; no assets of the original application are
used.

---

## Status

| Milestone | State |
|---|---|
| **M0 — Scaffolding** | done: CMake project, Qt 6, logging, single instance, tray icon, settings dialog, desktop entry, tests, CI |
| **M1 — Capture MVP** | in progress: portal backend (full screen, monitor, region, window, freehand, fixed region, repeat), X11 backend, delay countdown, sound, notifications, outputs (clipboard, file, auto-save, external program), tray menu, global shortcuts via the GlobalShortcuts portal and X11 |
| **M2 — Editor core** | started: image window with zoom/pan, open/save; annotations and undo are next |
| **M3 — Effects** | planned |
| **M4 — Tools & overlays** | planned (colour picker, magnifier, ruler, protractor, crosshair, whiteboard, floating widget, capture bar) |
| **M5 — Polish & release** | planned (i18n, themes, diagnostics dialog, Flatpak/AppImage/deb) |

## Features

### Capture

* full screen, a single monitor, the active window, a window, a region,
  a fixed region, a freehand region and a repeat of the last capture
* optional delay with an on-screen countdown
* optional mouse cursor (composited from the cursor theme on Wayland,
  captured directly on X11)
* shutter sound and desktop notification
* always copy the result to the clipboard

### Output

* open in the editor
* copy to the clipboard (PNG and BMP are offered)
* save to a file
* auto-save into a folder with a configurable name pattern
* hand over to an external program (`gimp %f`)
* open the auto-save folder

### Shell

* tray icon with the full capture menu
* global shortcuts, configurable in the settings dialog
* single instance: a second start activates the running instance
* settings dialog with capture, shortcut, appearance and diagnostics tabs

## Building

Requirements:

* CMake 3.21 or newer
* Qt 6.5 or newer (Core, Gui, Widgets, DBus, Network, Concurrent)
* X11 development files (optional, enables the X11 backend)
* LayerShellQt (optional, enables compositor anchored overlays)

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j"$(nproc)"
ctest --test-dir build --output-on-failure
```

Install with `cmake --install build` (prefix `/usr/local` by default).

### Build options

| Option | Default | Meaning |
|---|---|---|
| `IMAGEN_BUILD_TESTS` | `ON` | build the test suite |
| `IMAGEN_ENABLE_X11` | `ON` | build the X11 capture backend and hotkeys |
| `IMAGEN_ENABLE_WAYLAND` | `ON` | build the XDG portal capture backend |
| `IMAGEN_WARNINGS_AS_ERRORS` | `OFF` | turn compiler warnings into errors |

## Usage

```sh
imagen                       # start in the background (tray icon)
imagen --capture fullscreen  # take a capture right away
imagen --diagnostics         # print what the session supports
imagen --help
```

The configuration lives in `~/.config/imagen/imagen.ini` and can be edited
by hand; every value is documented in `docs/configuration.md`.

## Platform notes

### Wayland

* Screenshots go through the XDG desktop portal
  (`org.freedesktop.portal.Screenshot`), which is the only portable way to
  read the screen.
* Region, window and freehand captures show the selection interface of the
  compositor; `imagen` receives the already cropped image.
* Full screen and monitor captures request the whole virtual desktop and
  crop it client side, using the device pixel ratio of the screen.
* The pointer is not part of the portal image and is composited from the
  cursor theme instead.
* Global shortcuts use the `org.freedesktop.portal.GlobalShortcuts`
  interface, which Plasma 6 implements.

### X11

* The X11 backend talks to the server directly (`XShm`/`XGetImage`) and
  therefore offers every capture mode, including a client side region
  picker and a real pointer in the image.
* Global shortcuts are grabbed with `XGrabKey`.

See [docs/wayland.md](docs/wayland.md) for the details and the known
limitations.

## Testing

```sh
ctest --test-dir build --output-on-failure
```

The suite covers the geometry and DPI calculations, the configuration round
trip, the shortcut parsing and the output writers. The capture backends are
exercised manually because they need a running display server.

## Code style

The sources follow the rules in `.clang-format` (LLVM based, four spaces,
110 columns). Apply them with:

```sh
find src tests -name '*.cpp' -o -name '*.h' | xargs clang-format -i
```

## Licence

MIT, see [LICENSE](LICENSE). The artwork and the sound effect in `assets/`
were created for this project.
