# aurora-eda — Project Guide

## Goal

Build `aurora-eda`, an open-source, extensible analog/custom IC design environment
approximating Cadence Virtuoso-style workflows. Supports import of neutral IC formats
(GDS, LEF/DEF, CDL, SPICE, Verilog) without copying Cadence code, SKILL, or OA assets.

## Stack

| Layer | Technology |
|-------|-----------|
| Core engine | C++20 (`aurora_core`) |
| Layout domain | C++20 (`aurora_layout`) |
| Schematic domain | C++20 (`aurora_schematic`) |
| GUI | Qt 6 Widgets (`aurora_ui`) |
| Scripting / PCells | Python 3 + pybind11 (`aurora_python`) |
| Build | CMake 3.22+, out-of-source, vcpkg for dependencies |

External dependencies (all via vcpkg): `nlohmann-json`, `pybind11`, `fmt`, `spdlog`, `qtbase`.

## Legal Constraints

- Do not reimplement SKILL or copy SKILL source code.
- Do not reverse-engineer or reproduce OpenAccess internals.
- Do not ship OA SDK headers or proprietary Cadence assets.
- OA/Virtuoso interop lives in an optional plugin that builds only when the user
  has a legal OA installation.

## Repository Layout

```
aurora-eda/
  CMakeLists.txt            # root; options: AURORA_BUILD_UI, AURORA_BUILD_PYTHON, AURORA_BUILD_TESTS
  src/
    CMakeLists.txt          # creates aurora_core; subdirs contribute via target_sources()
    geom/                   # GeomPoint, GeomBox, GeomPolygon, GeomPath, GeomOps → aurora_core
    db/                     # DbCellLib, DbCell, DbView, DbNet, DbPin, DbShape, … → aurora_core
    tech/                   # TechDatabase (JSON loader via nlohmann_json) → aurora_core
    pdk/                    # PcellDescriptor, PcellRegistry → aurora_core
    netlist/                # NetlistGenerator (SPICE) → aurora_core
    sim/                    # SimRunner, SimResult → aurora_core
    drc_lvs/                # DrcEngine, LvsChecker, DrcViolation → aurora_core
    core/                   # CoreApp, ProjectManager, PluginManager → aurora_core
    schematic/              # SchDocument, SchEditorController, SchWire, SchSymbol → aurora_schematic
    layout/                 # LayDocument, LayEditorController, LayTool*, LayGdsWriter → aurora_layout
    ui/                     # MainWindow, view widgets, palette → aurora_ui  (Qt required)
    python/                 # pybind11 bindings → aurora_python  (pybind11 required)
    plugins/                # optional plugin shared libraries
  python/aurora/            # Python package: pdk, sim, db, schematic, layout, examples
  tests/                    # one test executable per module; all linked via CTest
  docs/                     # ARCHITECTURE.md, API_REFERENCE.md, PDK_SPEC.md, FILE_FORMAT.md
  build.bat                 # Windows build helper (--install-deps, --run, --clean, …)
  build.sh                  # macOS/Linux build helper
```

## CMake Target Hierarchy

```
aurora_core          ← geom + db + tech + pdk + netlist + sim + drc_lvs + core
aurora_schematic     → links aurora_core
aurora_layout        → links aurora_core
aurora_ui            → links aurora_schematic + aurora_layout + Qt6
aurora_python        → links aurora_core (pybind11 module)
aurora_app           → links aurora_ui  (executable: aurora-eda)
tests/*              → link aurora_core or aurora_layout as needed
```

All subdirectories under `src/` that contribute to `aurora_core` use
`target_sources(aurora_core PRIVATE … PUBLIC FILE_SET HEADERS …)`.
Subdirectories creating their own library (`aurora_layout`, etc.) use
`add_library(…)` followed by `target_link_libraries(… PUBLIC aurora_core)`.

## Development Rules

1. **Never create top-level directories** without explicit instruction.
2. All C++ sources live under `src/` in the matching subdirectory.
3. All Python package code lives under `python/aurora/`.
4. Plugins live under `src/plugins/<plugin-name>/`.
5. Keep headers and `.cpp` in sync; add new files to the relevant `CMakeLists.txt`.
6. Update `docs/ARCHITECTURE.md` when adding or changing a module.
7. Update `docs/PDK_SPEC.md` for PCell / PDK changes.
8. Update `docs/FILE_FORMAT.md` for new import/export formats.
9. Update `STATUS.md` after every completed milestone.

## Build Quick-Start (Windows)

```bat
build.bat --install-deps   # one-time: installs CMake, Ninja, VS Build Tools, vcpkg packages
build.bat --run            # configure → build → test → launch aurora-eda
build.bat --debug --run    # Debug build
build.bat --no-ui --no-python  # headless / CI build
```

## Build Quick-Start (macOS / Linux)

```sh
./build.sh --install-deps
./build.sh --run
./build.sh --no-ui --no-python
```

## Testing

```
ctest --test-dir build --output-on-failure
```

Registered CTest targets: `aurora_db_smoke`, `aurora_geom_ops_test`,
`aurora_tech_database_test`, `aurora_netlist_test`, `aurora_gds_writer_test`,
`aurora_drc_lvs_test`, `aurora_sim_test`.

## Task Checklist

| # | Task | Status |
|---|------|--------|
| 1 | Core skeleton (CMake, CoreApp, DB, Geom) | ✓ done |
| 2 | Geometry ops + TechDatabase | ✓ done |
| 3 | Qt UI shell | ✓ done |
| 4 | Schematic MVP | ✓ done |
| 5 | Layout MVP + GDS writer | ✓ done |
| 6 | Python bindings + PCell base + examples | ✓ done |
| 7 | Simulation integration (SimRunner / ngspice) | ✓ done |
| 8 | DRC / LVS engine | ✓ done |
| 9 | OpenAccess import plugin (optional) | pending |

## Key Classes — Quick Reference

### Core (`aurora_core`)

| Class | Header | Purpose |
|-------|--------|---------|
| `CoreApp` | `core/CoreApp.h` | Top-level app: owns ProjectManager, PluginManager, TechDatabase |
| `ProjectManager` | `core/ProjectManager.h` | Create/open/save projects; owns working DbCellLib |
| `PluginManager` | `core/PluginManager.h` | Load/unload plugin shared libraries |
| `DbCellLib` | `db/DbCellLib.h` | Library of cells and layers |
| `DbCell` | `db/DbCell.h` | Cell with named views |
| `DbView` | `db/DbView.h` | Container for shapes, instances, nets, pins, constraints |
| `TechDatabase` | `tech/TechDatabase.h` | Loads `tech.json`; provides layer rules |
| `PcellRegistry` | `pdk/PcellRegistry.h` | Registry for native C++ PCell generators |
| `NetlistGenerator` | `netlist/NetlistGenerator.h` | Produces SPICE `.subckt` from a DbView |
| `SimRunner` | `sim/SimRunner.h` | Writes SPICE file + calls external simulator (ngspice) |
| `DrcEngine` | `drc_lvs/DrcEngine.h` | Runs width/spacing/Manhattan DRC against tech rules |
| `LvsChecker` | `drc_lvs/LvsChecker.h` | Compares schematic vs layout nets and pins |

### Layout (`aurora_layout`)

| Class | Header | Purpose |
|-------|--------|---------|
| `LayDocument` | `layout/LayDocument.h` | Layout view wrapper |
| `LayEditorController` | `layout/LayEditorController.h` | Grid, zoom, active tool, mouse dispatch |
| `LayGdsWriter` | `layout/LayGdsWriter.h` | Binary GDS II file writer with SREF hierarchy |
| `LayToolRect` | `layout/LayToolRect.h` | Rectangle draw tool |

### Schematic (`aurora_schematic`)

| Class | Header | Purpose |
|-------|--------|---------|
| `SchDocument` | `schematic/SchDocument.h` | Schematic view wrapper; owns SchWire list |
| `SchEditorController` | `schematic/SchEditorController.h` | Grid management |
| `SchSymbol` | `schematic/SchSymbol.h` | Symbol with pin IDs |
| `SchWire` | `schematic/SchWire.h` | Wire segment list on a net |

### UI (`aurora_ui` — Qt 6 required)

| Class | Header | Purpose |
|-------|--------|---------|
| `MainWindow` | `ui/MainWindow.h` | Top-level QMainWindow: menus, toolbar, docks, tabs |
| `SchematicViewWidget` | `ui/SchematicViewWidget.h` | Schematic canvas with zoom/pan/grid |
| `LayoutViewWidget` | `ui/LayoutViewWidget.h` | Layout canvas with layer visibility |
| `LayerPaletteWidget` | `ui/LayerPaletteWidget.h` | Layer list with color icons and visibility toggles |
| `PropertyEditorWidget` | `ui/PropertyEditorWidget.h` | Object property form |

## Python Package (`python/aurora/`)

```
aurora/
  __init__.py
  pdk/
    pcell_base.py   # PcellBase ABC
    registry.py     # register_pcell / get_pcell
  sim/
    __init__.py     # run_spice(), SimResult, SimWaveform
  examples/
    nmos_pcell.py   # NmosPcell — single-finger NMOS reference PCell
  db/__init__.py, schematic/__init__.py, layout/__init__.py  # namespace stubs
```

## Common Pitfalls

- `DbId` is `uint64_t`; `kInvalidId = 0`. Always check for 0 before using an ID.
- `DbView` owns its shapes via `unique_ptr`; raw pointers from `findShape()` are
  valid only while the view is alive.
- `TechDatabase::defaultWidthForLayer()` returns 0 when the layer has no tech
  rule — DRC code must guard against this.
- `SimRunner::run()` requires `writeSpiceNetlist()` to be called first.
- Qt `Q_OBJECT` classes must be in the target listed under `CMAKE_AUTOMOC`.
- The `aurora_core` include root is `src/` (set via `target_include_directories`),
  so use `#include "db/DbView.h"`, not `#include "DbView.h"`.
