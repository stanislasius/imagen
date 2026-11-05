cmake_minimum_required(VERSION 3.21)

project(imagen
    VERSION 0.1.0
    DESCRIPTION "Screenshot capture and image annotation suite for Linux"
    HOMEPAGE_URL "https://github.com/imagen/imagen"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

include(GNUInstallDirs)

option(IMAGEN_BUILD_TESTS "Build the test suite" ON)
option(IMAGEN_ENABLE_X11 "Enable the X11 capture backend" ON)
option(IMAGEN_ENABLE_WAYLAND "Enable the XDG portal capture backend" ON)
option(IMAGEN_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets DBus Network Concurrent)
find_package(Qt6 6.5 QUIET COMPONENTS Test)

set(CMAKE_AUTOTEST ON)

if(IMAGEN_ENABLE_X11)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(X11 IMPORTED_TARGET x11)
        pkg_check_modules(XEXT IMPORTED_TARGET xext)
    endif()
endif()

# LayerShellQt gives crisp compositor-anchored overlays on wlroots/KWin.
# It is optional: a frameless always-on-top fallback is used when missing.
find_package(LayerShellQt QUIET)

# ---------------------------------------------------------------------------
# Warning flags
# ---------------------------------------------------------------------------

add_library(imagen_flags INTERFACE)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(imagen_flags INTERFACE
        -Wall -Wextra -Wpedantic -Wno-gnu-zero-vars-to-int
    )
    if(IMAGEN_WARNINGS_AS_ERRORS)
        target_compile_options(imagen_flags INTERFACE -Werror)
    endif()
elseif(MSVC)
    target_compile_options(imagen_flags INTERFACE /W4)
endif()

# ---------------------------------------------------------------------------
# Core library
# ---------------------------------------------------------------------------

add_library(imagen_core STATIC
    src/common/AppInfo.cpp
    src/common/AppInfo.h
    src/common/Logging.cpp
    src/common/Logging.h
    src/common/Platform.cpp
    src/common/Platform.h
    src/common/PortalRequest.cpp
    src/common/PortalRequest.h
    src/common/ScreenGeometry.cpp
    src/common/ScreenGeometry.h

    src/settings/Config.cpp
    src/settings/Config.h
    src/settings/SettingsDialog.cpp
    src/settings/SettingsDialog.h

    src/capture/CaptureBackend.cpp
    src/capture/CaptureBackend.h
    src/capture/CaptureHistory.cpp
    src/capture/CaptureHistory.h
    src/capture/CaptureManager.cpp
    src/capture/CaptureManager.h
    src/capture/CaptureTypes.cpp
    src/capture/CaptureTypes.h
    src/capture/CursorCompositor.cpp
    src/capture/CursorCompositor.h
    src/capture/DelayCountdown.cpp
    src/capture/DelayCountdown.h
    src/capture/PortalCaptureBackend.cpp
    src/capture/PortalCaptureBackend.h
    src/capture/X11CaptureBackend.cpp
    src/capture/X11CaptureBackend.h

    src/editor/EditorWindow.cpp
    src/editor/EditorWindow.h

    src/hotkeys/HotkeyAction.cpp
    src/hotkeys/HotkeyAction.h
    src/hotkeys/HotkeyManager.cpp
    src/hotkeys/HotkeyManager.h
    src/hotkeys/HotkeyProvider.h
    src/hotkeys/HotkeySequence.cpp
    src/hotkeys/HotkeySequence.h
    src/hotkeys/KeyNames.cpp
    src/hotkeys/KeyNames.h
    src/hotkeys/PortalShortcutsBackend.cpp
    src/hotkeys/PortalShortcutsBackend.h
    src/hotkeys/X11HotkeysBackend.cpp
    src/hotkeys/X11HotkeysBackend.h

    src/outputs/AutoSaveOutput.cpp
    src/outputs/AutoSaveOutput.h
    src/outputs/ClipboardOutput.cpp
    src/outputs/ClipboardOutput.h
    src/outputs/ExternalProgramOutput.cpp
    src/outputs/ExternalProgramOutput.h
    src/outputs/FileOutput.cpp
    src/outputs/FileOutput.h
    src/outputs/OutputRouter.cpp
    src/outputs/OutputRouter.h

    src/overlays/OverlayWindow.cpp
    src/overlays/OverlayWindow.h
    src/overlays/SelectionOverlay.cpp
    src/overlays/SelectionOverlay.h

    src/system/Notifier.cpp
    src/system/Notifier.h
    src/system/SoundPlayer.cpp
    src/system/SoundPlayer.h

    src/tray/TrayIcon.cpp
    src/tray/TrayIcon.h
)

target_include_directories(imagen_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_definitions(imagen_core PUBLIC
    IMAGEN_VERSION="${PROJECT_VERSION}"
)
target_link_libraries(imagen_core PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    Qt6::Network
    imagen_flags
)

if(IMAGEN_ENABLE_X11 AND X11_FOUND)
    target_link_libraries(imagen_core PUBLIC PkgConfig::X11)
    if(XEXT_FOUND)
        target_link_libraries(imagen_core PUBLIC PkgConfig::XEXT)
    endif()
    target_compile_definitions(imagen_core PUBLIC IMAGEN_HAVE_X11=1)
else()
    target_compile_definitions(imagen_core PUBLIC IMAGEN_HAVE_X11=0)
endif()

if(IMAGEN_ENABLE_WAYLAND)
    target_compile_definitions(imagen_core PUBLIC IMAGEN_HAVE_PORTAL=1)
else()
    target_compile_definitions(imagen_core PUBLIC IMAGEN_HAVE_PORTAL=0)
endif()

if(LayerShellQt_FOUND)
    target_link_libraries(imagen_core PUBLIC LayerShellQtInterface)
    target_compile_definitions(imagen_core PUBLIC IMAGEN_HAVE_LAYER_SHELL=1)
else()
    target_compile_definitions(imagen_core PUBLIC IMAGEN_HAVE_LAYER_SHELL=0)
endif()

# ---------------------------------------------------------------------------
# Executable
# ---------------------------------------------------------------------------

qt_add_executable(imagen src/app/main.cpp src/app/Application.cpp src/app/Application.h)

target_link_libraries(imagen PRIVATE imagen_core)

set_target_properties(imagen PROPERTIES
    WIN32_EXECUTABLE OFF
    MACOSX_BUNDLE OFF
)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

if(IMAGEN_BUILD_TESTS AND TARGET Qt6::Test)
    enable_testing()
    add_subdirectory(tests)
endif()

# ---------------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------------

install(TARGETS imagen RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES data/org.imagen.Imagen.desktop
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
)

install(FILES data/org.imagen.Imagen.metainfo.xml
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo
)

install(FILES assets/icons/org.imagen.Imagen.svg
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps
)

install(FILES assets/sounds/shutter.wav
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/imagen/sounds
)

install(FILES assets/icons/org.imagen.Imagen-tray.svg
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps
)
