# Architecture

`aurora-eda` is organized around a small C++20 core with optional UI and Python
front ends.

## Core

`aurora_core` owns application lifecycle, project management, plugin loading,
geometry primitives, technology metadata, native database objects, and netlist
entry points. The current implementation provides the initial skeleton:

- `CoreApp` initializes/shuts down the application and owns managers.
- `ProjectManager` creates/opens/saves directory-based projects.
- `PluginManager` loads shared libraries and calls `aurora_register_plugin`.
- `DbCellLib`, `DbCell`, `DbView`, instances, nets, pins, layers, constraints,
  and shape classes provide the first ID-based database API.
- `GeomOps` provides initial grid snapping, rectangle boolean operations, and
  primitive width/spacing/enclosure checks.
- `TechDatabase` parses structured `tech.json` files through `nlohmann_json`.

## Schematic And Layout

`aurora_schematic` and `aurora_layout` are separate libraries linked against
`aurora_core`. They currently provide minimal document/controller abstractions so
future editor tools can build without changing target boundaries.

## UI

`aurora_ui` is a Qt Widgets target when Qt 6 is installed. If Qt is not found,
CMake still creates an `aurora_ui` stub target, keeping the overall build graph
stable for early development and CI.

`aurora_app` is the runnable application target. It links the Qt shell and emits
an executable named `aurora-eda` when Qt 6 is available. Without Qt 6, it builds a
small stub executable that reports the missing UI dependency.

The initial view widgets support cursor-centered wheel zoom, middle-button
panning, and click selection markers. They still render sample content rather
than live database-backed documents.

## Python

`aurora_python` builds a pybind11 extension when pybind11 is installed. Without
pybind11, CMake creates a stub target. Pure-Python helper modules live under
`python/aurora/`.

## Build Helpers

The repository root includes `build.sh` for macOS/Linux and `build.bat` for
Windows. Both scripts can install common dependencies, configure CMake, build the
project, run tests, and launch the `aurora-eda` executable.

## Plugin Boundary

Plugins are shared libraries that export:

```cpp
extern "C" void aurora_register_plugin(aurora::core::PluginRegistry& registry);
```

OpenAccess/Virtuoso support must remain optional and must not vendor Cadence or
OpenAccess SDK content.
