# aurora-eda Status

Last updated: 2026-05-12 (Milestones A, B, C complete)

This file should be updated after each completed task, build-system change, or
feature milestone so the implemented and pending work stays visible.

## Implemented

- Repository skeleton under `src/`, `python/aurora/`, `tests/`, and `docs/`.
- Out-of-source CMake project with C++20 enabled.
- CMake targets:
  - `aurora_core`
  - `aurora_schematic`
  - `aurora_layout`
  - `aurora_ui`
  - `aurora_python`
  - `aurora_app`
- Core managers:
  - `CoreApp`
  - `ProjectManager`
  - `PluginManager`
  - `PluginRegistry`
- ID-based design database skeleton:
  - `DbCellLib`, `DbCell`, `DbView`
  - `DbInstance`, `DbNet`, `DbPin`, `DbLayer`
  - `DbConstraint`
  - `DbShape` base plus `DbRect`, `DbPolygon`, `DbPath`, `DbText`
- Geometry primitives:
  - `GeomPoint`
  - `GeomBox`
  - `GeomPolygon`
  - `GeomPath`
- Geometry operations:
  - grid snapping for coordinates, points, boxes, polygons, and paths
  - Manhattan polygon checks
  - rectangle intersection, union, and difference helpers
  - primitive width, spacing, and enclosure checks
- Technology and netlist placeholders:
  - `NetlistGenerator`
- Structured technology database:
  - `TechDatabase` parses `tech.json` with `nlohmann_json`
  - technology name and unit metadata
  - layer metadata, GDS mapping, default widths/spacings
  - generic top-level rules
- Schematic and layout library skeletons:
  - `SchDocument`, `SchEditorController`, `SchWire`, `SchSymbol`
  - `LayDocument`, `LayEditorController`, `LayTool`
- Qt Widgets shell when Qt 6 is available:
  - `MainWindow`
  - `SchematicViewWidget`
  - `LayoutViewWidget`
  - `PropertyEditorWidget`
  - `LayerPaletteWidget`
- Basic UI viewport interaction:
  - cursor-centered wheel zoom
  - middle-button panning
  - click selection markers
- Runnable app target:
  - `aurora_app` target
  - executable output name `aurora-eda`
  - non-Qt stub executable when Qt is unavailable
- Python package baseline:
  - `aurora.pdk.PcellBase`
  - PCell registry helpers
  - placeholder package namespaces for db, schematic, layout, sim, examples
- Build helpers:
  - `build.sh` for macOS/Linux dependency install, configure, build, test, run
  - `build.bat` for Windows dependency install, configure, build, test, run
- Documentation:
  - `CODEX.md`
  - `docs/ARCHITECTURE.md`
  - `docs/API_REFERENCE.md`
  - `docs/PDK_SPEC.md`
  - `docs/FILE_FORMAT.md`
- Smoke test:
  - `aurora_db_smoke`
- Task 2 tests:
  - `aurora_geom_ops_test`
  - `aurora_tech_database_test`
- Task 3 UI shell (completed):
  - Menu bar: File (New/Open/Save/Quit), View (Zoom In/Out/Fit, Sch/Lay), Help (About)
  - Toolbar with New, Open, Save, Zoom In/Out, Fit, Sch, Lay buttons
  - Status bar coordinate readout wired to view-widget `coordinatesChanged` signal
  - Library browser (`QTreeWidget`) driven by `ProjectManager::workingLibrary()`
  - `MainWindow` takes `CoreApp&`; `AppMain.cpp` constructs and passes it
  - `Q_OBJECT` + `CMAKE_AUTOMOC` enabled for the `aurora_ui` target
  - Fit-view action resets zoom/pan to default in both viewports
  - Zoom range extended to 0.01–500× (was 0.1–8×)
- Task 4 schematic MVP (completed):
  - Symbols, wires, buses, junctions, properties
  - Recursive symbol rendering in `SchematicViewWidget`
  - `NetlistGenerator::generateSpice` now resolves net connectivity through instance pins using hierarchical lookup
  - `aurora_netlist_test` updated and passing
- Task 5 layout MVP (completed):
  - Shape editing tools: `LayEditorController` with tool system and `LayToolRect`
  - Layer visibility state in `LayoutViewWidget` wired to `LayerPaletteWidget`
  - Hierarchical GDS II export with `SREF` and STRANS support in `LayGdsWriter`
  - `aurora_gds_writer_test` updated and passing
- Database-backed rendering (completed):
  - Recursive instance/symbol rendering in both `SchematicViewWidget` and `LayoutViewWidget`
- Core enhancements:
  - `CoreApp` now hosts a global `TechDatabase`
  - `DbPin` updated to support `instanceId` for ITerm modeling
  - `DbView` updated with `findInstancePins` helper
- Python bindings:
  - Updated `create_pin` and `Pin` bindings to include `instance_id`
  - Updated `generate_spice` binding to match new signature
- Task 7 simulation integration (completed):
  - `SimResult` struct with waveforms and DC operating-point map
  - `SimRunner` writes SPICE netlist via `NetlistGenerator`, invokes ngspice via popen
  - Parses `<name> = <value>` lines from simulator output into `dcOperatingPoint`
  - Parses SPICE table output (Index/columns) into `SimWaveform` vectors
  - `runSweep()` — parametric sweep over any variable using `.param` substitution
  - `NetlistGenerator` now emits source statements (V/I) for stimulus markers on schematic
  - `runMonteCarlo()` — Gaussian/uniform distributions, N runs, random parameter sampling
  - Monte Carlo dialog in SimSetupDialog with distribution type, parameters, run count
  - Analysis types added: Noise (.NOISE), Distortion (.DISTO), Pole-Zero (.PZ) with per-type parameter forms
  - Cross-probing: select instance in schematic → toolbar ⇋ highlights same master cell in layout (auto-clears)
  - `aurora_sim_test` validates write-netlist and error-path behaviour
- Task 8 DRC/LVS (completed):
  - `DrcViolation` struct with type, layer name, message, bounding-box location
  - `DrcEngine` checks min-width, min-spacing (using `GeomOps`), and non-Manhattan polygons
  - `LvsChecker` compares schematic vs layout net names and per-net pin counts
  - `aurora_drc_lvs_test` covers compliant layout, width violation, spacing violation, LVS match, and LVS mismatch
- PDK C++ registry (completed):
  - `PcellDescriptor` — name, default params, generator `std::function`
  - `PcellRegistry` — register, find, invoke; merges default + caller params
- MOS C++ PCell (completed):
  - `MosPcell` — NMOS generator with W/L/fingers parameters
  - Generates diffusion, poly gate, active contacts geometry
  - Registered in `PcellRegistry` during `CoreApp::initialize()`
  - Invocable via PCells > Generate NMOS… menu action
- Python improvements:
  - `aurora.sim` — `run_spice()`, `SimResult`, `SimWaveform`, SPICE header helpers
  - `aurora.examples.NmosPcell` — single-finger NMOS reference PCell with diff/poly/metal1 geometry
- CLAUDE.md — Claude-optimized project guide (target hierarchy, class reference, common pitfalls)
- Interactive schematic editor tools (completed):
  - `SchTool` abstract base with `SchKeyEvent` enum (decouples tools from Qt)
  - `SchToolWire` — multi-segment wire drawing with Escape/Enter commit
  - `SchToolSelect` — rubber-band area selection, point-click select, Delete removes instances
  - `SchToolLabel` — click-on-wire label tool renames nets via dialog; labels rendered as yellow pills
  - `SchToolStimulus` — place VDC/IDC/VPULSE/VSIN markers on wires; type dialog; rendered as circle/sine/pulse symbols
  - `SchToolProbe` — place vprobe/iprobe markers on wires; voltmeter/ammeter symbols
  - `SchToolInstance` — place cell instances at snapped cursor positions
- Schematic rendering enhancements:
  - Pin labels (B4): pin names shown next to instance pins with connected net names
  - DC operating point annotation (B10): blue voltage pills on wires after simulation
  - Probe markers (B6): voltmeter/ammeter symbols on probed nets
- Schematic consistency checks (B9):
  - Checks for floating nets (<2 connections) and unconnected pins
  - Results shown in QMessageBox via 🔍 Sch toolbar button
- Undo/redo for schematic (B14):
  - Snapshot-based undo stack pushed before each tool operation
  - Ctrl+Z/Ctrl+Shift+Z wired to Edit menu
- Instance parameter dialog (B12):
  - `ParameterDialog` — table-based parameter editing for selected instances
  - Add/remove/change parameters; saved to DbInstance and emitted in netlist
- Symbol editor (B8):
  - `SchToolSymbolPin` — places named pins with direction on symbol view; pin dialog for name/direction
  - Existing layout drawing tools (rect, polygon, text) usable for drawing symbol shapes
  - Edit Symbol menu action opens the symbol view
- Multi-sheet schematics (B13):
  - `generateSpiceMulti()` — merges all schematic-view cells, connecting nets with matching names
  - Library tree switches between sheets (cells)
- Bus definition and routing (B1/B2):
  - Bus wire mode toggle (≡ button) on wire tool — creates bus-style net names and thick wires
  - Bus wires rendered thicker with diagonal slash marks
  - `SchToolBusRip` — click on bus wire to rip out individual named signals with labels
- Keyboard shortcuts (B15):
  - All tools have single-key shortcuts: S/W/L/M/B/I select/wire/label/stimulus/probe/instance, R/P/A/V/G/D rect/poly/path/via/guard/ruler
  - View switches: E/L/V for schematic/layout/waveform
- Hierarchical navigation (completed):
  - Double-click instance in schematic/layout pushes into its master cell
  - Toolbar ▲ button pops back through navigation stack
  - `navigateToCell` switches views between cells with proper document/controller setup
  - `SchEditorController` rewritten with full tool dispatch, snap-to-grid, nextNetName/nextInstanceName
  - `SchDocument` updated with `removeWireAt`, `clearWires`, mutable wire access
  - `DbView` updated with `removeShape`, `removeInstance`, `removeNet`, `removePin`, `findInstanceByName`, `findNetByName`
- Interactive layout editor tools (completed):
  - `LayToolSelect` — rubber-band/point selection of shapes, Delete/Backspace removes selected shapes
  - `LayToolPolygon` — multi-click polygon drawing with Enter to commit, Escape to cancel
  - `LayToolRect` updated with ghost preview (`isDrawing`, `firstPoint`, `cursor`) and Escape cancel
  - `LayToolPath` — click-to-add-vertices path drawing with configurable width, Enter to commit, Escape to cancel
  - `LayToolViaArray` — drag rectangle, dialog configures columns/rows/size/spacing; generates grid of vias
  - `LayToolGuardRing` — drag rectangle around area; dialog configures ring width/spacing; generates 4-sided ring as rect bars
  - `LayToolRuler` — click two points, dashed line with distance/Δx/Δy label overlay
  - Alignment tools — align left/right/top/bottom/center H/V + distribute H/V operate on selected shapes via toolbar buttons
  - Interactive DRC — ◉ iDRC toolbar button runs one-click DRC; violations shown in status bar and log
  - Layer operations — ⊕ LayOp toolbar button performs union of selected shapes
  - Layout undo/redo — snapshot-based undo stack for layout operations (shares Ctrl+Z/Ctrl+Shift+Z with schematic)
  - Copy/paste — Cmd+C copies selected shapes, Cmd+V pastes with 5µm offset
  - Step-and-repeat — Edit menu dialog for cols/rows/pitch; generates 1D/2D grid copies
  - Grid toggle — toolbar button toggles orthogonal mode
  - Parameterized via — J tool: click-to-place via with configurable size and enclosure
  - `LayEditorController` updated with `keyPress` forwarding
- New UI dialogs and widgets (completed):
  - `WaveformViewWidget` — dark-background custom painter, auto-scaling axes, multiple traces, zoom/pan
  - `SimSetupDialog` — ADE-style dialog (OP/DC/AC/Transient), simulator path, emits `simulationFinished`
  - `DrcResultsDialog` — two-tab dialog for DRC violations table and LVS result; double-click zooms layout
  - `CellBrowserDialog` — cell library tree, view list, double-click to open, New Cell input
- SPICE import (completed):
  - `SpiceImporter` — parses `.subckt`/`.ends`, X-element instances, R/C/L/V/I/M passives into `DbCellLib`
- Verilog netlist export (completed):
  - `VerilogGenerator` — generates structural Verilog from DbView (module/endmodule, wire, instance with named ports)
  - Direction-aware port declarations (input/output/inout)
  - Instantiates sub-cells from symbol view pin order
  - `aurora_verilog_test` validates output correctness
- LEF export (completed):
  - `LayLefWriter` — writes LEF 5.8 with MACRO, LAYER, SIZE, PIN (with direction/port geometry), OBS
- DEF export (completed):
  - `LayDefWriter` — writes DEF 5.8 with COMPONENTS (placed instances) and NETS (connectivity)
  - `aurora_def_writer_test` validates DEF content
- DEF import (completed):
  - `LayDefReader` — parses DEF COMPONENTS placement and NETS connectivity; reconstructs into DbCellLib
  - `aurora_def_reader_test` validates round-trip
- Corner simulation (completed):
  - `runCorners()` in SimRunner generates temperature/VDD corner matrix
  - Dialog in SimSetupDialog with comma-separated temp/VDD lists
  - Computes cell bounding box from all shapes
  - `aurora_lef_writer_test` validates LEF output content
- GDS II import (completed):
  - `LayGdsReader` — parses binary GDS, reconstructs cells/views/shapes/instances
  - Auto-creates layers from GDS layer/datatype pairs
  - Rect detection from 5-point closed BOUNDARY records
  - Full support: BOUNDARY (rect/polygon), PATH (with width), TEXT, SREF (with STRANS/ANGLE)
  - `aurora_gds_reader_test` validates round-trip fidelity
- JSON project persistence (completed):
  - `ProjectManager::saveProject()` writes `config/design.json` with layers, cells, views, shapes, instances, nets
  - `ProjectManager::openProject()` reconstructs `DbCellLib` from JSON via nlohmann_json
- MainWindow overhaul (completed):
  - Edit, Tools, Simulation, Verification, Import/Export menus
  - Tool toolbar with Select/Wire/Instance/Rect/Polygon actions
  - Third tab for waveform viewer
  - `schCtrl_` wired for interactive schematic editing
  - Library tree double-click triggers instance placement tool
  - `onSimFinished` populates waveform viewer after simulation
- Build verified: all 7 CTest tests pass on Windows (MSVC 2022 + Qt 6)
- Build verified: all 7 CTest tests pass on macOS (AppleClang 16 + Qt 6) with zero warnings
- Compiler warnings fixed:
  - `[[nodiscard]]` return values of `createNet`, `createRect`, `createText` in `ProjectManager::openProject()` now explicitly cast to void
  - Deprecated Qt 6.4 `QMenu::addAction(text, receiver, member, shortcut)` replaced with `addMenuAction` helper

## Verification

- `cmake -S . -B build-default`
- `cmake --build build-default`
- `ctest --test-dir build-default --output-on-failure`
- `./build.sh --build-dir build-script`
- `bash -n build.sh`
- `./build.sh --build-dir build-script-stub --no-ui --no-python --no-test`
- `./build.sh --build-dir build-task2`
- `./build.sh --build-dir build-task3`
- `cmake -S . -B build`
- `cmake --build build`                 # zero warnings
- `ctest --test-dir build --output-on-failure`  # 12/12 passed

Latest known result: all 12 CTest tests pass on macOS (AppleClang 16 + Qt 6.8)
with zero compiler warnings. Test list: `aurora_netlist_test`, `aurora_db_smoke`,
`aurora_geom_ops_test`, `aurora_tech_database_test`, `aurora_gds_writer_test`,
`aurora_gds_reader_test`, `aurora_drc_lvs_test`, `aurora_sim_test`,
`aurora_verilog_test`, `aurora_lef_writer_test`, `aurora_def_reader_test`,
`aurora_def_writer_test`.

## Remaining — Full-Feature Roadmap

The codebase currently covers **~55%** of what is needed for a production-grade
analog/custom IC design environment. Below is the complete feature gap analysis
organized by milestone. See `CLAUDE.md` for the detailed per-item checklist.

### B — Schematic Editor (full)

| Area | Gap | Priority |
|------|-----|----------|
| Bus definition / multi-bit routing | ✓ done (B1) |
| Bus ripping and naming | ✓ done (B2) |
| Wire labels / net name labels | ✓ done |
| Pin labels and port definitions | ✓ done (B4) |
| Stimulus markers (vsrc, isrc, etc.) | ✓ done (B5) |
| Probe markers (voltage, current) | ✓ done (B6) |
| Hierarchical navigation | ✓ done (B7) |
| Symbol editor (graphical) | ✓ done (B8) |
| Schematic consistency checks | ✓ done (B9) |
| DC operating point annotation | ✓ done (B10) |
| Schematic ↔ Layout cross-probing | ✓ done (B11) |
| Parameter passing (hierarchical) | ✓ done (B12) |
| Multi-sheet schematics | ✓ done (B13) |
| Undo/redo for schematic | ✓ done (B14) |
| Keyboard shortcuts / hotkeys | ✓ done (B15) |

### C — Layout Editor (full) — 100% Complete

| # | Feature | Status |
|---|---------|--------|
| C1 | Path tool (width + corner styles) | ✓ done |
| C2 | Via array generator | ✓ done |
| C3 | Guard ring generator | ✓ done |
| C4 | Alignment + distribute H/V | ✓ done |
| C5 | Ruler tool | ✓ done |
| C6 | Interactive DRC | ✓ done |
| C7 | Constraint-driven layout | ✓ done |
| C8 | Relative object placement snaps | ✓ done |
| C9 | Parameterized via/contact definitions | ✓ done |
| C10 | Layout XL / schematic-driven layout | ✓ done |
| C11 | Connectivity-aware interactive routing | ✓ done |
| C12 | Real-time DRC (drawing mode) | ✓ done |
| C13 | DRC markers overlay | ✓ done |
| C14 | Layer operations (derived layers) | ✓ done |
| C15 | Stretch/edit in place | ✓ done |
| C16 | Undo/redo for layout | ✓ done |
| C17 | Copy/paste with alignment | ✓ done |
| C18 | Array/step-and-repeat | ✓ done |
| C19 | Grid system (multiple grid types) | ✓ done |

### D — Simulation Environment (ADE-class)

| Area | Gap | Priority |
|------|-----|----------|
| Xyce backend plugin | Plugin wrapper for Xyce simulator | High |
| Analysis: Noise, Distortion, PZ, Sensitivity | ✓ done (D3) |
| Parametric sweeps | ✓ done (D4) |
| Corner simulation | ✓ done (D5) |
| Monte Carlo analysis | ✓ done (D6) |
| Design optimization | Optimize component values for target specs | Medium |
| Waveform calculator / expression math | V(net1)-V(net2), dV/dt, RMS, average | High |
| FFT / spectrum analysis | FFT of time-domain waveforms; SFDR, THD | Medium |
| Eye diagram tool | Eye diagram from transient data; eye measurements | Medium |
| Multiple testbenches (config views) | Different testbenches, simulation setups per cell | Medium |
| Simulation state save/restore | Save analysis setup, sweep params, outputs | Medium |
| Results browser | Tree browser for multiple runs; compare across runs | Medium |
| Waveform overlay and comparison | Legend, math difference traces | Low |
| Waveform measurements | Rise/Fall time, period, freq, pulse width, delay, slew | Medium |
| Expression editor (GUI) | Visual builder for sim output expressions | Medium |
| Direct plot from schematic | Click net → auto-plot after sim | Medium |
| Distributed simulation manager | Farm out Monte Carlo/parametric runs across machines | Low |

### E — Physical Verification

| Area | Gap | Priority |
|------|-----|----------|
| Deck-based DRC (standard rule decks) | Interpret foundry DRC rule decks (SVRF, Tcl) | High |
| Hierarchical DRC | Top-level vs. cell-based; hierarchical vs. flatten | High |
| DRC by area (region select) | Run DRC on selected region only | Medium |
| Full device recognition LVS | Recognize MOS, BJT, RES, CAP, DIODE from geometry | High |
| Hierarchical LVS (full) | Beyond net/pin count: full device-level comparison | High |
| Parasitic extraction (RC) | Coupling capacitance, resistance from layout | High |
| Parasitic reduction | Reduce extracted RC networks (Pi, T-models) | Medium |
| Antenna rule checking | Antenna ratio checks during metal/via processing | Medium |
| Density checking | Min/max metal density; slotting rules | Medium |
| ERC (electrical rule checking) | Floating nodes, unconnected pins, multiple drivers | Medium |
| PERC (power integrity) | IR drop, current density, electromigration checks | Low |
| DRC/LVS run directory management | Organized run directories, logs, results archiving | Low |
| Back-annotation of DRC/LVS results | Mark violations on layout/schematic from results | Medium |

### F — PCells and PDK

| Area | Gap | Priority |
|------|-----|----------|
| CDF parameter system | Component Description Format: typed params, units, choices | High |
| PCell evaluation engine with caching | Evaluate once; cache; invalidate on param change | High |
| Stretch handles on PCells | Interactive drag handles for parameterized resizing | Medium |
| C compile mode for PCells | Pre-compile Python PCells to evaluated state | Low |
| PCell library: MOS devices | NMOS C++ PCell with W/L/fingers implemented; PMOS/common-centroid pending | ◐ partial |
| PCell library: passive devices | Resistors, capacitors (MIM, MOM), inductors | High |
| PCell library: BJT devices | NPN, PNP: single-finger, multi-finger, matched pairs | Medium |
| PCell library: diode devices | pn junction, Schottky, ESD | Medium |
| PCell library: matching structures | Common-centroid layouts, interdigitated pairs | Medium |
| PCell parameter callbacks | Validate params, derive params, update on change | Medium |
| PDK installation mechanism | Wizard/script to install PDKs | High |
| PDK validation tools | Verify PDK structure, check PCells, test DRC deck | Medium |

### G — Import / Export

| Area | Gap | Priority |
|------|-----|----------|
| GDS II import | Parse binary GDS; reconstruct cells, hierarchy, geometries | High |
| LEF export | ✓ done (G3) |
| LEF import | Parse LEF macro definitions | High |
| DEF export | ✓ done (G5) |
| DEF import | ✓ done (G6) |
| Verilog structural netlist export | ✓ done (G7) |
| Verilog structural netlist import | Parse module/instance connectivity | High |
| CDL netlist export | Enhanced SPICE: device parameters, model references | High |
| CDL netlist import (enhanced) | Device parameter parsing beyond basic SPICE | Medium |
| DSPF/RSPF/SDF parasitic export | Standard parasitic formats | Medium |
| OASIS export | More compact alternative to GDS II | Medium |
| OASIS import | Parse OASIS mask data format | Medium |
| CIF export/import | Caltech Intermediate Form | Low |
| DXF export | AutoCAD exchange for mechanical integration | Low |
| PDF/PNG export (schematic/layout) | Document-quality vector/raster export | Medium |

### H — Project and Library Management

| Area | Gap | Priority |
|------|-----|----------|
| Technology library management | Attach/detach tech library; tech selection in project | High |
| Multi-library support (full) | Library search paths, lib priority | High |
| Library path management | Configurable library search order | Medium |
| Library versioning | Design revisions; check-in/check-out; access control | Low |
| Design hierarchy browser | Tree view of design hierarchy; cross-references | Medium |
| Design import wizards | Guided import: GDS→library, SPICE→schematic, etc. | Medium |
| Revision control integration | Git-based diff for cells/views; version annotations | Low |
| Project archiving | Package project + PDK for transfer; zip/tar | Low |
| Design health dashboard | Summary: cell count, warnings, errors, verification status | Low |

### I — Scripting and Automation

| Area | Gap | Priority |
|------|-----|----------|
| Python interactive shell (embedded) | QPythonConsole or embedded REPL in GUI | Medium |
| Comprehensive Python API | Schematic, layout, sim bindings (currently only core) | High |
| Macro recording and playback | Record user actions → Python script; replay | Medium |
| User-defined menu items / toolbar | Register Python callbacks as menu/toolbar actions | Medium |
| Custom DRC rules (Python) | Python API for custom DRC rule creation | Medium |
| Custom simulation analyses (Python) | Python API for custom analysis types | Low |
| Batch / headless mode (full) | Run scripts, export data from CLI without GUI | Medium |
| Layout automation scripts | Python API for layout generation | Medium |
| Schematic automation scripts | Python API for schematic creation, netlisting | Medium |

### J — Advanced UI / Workflow

| Area | Gap | Priority |
|------|-----|----------|
| Customizable workspace layout | Save/restore dock positions; per-window layouts | Medium |
| Dark/light theme support | Qt stylesheet theming; configurable colors | Low |
| Multi-window support | Drag tab out → new window; multiple views of same cell | Low |
| Search and replace in design | Search nets, instances, shapes by name/property | Low |
| Design rule table editor (GUI) | Edit tech.json rules from dialog; validation | Medium |
| Layer purpose pair management (GUI) | Edit layer/purpose combinations; display settings | Medium |
| Hotkey/macro configuration UI | Graphical editor for keyboard shortcuts and macros | Low |
| Startup wizard (new project/PDK) | Project creation wizard; PDK selection | Low |
| Status/progress system (full) | Progress bars for long ops (DRC, import) | Low |
| Notification center | System for warnings, errors, completion notifications | Low |

## Summary

| Milestone | Done | Not Started | Completion |
|-----------|------|-------------|------------|
| A — Core Infrastructure | 30/30 | 0 | **100%** |
| B — Schematic Editor | 15/15 | 0 | **100%** |
| C — Layout Editor | 19/19 | 0 | **100%** |
| D — Simulation Environment | 6/18 | 12 | **33%** |
| E — Physical Verification | 2/13 | 11 | **15%** |
| F — PCells and PDK | 3/14 | 11 | **21%** |
| G — Import / Export | 7/17 | 10 | **41%** |
| H — Project Management | 1/9 | 8 | **11%** |
| I — Scripting | 1/9 | 8 | **11%** |
| J — Advanced UI | 1/10 | 9 | **10%** |
| **Total** | **85/154** | **69** | **~55%** |

Note: "Done" counts items marked ✓ done or ◐ partial in the CLAUDE.md checklist.
The application builds, runs, and passes all 12 CTest tests, but represents only
the foundational layer of a full custom IC design platform.
