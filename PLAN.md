# PicPick for Linux — Implementation Plan

**Status:** Draft for implementation
**Date:** 2026-11-05
**Source analysis:** Static REA analysis of `picpick_portable/picpick.exe` (PicPick 7.6.2.0, Delphi 12/VCL, PE32) — imports, resources, strings, and `language/English.ini` feature inventory.

---

## 0. Progress

The implementation lives in this repository. The current state is tracked in
the status table of the [README](README.md); the architecture is described
in [docs/architecture.md](docs/architecture.md).

| Date | Milestone | State |
|---|---|---|
| 2026-11-05 | M0 — Scaffolding | done |
| 2026-11-05 | M1 — Capture MVP | in progress (capture backends, outputs, tray, global shortcuts working) |
| 2026-11-05 | M2 — Editor core | started (image window only) |

### Resolved open questions (§11)

| Question | Decision |
|---|---|
| 1. Application name | **imagen**, application id `org.imagen.Imagen` |
| 2. Language | **C++17 with Qt 6** — no Rust toolchain, and the Qt bindings for Widgets/QPainter/QDBus are far weaker than native Qt |
| 3. Distribution priority | **all at M5** (Flatpak, deb, rpm, AppImage) |
| 4. Target DE breadth | **KDE first, GNOME best effort** (as designed in §3 and §6) |
| 5. Minimum Qt version | **Qt 6.5**, developed against 6.8 |
| 6. "Open from URL" | **deferred to M3/M4** |

### Decisions taken during the implementation

| Decision | Reason |
|---|---|
| The `handle_token` of a portal request must only contain `[A-Za-z0-9_]` | it becomes the last element of an object path; a dot or a dash crashes xdg-desktop-portal |
| The shortcut list of the GlobalShortcuts portal is marshalled from `QList<std::pair<QString, QVariantMap>>` | this is the type the portal documents, and it produces the required `a(sa{sv})` |
| The portal connection handle is kept for the lifetime of the client | dropping it closes the bus connection while calls are still in flight |
| The backend is chosen by session type, not by availability | on X11 the direct backend is faster and offers the pointer and a client side region picker |

---

## 1. Background & Goal

PicPick (Windows) is a screenshot capture + image editing suite. No Linux equivalent combines capture, annotation editor, and overlay measuring tools in one app. This project is a **clean-room reimplementation** of the retained feature set for Linux, targeting **Wayland (KDE Plasma) first**, with X11 support.

### 1.1 Legal constraints (hard requirements)

- Do **not** copy code, icons, skins, images, sounds, or translation files from PicPick. The PicPick EULA prohibits reverse engineering; this project is built from *observed functionality and UI behavior*, not decompiled code.
- Do **not** use the name "PicPick" for the distributed application (working name TBD).
- All dependencies must be permissively licensed (MIT/BSD/Apache) or weak-copyleft with dynamic linking (LGPL). No GPL dependencies unless the whole project is intentionally GPL.

---

## 2. Scope

### 2.1 In scope (kept)

| Area | Features |
|---|---|
| **Capture** | Full screen, multi-monitor (all/one), active window, window (picker), rectangular region, fixed region (remembered size), freehand region, repeat last capture |
| **Capture options** | Delay with countdown, include mouse cursor, sound effect on capture, desktop notification, "hide editor before capture", always copy to clipboard |
| **Outputs** | Open in editor, copy to clipboard, save to file, auto-save to folder, launch external program (user-configured, e.g. GIMP/Krita), open containing folder |
| **Editor — file** | New/Open/Recent/Open from clipboard/Open from URL (plain HTTP GET), Save/Save As, Save As formats: PNG, JPEG, BMP, GIF, TIFF, WebP, HEIF, AVIF, JXL, delete file |
| **Editor — edit** | Undo/Redo, Cut/Copy/Paste, Delete selection, Crop to selection, Deselect, Select all, Image Resize, Canvas Resize, view bitmap fullscreen |
| **Editor — annotation** | Layers (raster + vector overlays), shapes: arrow, line, rectangle, ellipse, freehand pen, text, number stamp, highlight, stamps; shape styles/colors/opacity; drag & drop of objects |
| **Editor — effects** | Invert, Grayscale, Pixelate (mosaic), Blur, Sharpen, Brightness/Contrast, Hue/Saturation, Edge effects, Flip H/V, Rotate 90/180/270/custom angle, Frame/Border, Color Balance, Watermark (image/text, position, opacity, auto-apply option) |
| **Editor — image ops** | Resize (with aspect lock, DPI), Canvas resize (anchor, background color/grid) |
| **Tools** | Color picker (eyedropper), color palette with history + copy-as formats (HTML/RGB/C++/Delphi), magnifier (overlay lens), pixel ruler, protractor, crosshair, whiteboard (fullscreen annotate overlay) |
| **Shell** | Tray icon with context menu, global hotkeys (configurable, presets), floating widget (quick actions palette), capture bar (post-capture output chooser), "repeat last capture" |
| **Settings** | INI-based config (`picpick.ini` semantics), light/dark/system theme, language selection |
| **i18n** | Own translations; ship English + Russian initially; community workflow for more |

### 2.2 Out of scope (cut — do not implement)

- ❌ Screen recorder / video editor (FFmpeg, WASAPI, webcam, mouse click effects)
- ❌ Web page capture (WebView2/WebKit)
- ❌ Send to Word/Excel/PowerPoint/Outlook/Skype/email (Office/COM integration)
- ❌ Printing
- ❌ FTP, cloud services, social networks, all upload targets (dedicated uploader)
- ❌ Auto-scroll / scrolling-window stitching capture
- ❌ "Window Control" capture (child element of a window)
- ❌ "Save All as Single PDF" (PDFium)
- ❌ File type associations (defer; revisit only if requested)
- ❌ Update checker, bug reporter, registration/licensing UI

---

## 3. Target environment

| | Primary | Secondary |
|---|---|---|
| Session | **Wayland, KDE Plasma 6** | X11 (any DE) |
| Fallback | GNOME Wayland (degraded: no layer-shell overlays, portal permission prompts) | — |

Known Wayland constraints and how this scope avoids them:

- No child-window capture → **cut** (feature removed).
- No synthetic scrolling → **cut** (feature removed).
- Remaining capture modes are all supported by XDG portals (see §6).
- Screen pixel access (picker/magnifier) → snapshot-based sampling (see §6.4).

---

## 4. Technology stack

| Concern | Choice | Rationale |
|---|---|---|
| Language | **C++17** (Rust+Qt acceptable alternative; decide at M0) | Mature Qt integration, team familiarity assumed |
| UI framework | **Qt 6.5+ (Widgets + minimal QML)** | Native Wayland support, KDE affinity, QPainter for editor, QUndoStack, QSystemTrayIcon → StatusNotifierItem |
| Capture (Wayland) | **xdg-desktop-portal** via `QDBus` (`org.freedesktop.portal.Screenshot`, `ScreenCast`) | Standard, no compositor-specific hacks |
| Capture (X11) | X11 libs: `XShm`, `XComposite`, `XGetImage` | Direct, fast (shared memory) |
| Hotkeys | Abstraction: ① `org.freedesktop.portal.GlobalShortcuts` ② KDE `kglobalaccel` D-Bus ③ X11 `XGrabKey` | KDE support is reliable via kglobalaccel on Wayland |
| Overlays (lens, ruler, widget, whiteboard) | **LayerShellQt** (KDE layer-shell) with fallback to frameless always-on-top windows | KWin implements wlr-layer-shell |
| Notifications | `org.freedesktop.Notifications` (QDBus) | Standard |
| Sound | `QSoundEffect` + own `.wav` assets | Trivial |
| Image I/O | Qt image plugins + **kimageformats** (WebP/HEIF/JXL/AVIF read); write via plugin or `libheif`/`libjwb`/`libwebp` directly | KDE systems already ship kimageformats |
| Image ops | `QImage` + `QPainter`; convolution effects on worker threads | Sufficient for editor-class workloads |
| Settings | `QSettings` (INI format) in `~/.config/<app>/` | Mirrors PicPick's `picpick.ini` portability |
| Build | **CMake** + vcpkg/FetchContent or system deps | Standard |
| Packaging | Flatpak (portal-native permissions) + AppImage/deb | Flatpak is the cleanest Wayland permission story |
| Tests | Qt Test / Catch2 + golden-image tests for effects | See §9 |

---

## 5. Architecture

### 5.1 Directory layout

```
picpick-linux/
├── CMakeLists.txt
├── src/
│   ├── app/                 # main(), single-instance, wiring
│   ├── capture/             # capture subsystem
│   │   ├── CaptureBackend.h         # abstract interface
│   │   ├── PortalCaptureBackend.cpp # Wayland: xdg-desktop-portal
│   │   ├── X11CaptureBackend.cpp    # X11: XShm/XComposite
│   │   └── CaptureHistory.cpp       # "repeat last capture", fixed region
│   ├── hotkeys/
│   │   ├── HotkeyManager.h          # abstract
│   │   ├── PortalHotkeys.cpp        # GlobalShortcuts portal
│   │   ├── KGlobalAccelHotkeys.cpp  # KDE D-Bus
│   │   └── X11Hotkeys.cpp           # XGrabKey
│   ├── editor/
│   │   ├── Document.cpp             # image + layer list + dirty state
│   │   ├── Layer.h                  # raster layer, vector object layer
│   │   ├── objects/                 # Arrow, Rect, Ellipse, Line, Text,
│   │   │                            # NumberStamp, Highlight, PenPath
│   │   ├── tools/                   # ToolBase: Select, Crop, Shape, Text,
│   │   │                            # Pen, Eyedropper, Eraser
│   │   ├── commands/                # QUndoStack commands
│   │   ├── effects/                 # one file per effect (see §7.3)
│   │   └── EditorWidget.cpp         # canvas: zoom/pan/overlay handles
│   ├── overlays/
│   │   ├── OverlayWindow.h          # layer-shell or frameless base
│   │   ├── MagnifierOverlay.cpp
│   │   ├── RulerOverlay.cpp
│   │   ├── ProtractorOverlay.cpp
│   │   ├── CrosshairOverlay.cpp
│   │   ├── WhiteboardOverlay.cpp
│   │   └── ColorPickerSession.cpp   # snapshot-based sampling (§6.4)
│   ├── widget/                      # floating quick-actions palette
│   ├── tray/                        # tray icon + MainPopup menu
│   ├── capturebar/                  # post-capture output bar
│   ├── outputs/                     # Clipboard, File, AutoSave, ExternalProc
│   ├── settings/                    # SettingsDialog + Config (QSettings)
│   └── i18n/                        # .ts files, translators
├── assets/                          # OWN icons, sounds (no PicPick assets)
├── data/                            # .desktop file, appstream metainfo
├── tests/
└── docs/
```

### 5.2 Key abstractions

```cpp
// CaptureBackend — one implementation per session type, selected at runtime
class CaptureBackend {
public:
  virtual ~CaptureBackend() = default;
  virtual bool available() const = 0;
  // One-shot capture; kind selects full/monitor/window/region/freehand
  virtual CaptureRequest startCapture(CaptureKind kind) = 0;
  // Emitted when a frame is ready (possibly async, portal is D-Bus)
  signal void frameReady(const CapturedFrame& frame);
  signal void cancelled();
};

// CapturedFrame = QImage + metadata (source geometry, scale factor, cursor pos)
```

```cpp
// HotkeyManager — resolves configured combos through the best provider
class HotkeyManager {
  // providers tried in order: Portal → KGlobalAccel → X11
  // signal activated(QString actionId)  e.g. "capture.fullscreen"
};
```

**Runtime session detection:** `QGuiApplication::platformName()` → `"wayland"` / `"xcb"`; on Wayland additionally probe portal availability over D-Bus and probe `org.kde.KGlobalAccel`. Show a clear diagnostic dialog if no capture backend works.

---

## 6. Platform integration details

### 6.1 Screenshot capture (Wayland)

- **Full screen / all monitors:** `org.freedesktop.portal.Screenshot` with `interactive=false`. Portal returns the full virtual-desktop image; crop per-monitor client-side using `QScreen::geometry()` × device pixel ratio.
- **Region / window (one-shot):** Screenshot portal with `interactive=true` — compositor UI (KDE picker) lets the user drag a region or pick a window; returned image is the selection. Handle the `handle_token` request/response dance with a dedicated D-Bus connection.
- **Repeat last capture / fixed region:** cannot re-select programmatically on Wayland — capture the full frame via portal and crop with the stored geometry. Note in UX: on Wayland this may show a permission prompt depending on portal implementation (KDE: remembered per app).
- **Cursor inclusion:** portals do not composite the cursor; request `cursor` hint if available (`org.freedesktop.Screenshot` option), otherwise capture without cursor and document the limitation (X11 backend supports real cursor capture).

### 6.2 Window capture (X11)

- `XGetImage` on the window pixmap via `XComposite` (redirected window), `XShm` for full screen.
- Active window: `_NET_ACTIVE_WINDOW` client message; window: click-to-pick with a crosshair grab.
- Multi-monitor: `Xinerama`/`Xrandr` geometry union.

### 6.3 Global hotkeys

| Provider | When | Notes |
|---|---|---|
| `kglobalaccel` D-Bus | KDE (Wayland+X11) | Register component + actions at startup; works on Wayland |
| GlobalShortcuts portal | Other Wayland DEs where supported | Asynchronous accept/activated signals |
| `XGrabKey` | X11 fallback | Standard |

Default bindings mirror PicPick presets (e.g. `PrtSc` fullscreen, `Ctrl+PrtSc` region, `Alt+PrtSc` active window, `Ctrl+Shift+PrtSc` last capture — final list in settings UI, user-editable).

### 6.4 Screen pixel access (color picker / magnifier / crosshair)

- **X11:** direct `XGetImage` at cursor position — live sampling.
- **Wayland:** no live pixel access. Strategy: on picker/magnifier activation, take **one snapshot** via the Screenshot portal; sample pixels from the snapshot while the tool is active; on deactivation discard. Lens/magnifier renders the snapshot region under the cursor (visually identical to live). Crosshair shows coordinates from pointer position (`QCursor::pos()` works on Wayland) without pixels.

### 6.5 Overlays

- Base class `OverlayWindow`: try LayerShellQt (layer `Overlay`, keyboard interactivity `OnDemand` for ruler/protractor); fallback `Qt::Tool | Qt::WindowTransparentForInput | Qt::FramelessWindowHint` always-on-top (works everywhere, slightly less crisp positioning).
- Whiteboard: fullscreen overlay layer with input grab, pen/highlighter/eraser tools, Esc → save/copy dialog.
- Widget: small palette window (lock, opacity, always-on-top, auto-hide) — layer-shell anchored to a screen edge.

### 6.6 Notifications & sound

- `org.freedesktop.Notifications` with app icon; "Copied to clipboard" / "Saved to <path>" actions.
- Shutter sound: bundled WAV (own asset), toggle in settings.

---

## 7. Feature specifications

### 7.1 Capture flow

```
hotkey/menu → [delay countdown overlay] → backend.startCapture(kind)
  → frameReady → (optional cursor composite) → output routing:
       1. copy to clipboard (if "always copy" on)
       2. route to configured Output Type: Editor | File | AutoSave | External | Folder
       3. notification + sound
       4. store in CaptureHistory (for "repeat last")
```

- **AutoSave:** pattern-based filenames (`screenshot_YYYYMMDD_HHMMSS.png`), configurable folder + format + quality.
- **External program:** user registers command templates (e.g. `gimp %f`), PicPick writes temp file and launches.

### 7.2 Editor

- **Canvas:** `EditorWidget` — zoom (fit/100%/wheel+Ctrl), pan (space/middle), checkerboard for transparency, pixel grid at high zoom.
- **Document model:** base raster layer + ordered object layers. Objects keep vector state until "flatten" (auto-flatten option mirrors PicPick `CHK_AUTOFLATTEN`).
- **Selection:** rect + freehand; marching-ants overlay; crop/delete/copy operate on selection raster.
- **Undo:** `QUndoStack`, commands: AddObject, MoveObject, DeleteObject, TransformObject, ApplyEffect (keeps pre-image), Resize, Crop, Fill, TextEdit.
- **Tools palette:** Select, Crop, Pen, Arrow, Line, Rect, Ellipse, Text, Number stamp, Highlight, Mosaic/blur brush, Eyedropper, Eraser.
- **Text:** font/size/color/alignment via Qt font dialog; in-canvas editing with handles.

### 7.3 Effects (all operate on `QImage`, run on `QThreadPool`, preview in dialog where applicable)

| Effect | Implementation notes |
|---|---|
| Invert / Grayscale | Pixel table (fast path: `QImage::invertPixels`, color transform) |
| Pixelate | Block-average downscale + nearest upscale |
| Blur | Box blur ×3 ≈ Gaussian; optional kernel dialog |
| Sharpen | Convolution 3×3 (unsharp mask) |
| Brightness/Contrast | Linear LUT, live preview slider |
| Hue/Saturation | Convert to HSV, shift, convert back |
| Edge effects | Sobel/emboss kernels |
| Flip / Rotate | `QTransform` with high-quality transform |
| Frame | Draw border bands (style, width, color, shadow) |
| Color Balance | Per-channel gain/offset LUT (shadows/midtones/highlights simplified to gain) |
| Watermark | Overlay image or text; corner/center/tiled positions; opacity; "auto-apply on capture" option |

### 7.4 Outputs

- Clipboard: set both `image/png` and `image/bmp` mime targets (Wayland clipboard owned by Qt).
- File dialogs: native Qt dialogs, format list with quality slider for lossy formats.

---

## 8. Milestones

Each milestone ends with a runnable, demoable build.

### M0 — Scaffolding (≈1 week)
- CMake project, Qt 6, CI (build + clang-tidy + tests), logging, single-instance, empty tray icon, settings dialog skeleton, `.desktop` file.
- **Acceptance:** installs and runs on KDE Wayland and X11; `ctest` green.

### M1 — Capture MVP (≈2–3 weeks)
- Portal + X11 backends: fullscreen, monitor, region, window, freehand, fixed region, repeat.
- Delay countdown overlay, cursor option, sound, notification.
- Outputs: clipboard, file, auto-save; tray menu wired; hotkeys (kglobalaccel + X11).
- **Acceptance:** all capture modes work on KDE Wayland **and** X11; hotkeys work on both; repeat/fixed-region work after a portal permission grant.

### M2 — Editor core (≈3–4 weeks)
- Open/save (PNG/JPEG/BMP), zoom/pan, selection, crop, shapes (arrow/line/rect/ellipse/text), pen, undo/redo, copy/paste, open-from-clipboard.
- Route "Output = Editor" end-to-end.
- **Acceptance:** take screenshot → annotate with arrows/text → undo → save PNG.

### M3 — Effects & transforms (≈2–3 weeks)
- Full effects table (§7.3), resize/canvas-resize dialogs, watermark dialog + auto-apply, format expansion (WebP/TIFF/GIF/HEIF/AVIF/JXL via plugins).
- **Acceptance:** every effect has a golden-image test; large (4K) images process < 500 ms for point ops.

### M4 — Tools & overlays (≈3–4 weeks)
- Color picker + palette (snapshot-based on Wayland, live on X11) with copy-as formats; magnifier, ruler, protractor, crosshair, whiteboard; widget; capture bar; external-program output.
- **Acceptance:** each tool usable on KDE Wayland; X11 pass; Esc/cancel paths clean.

### M5 — Polish & release (≈2 weeks)
- i18n (en, ru) with Linguist workflow; light/dark/system themes; settings UI complete; diagnostics dialog for missing portals; Flatpak + AppImage; README, screenshots (own), appstream metadata.
- **Acceptance:** fresh-machine install on KDE Wayland works from Flathub-style sandbox with required portal permissions documented.

**Rough total: 11–16 weeks single developer.**

---

## 9. Testing strategy

1. **Unit tests (Catch2/Qt Test):** geometry (region math, DPI scaling), config round-trip, effect golden images (small fixtures), hotkey parsing.
2. **Backend tests:** X11 backend testable headless via Xvfb; portal backend exercised in manual/semi-auto tests (D-Bus mocking optional at M5).
3. **Manual test matrix (per milestone):** KDE Wayland (primary), KDE X11, GNOME Wayland (smoke), multi-monitor + HiDPI (125/150%) cases.
4. **Regression checklist:** clipboard formats, repeat-capture after permission prompt, tray behavior on Plasma 6, hotkey conflict handling.

---

## 10. Risks & mitigations

| Risk | Impact | Mitigation |
|---|---|---|
| Portal permission prompts interrupt flows (esp. non-KDE) | UX friction | Cache portal handles; KDE primary target (persistent permissions); clear first-run guidance |
| kglobalaccel unavailable outside KDE | Hotkeys dead | Provider chain (§6.3); portal provider; degrade to in-app only + warning |
| layer-shell unavailable (GNOME) | Overlay tools degraded | Frameless always-on-top fallback (§6.5) |
| Wayland has no live pixel access | Picker/magnifier snapshot UX | Documented in §6.4; visually equivalent |
| HiDPI: portal images vs logical coordinates | Misaligned region crops | Always work in device pixels; multiply by `QScreen::devicePixelRatio()` |
| HEIF/JXL **encoding** plugin availability varies | Save formats missing | Bundle `libheif`/`libjxl` in Flatpak; feature-detect at runtime |
| Scope creep back toward cut features | Delay | This document is the contract; cut list in §2.2 is explicit |

---

## 11. Open questions (resolve at M0)

1. **Application name** (must not be "PicPick").
2. Language: C++ or Rust (Rust: `cxx-qt` bindings — viable, slower start)?
3. Distribution priority: Flatpak only, or also deb/rpm/AUR?
4. Target DE breadth: KDE-only support commitment, or best-effort GNOME?
5. Minimum Qt version / distro baseline (recommend Qt 6.5, KDE Frameworks 6 baseline).
6. Do we want an "Open from URL" feature (kept from PicPick scope) in M2 or defer?

---

## 12. Appendix — scope decisions log

| Decision | Date | Rationale |
|---|---|---|
| Cut video recording/editor | 2026-11-05 | User does not use it; removes FFmpeg/WASAPI/MF stack |
| Cut web page capture | 2026-11-05 | WebView2 has no Linux equivalent; browser handles this |
| Cut Office/print/FTP/cloud/social outputs | 2026-11-05 | User does not use them |
| Cut auto-scroll stitching | 2026-11-05 | User does not use it; hardest Wayland feature |
| Cut "Window Control" capture | 2026-11-05 | Impossible on Wayland; user chose full removal over X11-only |
| Cut "Save All as Single PDF" | 2026-11-05 | User chose removal; drops PDFium dependency |
| Keep "External Program" output | 2026-11-05 | Cheap and useful (GIMP/Krita integration) |
