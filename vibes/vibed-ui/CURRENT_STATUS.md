# CURRENT STATUS

Last updated: 2026-05-09

## Overall summary

All 6 demo applications build and run on Windows 11 (MSVC 2022) with zero warnings and zero errors. Debug and Release builds both succeed. The macOS CMakeCache conflict that blocked Windows builds has been resolved.

## Completed work

### Project structure and build

- Required folder/module structure is in place under src and examples.
- CMake project builds module static libraries and demo executables.
- .gitignore added for build/editor/system artifacts.
- build.sh and build.bat support:
  - build type selection
  - clean builds
  - optional dependency install
  - running any demo_* target via --run
  - invocation from outside project directory (script self-chdir behavior)

### Core and app lifecycle

- core::Application initializes selected platform backend, creates window, sets active platform, and manages shutdown.
- core::Event and EventQueue support cross-platform event flow.
- Event payload includes keyCode plus textCode for text-oriented input handling.
- Application::run() has a proper event loop with 16ms frame pacing (~60fps).

### Platform backends

- macOS backend (Cocoa/CoreGraphics):
  - window creation and event pumping
  - focusable custom content view
  - native menu bar setup (Application and File menus)
  - text character extraction for keyboard events
  - software buffer blit to window content
- Linux backend (X11):
  - window creation
  - mouse and keyboard event translation
  - character extraction with XLookupString
  - buffer blit via XPutImage
- Windows backend (Win32/GDI):
  - window creation and message loop
  - mouse event translation
  - text input routed via WM_CHAR
  - non-text keys (arrows, F-keys, Escape, Delete, Home, End, PgUp/Dn) via WM_KEYDOWN
  - buffer blit via DIB + BitBlt

### Rendering

- SoftwareRenderer implemented for clear/rect/rounded-rect/present pipeline.
- drawRoundedRect uses circle-arc corner clipping (not just a plain rect fallback).
- drawText has a 5x7 bitmap glyph font covering A-Z, 0-9, and common punctuation.
- renderer.present() delegates to active platform blit backend.
- ModernRenderer implemented: currently delegates all calls to SoftwareRenderer.
  Provides a working path; GPU acceleration can be added per-platform later.

### UI, compositor, and shell

- UI primitives and widgets implemented: View, ViewTree, Layout/FlexLayout, Label, Button, Theme, Color, Font.
- ViewTree used as the canonical draw/event root in demos.
- Compositor and WindowManager include:
  - surface registration
  - focus selection on pointer events
  - z-order bring-to-front behavior
- Compositor::run() has 16ms frame pacing (no longer a spin loop).
- Shell preview app is integrated and runnable.

### Demos

All 6 demos build and run on Windows 11:

- demo_basic — label + clickable button that updates text
- demo_controls — multiple buttons, flex layout, live theme toggle
- demo_text_editor — keyboard input, backspace, Save writes to editor_output.txt
- demo_event_inspector — live counters for MouseDown / MouseMove / KeyDown events
- demo_button_lab — 6 styled buttons with click-count tracking and BG toggle
- demo_shell_preview — panel + launcher with compositor focus highlighting

### Windows-specific improvements (2026-05-09)

- Fixed stale macOS CMakeCache.txt that blocked Windows configure step.
- MSVC compile flags changed from /EHs-c- to /EHsc — standard library headers require
  synchronous exception handling to compile without C4530 warnings.
- All demos use /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup to suppress the console
  window (GUI-only; log output visible via OutputDebugStringA / DebugView).
- Logger now also emits to OutputDebugStringA on Windows so messages are visible
  without a console (use Visual Studio output pane or Sysinternals DebugView).

## Pending work

### Rendering quality

- Replace 5x7 bitmap font in SoftwareRenderer with a vector/TrueType glyph rasterizer
  for proper proportional spacing and sub-pixel rendering.
  (Requires a font library — postponed to avoid external dependencies.)
- Improve rounded rectangle anti-aliasing (currently hard-edged pixel fill).

### Compositor

- Implement real pixel compositing in WindowManager::renderAll.
  Blocked on Surface not having a pixel buffer; would require adding buffer storage
  to Surface and a way to write rendered output into it before blitting.
- The compositor is currently a logical window manager only (focus + z-order);
  each demo renders directly to the platform blit path.

### Reliability and quality

- Add automated tests (unit/integration/smoke) for core event queue, platform event
  mapping, and UI interactions.
- Add CI build matrix for macOS/Linux/Windows.
- Validate and harden cross-platform key mapping consistency for non-ASCII and
  special keys.

### Platform

- Implement persistence for demo_text_editor save action on macOS/Linux
  (currently uses fopen_s / fopen conditionally; path tested on Windows only).
- Verify Linux and macOS builds still pass after the MSVC flag changes
  (GCC/Clang flags were not modified).

## Known issues / caveats

- macOS build shows deprecated API warnings around lockFocus/unlockFocus drawing
  path; build still succeeds.
- Some targets may show duplicate static library linker warnings (non-fatal).
- GUI typing behavior can only be fully validated through interactive local runs.
- Log output from Logger is not visible in Release builds launched outside a
  terminal — use Sysinternals DebugView or attach Visual Studio debugger.

## How to build and run (Windows)

```
build.bat                          # Release build
build.bat --build-type Debug       # Debug build
build.bat --run demo_basic         # Build + launch demo_basic
build.bat --run demo_text_editor   # Build + launch text editor
build.bat --clean --run demo_controls  # Clean, build, launch
```

Executables land in:
  build\examples\Release\demo_*.exe   (Release)
  build\examples\Debug\demo_*.exe     (Debug)
