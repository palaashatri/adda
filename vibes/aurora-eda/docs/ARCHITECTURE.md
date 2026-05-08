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
`aurora_core`. They implement interactive editing through abstract tool hierarchies.

### Schematic tools (`aurora_schematic`)

`SchTool` is the abstract base. A `SchKeyEvent` enum decouples tool key handling
from Qt so `aurora_schematic` has no Qt dependency. Concrete tools:

- `SchToolWire` — draws multi-segment wires; Escape cancels, Enter commits.
- `SchToolSelect` — rubber-band area or point-click selection; Delete removes instances.
- `SchToolInstance` — places cell instances at snap-aligned cursor positions.

`SchEditorController` owns the active tool, dispatches mouse/key events, implements
`snap()` to a configurable grid, and generates unique net/instance names via counters.

### Layout tools (`aurora_layout`)

`LayTool` is the abstract base; `LayEditorController` owns the active tool.
Concrete tools use raw integer Qt key codes (no Qt headers in `aurora_layout`):

- `LayToolRect` — drag-to-draw axis-aligned rectangles with ghost preview.
- `LayToolPolygon` — multi-click polygon; Enter commits (≥3 points), Escape cancels.
- `LayToolSelect` — rubber-band or point selection; Delete/Backspace removes shapes.

### SPICE import (`aurora_core/netlist`)

`SpiceImporter` parses SPICE netlists (`.subckt`/`.ends`, X-element instances,
R/C/L/V/I/M passives) and populates a `DbCellLib`.

### JSON persistence (`aurora_core/core`)

`ProjectManager::saveProject()` serialises the working `DbCellLib` to
`config/design.json` via `nlohmann_json`. `openProject()` reconstructs the full
cell library on load.

## UI

`aurora_ui` is a Qt Widgets target when Qt 6 is installed. If Qt is not found,
CMake still creates an `aurora_ui` stub target, keeping the overall build graph
stable for early development and CI.

`aurora_app` is the runnable application target. It links the Qt shell and emits
an executable named `aurora-eda` when Qt 6 is available. Without Qt 6, it builds a
small stub executable that reports the missing UI dependency.

### View widgets

- `SchematicViewWidget` — full interactive canvas; forwards mouse/key events to
  `SchEditorController`; dynamic_cast overlays for wire ghost, rubber-band, placement preview.
- `LayoutViewWidget` — interactive layout canvas; forwards events to `LayEditorController`;
  ghost overlays for rect, polygon, and selection rubber-band. `zoomToBox()` supports DRC zoom.

### Dialogs and utility widgets

- `WaveformViewWidget` — dark-background custom painter; auto-scaling X/Y axes; multiple
  named traces; wheel zoom and click-drag pan.
- `SimSetupDialog` — ADE-style simulation setup (OP/DC/AC/Transient); simulator path
  browse; emits `simulationFinished(SimResult)`.
- `DrcResultsDialog` — tabbed DRC violation table and LVS result panel; double-click a
  violation row calls `LayoutViewWidget::zoomToBox()`.
- `CellBrowserDialog` — cell library tree with view list; New Cell via `QInputDialog`.

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
