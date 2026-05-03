# vibed-ui

Cross-platform C++20 UI toolkit and shell prototype built from the project blueprint in MASTER_INSTRUCTIONS.md.

## What this project is

vibed-ui is a modular UI toolkit experiment with:

- Core app lifecycle and event queue
- Platform backends for macOS, Linux (X11), and Windows
- Legacy software renderer and placeholder modern renderer
- Basic UI widgets (View, Label, Button, layouts, themes)
- Minimal compositor/window manager and shell prototype
- Runnable demos for validation

## Repository layout

- src/core: app lifecycle, event queue, timing, logging
- src/platform: per-OS window/event/blit implementations
- src/render: rendering interfaces and implementations
- src/ui: view tree, layout, controls, style primitives
- src/compositor: surfaces, focus/z-order, compositor loop
- src/shell: panel, launcher, shell app loop
- examples: demo applications

## Build requirements

### macOS

- Xcode Command Line Tools
- CMake

### Linux

- CMake
- C++ compiler (g++)
- pkg-config
- X11 development headers (for example libx11-dev)

### Windows

- CMake
- MSVC Build Tools or MinGW g++

## Build and run

### macOS/Linux

```bash
./build.sh --build-type Release
./build.sh --run demo_text_editor
```

Common options:

- --build-type Debug|Release
- --run demo_* target name
- --clean
- --install-deps

### Windows

```bat
build.bat --build-type Release
build.bat --run demo_text_editor
```

Common options:

- --build-type Debug|Release
- --run demo_* target name
- --clean
- --install-deps

## Available demos

- demo_basic: minimal app/window/render path
- demo_controls: basic widget interaction sample
- demo_text_editor: text-entry and button actions (save is stubbed)
- demo_event_inspector: live event counters and event labels
- demo_button_lab: button interaction and simple state toggles
- demo_shell_preview: runs shell/compositor preview path

## Current scope and limitations

- Legacy software renderer is the active rendering path.
- Modern renderer exists as a placeholder with TODO implementation points.
- Some compositor and rendering paths are intentionally skeletal and marked TODO.
- Text input and event paths are wired per platform, but deeper IME/shortcut behavior is not implemented.

See CURRENT_STATUS.md for a detailed done vs pending breakdown.
