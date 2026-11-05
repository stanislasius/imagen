# Configuration

The settings live in `~/.config/imagen/imagen.ini`. The file is written by
the settings dialog and can also be edited by hand.

```ini
[capture]
delay=0
includeCursor=false
playSound=true
showNotification=true
alwaysCopyToClipboard=true
hideEditorBeforeCapture=false

[output]
target=editor
autoSaveDirectory=/home/user/Pictures
autoSavePattern=screenshot_%Y%m%d_%H%M%S
autoSaveFormat=png
autoSaveQuality=90
externalProgram=

[shell]
showTrayIcon=true
launchAtStartup=false
theme=system
language=system

[hotkeys]
capture.fullscreen=Print
capture.region=Ctrl+Print
capture.activewindow=Alt+Print
capture.last=Ctrl+Shift+Print
```

## Keys

| Key | Values | Meaning |
|---|---|---|
| `capture/delay` | seconds, 0 … 60 | delay before the capture, with an on-screen countdown |
| `capture/includeCursor` | bool | draw the mouse cursor into the image |
| `capture/playSound` | bool | play the shutter sound |
| `capture/showNotification` | bool | show a desktop notification |
| `capture/alwaysCopyToClipboard` | bool | copy every capture to the clipboard |
| `capture/hideEditorBeforeCapture` | bool | hide the editor before a capture starts |
| `capture/lastRegion` | rectangle | the region of the last capture |
| `capture/lastKind` | id | the kind of the last capture |
| `output/target` | `editor`, `clipboard`, `file`, `autosave`, `external`, `folder` | where a capture is sent |
| `output/autoSaveDirectory` | path | the folder for automatic saves |
| `output/autoSavePattern` | pattern | see below |
| `output/autoSaveFormat` | `png`, `jpg`, `webp`, `tif`, `bmp` | the format for automatic saves |
| `output/autoSaveQuality` | 1 … 100 | quality for the lossy formats |
| `output/externalProgram` | command | command template, `%f` is replaced with the file name |
| `shell/showTrayIcon` | bool | show the tray icon |
| `shell/launchAtStartup` | bool | start with the session |
| `shell/theme` | `system`, `light`, `dark` | the colour scheme |
| `shell/language` | `system`, `en`, `ru` | the interface language |
| `hotkeys/<action>` | shortcut | the shortcut of an action, empty removes it |

## The auto-save pattern

The pattern uses `strftime` style specifiers:

| Specifier | Meaning |
|---|---|
| `%Y` | year, four digits |
| `%y` | year, two digits |
| `%m` | month |
| `%d` | day of the month |
| `%H` | hour |
| `%M` | minute |
| `%S` | second |
| `%j` | day of the year |
| `%%` | a literal percent sign |

Everything else is copied verbatim, so `shot_%Y%m%d_%H%M%S` produces
`shot_20261106_001449`.

## The shortcut format

Shortcuts are stored in the portable text format of Qt, for example
`Ctrl+Shift+Print`. The key names are the ones of the XKB scheme, so
`Print`, `space`, `Escape`, `F1` … `F35` and `A` … `Z` are valid.

The identifiers of the actions are:

`capture.fullscreen`, `capture.monitor`, `capture.activewindow`,
`capture.window`, `capture.region`, `capture.fixedregion`,
`capture.freehand`, `capture.last`, `editor.open`, `tools.colorpicker`,
`tools.magnifier`, `shell.traymenu`, `shell.settings`.
