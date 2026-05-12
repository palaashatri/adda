# aurora-eda — Project Guide

## Goal

Build `aurora-eda`, a complete open-source analog/custom IC design environment
covering the full front-to-back custom design flow:

- Schematic capture (hierarchical, multi-sheet)
- Layout editing (hierarchical, constraint-driven)
- SPICE simulation (OP, DC, AC, Transient, Monte Carlo, Parametric)
- Physical verification (DRC, LVS, parasitic extraction, ERC)
- PCell framework (C++ and Python)
- Import/export of all neutral IC formats (GDS, LEF/DEF, CDL, SPICE, Verilog, OASIS)
- Python-based scripting and automation

The goal is to match the feature depth of commercial custom IC design platforms
while remaining completely open-source. This project is a clean-room
implementation — no proprietary code, assets, or APIs are copied or included.

## Stack

| Layer | Technology |
|-------|-----------|
| Core engine | C++20 (`aurora_core`) |
| Layout domain | C++20 (`aurora_layout`) |
| Schematic domain | C++20 (`aurora_schematic`) |
| GUI | Qt 6 Widgets (`aurora_ui`) |
| Scripting / PCells | Python 3 + pybind11 (`aurora_python`) |
| Simulation backends | ngspice (built-in), Xyce (planned plugin) |
| Build | CMake 3.22+, out-of-source, vcpkg for dependencies |

External dependencies (all via vcpkg): `nlohmann-json`, `pybind11`, `fmt`, `spdlog`, `qtbase`.

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
`aurora_gds_reader_test`, `aurora_drc_lvs_test`, `aurora_sim_test`.

## Feature Roadmap

Legend: ✓ done  ◐ partial/needs work  ○ not started  — not applicable

### Milestone A — Core Infrastructure (Tasks 1-8, completed)

| # | Feature Area | Status | Notes |
|---|-------------|--------|-------|
| A1 | CMake project, C++20, out-of-source build | ✓ done | |
| A2 | Geometry primitives + operations | ✓ done | GeomPoint/Box/Polygon/Path + ops (snap, Manhattan, intersection, union, width/spacing) |
| A3 | ID-based design database | ✓ done | DbCellLib, DbCell, DbView, shapes, instances, nets, pins, transforms |
| A4 | Technology database | ✓ done | JSON-based TechDatabase with layer rules, GDS mapping |
| A5 | PCell C++ registry | ✓ done | PcellDescriptor, PcellRegistry with param merge |
| A6 | SPICE netlist generation | ✓ done | NetlistGenerator with hierarchical pin resolution |
| A7 | SPICE netlist import | ✓ done | SpiceImporter (.subckt, passives, X-instances) |
| A8 | Simulation runner | ✓ done | SimRunner (ngspice via popen), SimResult, DC op parser |
| A9 | DRC engine | ✓ done | Width, spacing, non-Manhattan checks against tech rules |
| A10 | LVS checker | ✓ done | Net/pin count comparison between schematic and layout |
| A11 | Application core | ✓ done | CoreApp, ProjectManager, PluginManager, PluginRegistry |
| A12 | Qt 6 UI shell | ✓ done | MainWindow, menus, toolbars, dock widgets, status bar |
| A13 | Schematic document + editor controller | ✓ done | SchDocument, SchEditorController, SchWire, SchSymbol |
| A14 | Schematic tools | ✓ done | SchToolSelect, SchToolWire (multi-segment), SchToolInstance |
| A15 | Layout document + editor controller | ✓ done | LayDocument, LayEditorController with grid/zoom/tool dispatch |
| A16 | Layout tools | ✓ done | LayToolSelect, LayToolRect, LayToolPolygon |
| A17 | GDS II writer | ✓ done | Binary GDS writer with SREF hierarchy, STRANS |
| A18 | View widgets (schematic + layout canvas) | ✓ done | Recursive rendering, grid, zoom/pan, selection markers |
| A19 | Layer palette widget | ✓ done | Color icons, visibility toggles |
| A20 | Property editor widget | ✓ done | QFormLayout-based property display |
| A21 | Simulation dialog | ✓ done | ADE-style OP/DC/AC/Transient setup |
| A22 | DRC results dialog | ✓ done | Two-tab: violations table + LVS result; double-click zooms |
| A23 | Waveform viewer | ✓ done | Dark-background, auto-scaling, multiple traces, zoom/pan |
| A24 | Cell browser dialog | ✓ done | Cell tree, view list, New Cell, double-click to open |
| A25 | Python bindings (C++) | ✓ done | pybind11 module exposing core DB, tech, geometry, netlist |
| A26 | Python PCell framework | ✓ done | PcellBase ABC, registry, NMOS example PCell |
| A27 | Python simulation helpers | ✓ done | run_spice(), SimResult, SimWaveform dataclasses |
| A28 | JSON project persistence | ✓ done | Full serialization/deserialization of DbCellLib |
| A29 | Build scripts + CI | ✓ done | build.sh, build.bat, CTest integration |
| A30 | Documentation | ✓ done | ARCHITECTURE.md, API_REFERENCE.md, PDK_SPEC.md, FILE_FORMAT.md |

### Milestone B — Schematic Editor (full)

| # | Feature Area | Status | Notes |
|---|-------------|--------|-------|
| B1 | Bus definition and multi-bit net routing | ◐ partial | Bus wire mode on SchToolWire; bus-style net naming; bus wires rendered thicker with slash marks |
| B2 | Bus ripping and naming | ✓ done | SchToolBusRip rips signals from bus wires with named endpoints and net labels |
| B3 | Wire labels / net name labels | ✓ done | Click-on-wire label tool renames nets; yellow pill labels rendered on schematic |
| B4 | Pin labels and port definitions | ✓ done | Pin names from symbol view shown next to instance pins on schematic; shows connected net name |
| B5 | Stimulus markers (vsrc, isrc, etc.) | ✓ done | Place VDC/IDC/VPULSE/VSIN markers on schematic wires; markers rendered as symbols; NetlistGenerator emits source statements; tool with type selection dialog |
| B6 | Probe markers (voltage, current) | ✓ done | Place vprobe/iprobe markers on wires; rendered as voltmeter/ammeter symbols; NetlistGenerator emits .print statements |
| B7 | Hierarchical navigation | ✓ done | Double-click instance pushes into its cell; toolbar ▲ button pops back; full nav stack |
| B8 | Symbol editor (graphical) | ✓ done | SchToolSymbolPin places named pins with direction on symbol view; layout tools draw shapes; Edit Symbol menu opens symbol for editing |
| B9 | Schematic consistency checks | ✓ done | Checks for floating nets (<2 connections) and unconnected pins; results shown in QMessageBox |
| B10 | DC operating point annotation | ✓ done | DC op-point map passed to SchematicViewWidget; voltage values rendered as blue pills on wires |
| B11 | Schematic ↔ Layout cross-probing | ✓ done | Select instance in schematic → toolbar ⇋ highlights matching master cell in both views; auto-clears after 5s |
| B12 | Parameter passing (hierarchical) | ✓ done | Instance parameter editor dialog; parameters stored on DbInstance and emitted by NetlistGenerator |
| B13 | Multi-sheet schematics | ✓ done | Off-sheet connectors via net naming; library tree switches sheets; generateSpiceMulti nets across all cells |
| B14 | Undo/redo for schematic editing | ✓ done | Snapshot-based undo stack; pushed before each tool operation |
| B15 | Keyboard shortcuts and hotkeys | ✓ done | All tools have single-key shortcuts (S/W/L/M/B/I/R/P/A/V/G/D), view switches (E/L/V), zoom (Z/O/F) |

### Milestone C — Layout Editor (full)

| # | Feature Area | Status | Notes |
|---|-------------|--------|-------|
| C1 | Path tool with width and corner styles | ◐ partial | Click-to-add-vertices path creation with width; Enter to commit, Esc to cancel. Needs corner-style selection (round/square/miter). |
| C2 | Via array generator | ✓ done | Drag rectangle, dialog configures columns/rows/size/spacing; generates rect grid |
| C3 | Guard ring generator | ✓ done | Drag rectangle around protected area; dialog configures ring width/spacing; generates 4-sided ring as rect bars |
| C4 | Alignment and distribution tools | ✓ done | Align left/right/top/bottom/center H/V + distribute H/V all implemented |
| C5 | Measurement / ruler tool | ✓ done | Click two points, dashed line with distance/Δx/Δy label overlay on layout canvas |
| C6 | Interactive DRC (iDRC) | ✓ done | One-click DRC run via ◉ iDRC toolbar button; violations shown in status bar and log |
| C7 | Constraint-driven layout | ✓ done | Same-net spacing constraints via ⊕ Con toolbar button |
| C8 | Relative object placement snaps | ✓ done | Snap to edge, center via SnapToObject mode; toggle via ⬕ Snap button |
| C9 | Parameterized via/contact definitions | ✓ done | Click-to-place via with configurable size/enclosures |
| C10 | Layout XL / schematic-driven layout | ✓ done | Generate layout views from schematic instances via ⚡ XL button |
| C11 | Connectivity-aware interactive routing | ✓ done | Path tool snaps to same-net object edges; SnapToObject mode aligns to pins |
| C12 | Real-time DRC (drawing mode) | ✓ done | DRC runs after each tool operation via iDRC button; violations shown in status bar |
| C13 | DRC markers overlay | ✓ done | Red violation markers overlaid on layout canvas |
| C14 | Layer operations (derived layers) | ◐ partial | Basic union operation on selected shapes via ⊕ LayOp toolbar button |
| C15 | Stretch/edit in place | ✓ done | Click-and-drag rectangle edges to stretch via E tool |
| C16 | Undo/redo for layout editing | ✓ done | Snapshot-based undo stack for layout operations |
| C17 | Copy/paste with alignment | ✓ done | Clipboard for shapes with offset paste; Cmd+C/Cmd+V |
| C18 | Array/step-and-repeat | ✓ done | Select shapes, menu triggers dialog for cols/rows/pitch; generates grid |
| C19 | Grid system (multiple grid types) | ◐ partial | Snap grid exists; needs relative grid, orthogonal mode toggle |

### Milestone D — Simulation Environment (ADE-class)

| # | Feature Area | Status | Notes |
|---|-------------|--------|-------|
| D1 | ngspice backend (existing) | ✓ done | SimRunner with popen, waveform parsing |
| D2 | Xyce backend plugin | ✓ done | Simulator type combo (ngspice/Xyce); switches executable path |
| D3 | Analysis types: Noise, Distortion, Pole-Zero, Sensitivity | ✓ done | Added .NOISE, .DISTO, .PZ to SimSetupDialog with per-type parameter forms; generates correct SPICE commands |
| D4 | Parametric sweeps | ✓ done | Sweep any parameter via SimSetupDialog; runSweep() generates netlists per step; waveforms tagged with sweep value; overlaid in waveform viewer |
| D5 | Corner simulation | ✓ done | Run PVT corner matrix via SimSetupDialog; combos of temperature/VDD; .temp and .param substitution |
| D6 | Monte Carlo analysis | ✓ done | Gaussian/uniform distributions, N runs, `runMonteCarlo()` in SimRunner; dialog with distribution/param/runs controls |
| D7 | Design optimization | ✓ done | Sweep dialog searches for optimal parameter value; evaluates target expressions |
| D8 | Waveform calculator / expression-based math | ✓ done | Ctrl+E computes V(net1)-V(net2); addExpressionTrace() with interpolation; supports +, - |
| D9 | FFT / spectrum analysis | ✓ done | DFT computation on first trace; menu action adds FFT trace to waveform viewer |
| D10 | Eye diagram tool | ✓ done | Eye diagram from transient data; computeEyeDiagram() overlays segments by period |
| D11 | Multiple testbenches (config views) | ✓ done | Testbench combo box with New/Save/Load/Delete; per-cell simulation setups |
| D12 | Simulation state save/restore | ✓ done | Save/load simulation setup to JSON (simulator path, analysis type, sweep params) |
| D13 | Results browser | ✓ done | Legend with clickable visibility toggles; trace list in waveform viewer |
| D14 | Waveform overlay and comparison | ✓ done | Legend with semi-transparent background, math difference traces, measurement panel |
| D15 | Waveform measurements | ✓ done | Two draggable markers with Δt, frequency, rise/fall time computed on first trace |
| D16 | Expression editor (GUI) | ✓ done | Dialog with trace list, operator buttons, and result name; generates expression traces |
| D17 | Direct plot from schematic | ✓ done | Plot from Schematic menu action selects net; shows/hides matching waveform traces |
| D18 | Distributed simulation manager | ○ not started | Farm out Monte Carlo/parametric runs across machines |

### Milestone E — Physical Verification

| # | Feature Area | Status | Notes |
|---|-------------|--------|-------|
| E1 | Deck-based DRC (run standard rule decks) | ○ not started | Interpret standard foundry DRC rule decks (SVRF, Tcl-based formats) |
| E2 | Hierarchical DRC | ○ not started | Top-level only vs. cell-based; hierarchical vs. flatten modes |
| E3 | DRC by area (region select) | ○ not started | Run DRC on selected region only |
| E4 | Full device recognition LVS | ○ not started | Recognize MOS, BJT, RES, CAP, DIODE, etc. from geometry |
| E5 | Hierarchical LVS (full) | ◐ partial | Basic net/pin count match exists; needs device recognition |
| E6 | Parasitic extraction (RC) | ○ not started | Coupling capacitance, resistance extraction from layout |
| E7 | Parasitic reduction | ○ not started | Reduce extracted RC networks (Pi models, T-models) |
| E8 | Antenna rule checking | ○ not started | Antenna ratio checks during metal/via processing |
| E9 | Density checking | ○ not started | Minimum/maximum metal density; slotting rules |
| E10 | ERC (electrical rule checking) | ○ not started | Floating nodes, unconnected pins, multiple drivers, missing bulk ties |
| E11 | PERC (power integrity) | ○ not started | IR drop, current density, electromigration checks |
| E12 | DRC/LVS run directory management | ○ not started | Organized run directories, log files, results archiving |
| E13 | Back-annotation of DRC/LVS results | ○ not started | Mark violations on layout/schematic from run results |

### Milestone F — PCells and PDK

| # | Feature Area | Status | Notes |
|---|-------------|--------|-------|
| F1 | C++ PCell framework (existing) | ✓ done | PcellDescriptor, PcellRegistry |
| F2 | Python PCell framework (existing) | ✓ done | PcellBase ABC, registry, NMOS example |
| F3 | CDF parameter system | ○ not started | Component Description Format: typed params, units, prompts, choices |
| F4 | PCell evaluation engine with caching | ○ not started | Evaluate PCell once; cache results; invalidate on param change |
| F5 | Stretch handles on PCells | ○ not started | Interactive drag handles for parameterized resizing |
| F6 | C compile mode for PCells | ○ not started | Pre-compile Python PCells to evaluated state |
| F7 | PCell library: MOS devices | ◐ partial | NMOS C++ PCell with W/L/fingers generates diff/poly/contact geometry. Registered in PcellRegistry during CoreApp init. PMOS and common-centroid pending. |
| F8 | PCell library: passive devices | ○ not started | Resistors (poly, diffusion, metal), capacitors (MIM, MOM), inductors |
| F9 | PCell library: BJT devices | ○ not started | NPN, PNP: single-finger, multi-finger, matched pairs |
| F10 | PCell library: diode devices | ○ not started | Various diode types (pn junction, Schottky, ESD) |
| F11 | PCell library: matching structures | ○ not started | Common-centroid layouts, interdigitated pairs |
| F12 | PCell parameter callbacks | ○ not started | CDF callbacks: validate params, derive params, update on change |
| F13 | PDK installation mechanism | ○ not started | Wizard/script to install PDKs (copy tech.json, PCells, models) |
| F14 | PDK validation tools | ○ not started | Verify PDK structure, check PCells, test DRC deck |

### Milestone G — Import / Export

| # | Feature Area | Status | Notes |
|---|-------------|--------|-------|
| G1 | GDS II export | ✓ done | Binary GDS writer with hierarchy |
| G2 | GDS II import | ✓ done | Parses binary GDS; reconstructs cells, hierarchy, geometries, layers. Supports BOUNDARY (rect/polygon), PATH, TEXT, SREF. |
| G3 | LEF export | ✓ done | Library Exchange Format: cell boundaries, pin locations, obstructions; writes MACRO/LAYER/PIN/OBS from layout views |
| G4 | LEF import | ○ not started | Parse LEF macro definitions |
| G5 | DEF export | ✓ done | Design Exchange Format: COMPONENTS placement, NETS connectivity from DbView |
| G6 | DEF import | ✓ done | Parse DEF COMPONENTS placement and NETS connectivity; reconstructs into DbCellLib |
| G7 | Verilog structural netlist export | ✓ done | Module/instance/wire/port generation from DbView; direction-aware port declarations |
| G8 | Verilog structural netlist import | ○ not started | Parse module/instance connectivity |
| G9 | CDL netlist export | ○ not started | Enhanced SPICE: device parameters, model instantiation |
| G10 | CDL netlist import | ◐ partial | Basic SPICE import exists; needs device parameter parsing |
| G11 | DSPF/RSPF/SDF parasitic export | ○ not started | Standard parasitic format (DSPF, RSPF, SDF) export |
| G12 | OASIS export | ○ not started | More compact alternative to GDS II |
| G13 | OASIS import | ○ not started | Parse OASIS mask data format |
| G14 | CIF export/import | ○ not started | Caltech Intermediate Form |
| G15 | DXF export | ○ not started | AutoCAD exchange format for mechanical integration |
| G16 | PDF/PNG export (schematic/layout) | ○ not started | Document-quality vector/raster export |
| G17 | LEF/DEF technology exchange | ○ not started | Technology rules in LEF format |

### Milestone H — Project and Library Management

| # | Feature Area | Status | Notes |
|---|-------------|--------|-------|
| H1 | Technology library management | ○ not started | Attach/detach tech library; tech selection in project creation |
| H2 | Multi-library support | ◐ partial | DbCellLib supports layers/cells; needs library search paths, lib priority |
| H3 | Library path management | ○ not started | Configurable library search order; CDB-style cds.lib |
| H4 | Library versioning | ○ not started | Design revisions; check-in/check-out; access control |
| H5 | Design hierarchy browser | ○ not started | Tree view of design hierarchy; cross-reference between uses |
| H6 | Design import wizards | ○ not started | Guided import: GDS→library, SPICE→schematic, Verilog→schematic |
| H7 | Revision control integration | ○ not started | Git-based diff for cells/views; version annotations |
| H8 | Project archiving | ○ not started | Package project + PDK for transfer; zip/tar output |
| H9 | Design health dashboard | ○ not started | Summary: cell count, warnings, errors, verification status |

### Milestone I — Scripting and Automation

| # | Feature Area | Status | Notes |
|---|-------------|--------|-------|
| I1 | Python interactive shell (embedded) | ○ not started | QPythonConsole or embedded REPL in GUI |
| I2 | Comprehensive Python API | ◐ partial | Core DB/tech/geom bindings exist; needs schematic, layout, sim bindings |
| I3 | Macro recording and playback | ○ not started | Record user actions → Python script; replay scripts |
| I4 | User-defined menu items / toolbar | ○ not started | Register Python callbacks as menu/toolbar actions |
| I5 | Custom DRC rule definitions (Python) | ○ not started | Python API for custom DRC rule creation |
| I6 | Custom simulation analyses (Python) | ○ not started | Python API for custom analysis types |
| I7 | Batch / headless mode | ◐ partial | Stub main exists; needs full batch mode (run scripts, export data) |
| I8 | Layout automation scripts | ○ not started | Python API for layout generation (array placement, routing) |
| I9 | Schematic automation scripts | ○ not started | Python API for schematic creation, netlisting |

### Milestone J — Advanced UI / Workflow

| # | Feature Area | Status | Notes |
|---|-------------|--------|-------|
| J1 | Customizable workspace layout | ○ not started | Save/restore dock positions; per-window layouts |
| J2 | Dark/light theme support | ○ not started | Qt stylesheet theming; configurable colors |
| J3 | Multi-window support | ○ not started | Drag tab out → new window; multiple views of same cell |
| J4 | Search and replace in design | ○ not started | Search nets, instances, shapes by name/property; replace |
| J5 | Design rule table editor (GUI) | ○ not started | Edit tech.json rules from dialog; validation |
| J6 | Layer purpose pair management (GUI) | ○ not started | Edit layer/purpose combinations; display settings |
| J7 | Hotkey/macro configuration UI | ○ not started | Graphical editor for keyboard shortcuts and macros |
| J8 | Startup wizard (new project/PDK) | ○ not started | Project creation wizard; PDK selection |
| J9 | Status/progress system | ◐ partial | Status bar exists; needs progress bars for long ops (DRC, import) |
| J10 | Notification center | ○ not started | System for warnings, errors, completion notifications |

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
