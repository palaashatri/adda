# Architecture

`aurora-eda` is organized around a small C++20 core with optional UI and Python
front ends.

## Core

`aurora_core` owns application lifecycle, project management, plugin loading,
geometry primitives, technology metadata, native database objects, and netlist
entry points. Current modules:

- `CoreApp` — initializes/shuts down the application, owns managers.
- `ProjectManager` — create/open/save directory-based projects; includes
  JSON serialization via nlohmann_json.
- `PluginManager` — load shared libraries, call `aurora_register_plugin`.
- `DbCellLib`, `DbCell`, `DbView` — ID-based database: shapes, instances,
  nets, pins, layers, constraints. Each object has a uint64_t ID; 0 = invalid.
- `GeomOps` — grid snapping, rectangle boolean ops (union/intersection/diff),
  Manhattan checks, width/spacing/enclosure checks.
- `TechDatabase` — parses `tech.json` via nlohmann_json; provides layer rules,
  GDS mapping, default widths/spacings.
- `netlist/` — `NetlistGenerator` (SPICE .subckt), `SpiceImporter`, 
  `VerilogGenerator/Importer`, `CdlGenerator`, `DspfGenerator`.
- `sim/` — `SimRunner` (ngspice popen), `SimResult`, parametric sweep,
  Monte Carlo, corner simulation.
- `drc_lvs/` — `DrcEngine` (width/spacing/Manhattan), `LvsChecker`,
  `ParasiticExtractor`, `ParasiticReducer`, `PercChecker`.
- `pdk/` — `PcellDescriptor`/`PcellRegistry`, C++ PCells (`MosPcell`,
  `PcellLibrary`), `Cdf` parameter system, `PcellEvalCache`, `PdkManager`.
- `core/` — `LibraryManager` (multi-library, search paths, cds.lib),
  `DesignServices` (hierarchy browser, archiver, health dashboard),
  `ScriptEngine` (macro recorder, batch runner),
  `WorkspaceServices` (theme, layout, search, hotkeys — declared only).

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
