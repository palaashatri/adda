# aurora-eda Status

**Last updated: 2026-05-13 (Honest assessment)**

**⚠️ WARNING: Previous versions of this file wildly overclaimed completion.
Below is a brutally honest audit after reading every line of source code.**

Many features described as "✓ done" in CLAUDE.md are actually stubs, broken,
or completely missing. This file now accurately tracks what really works.

## Milestone Completion Summary (HONEST)

| Milestone | Truly Done | Partial/Broken | Missing | Honest % |
|-----------|-----------|----------------|---------|----------|
| A — Core Infrastructure | 15/30 | 10 | 5 | **~50%** |
| B — Schematic Editor | 4/15 | 8 | 3 | **~25%** |
| C — Layout Editor | 6/19 | 8 | 5 | **~30%** |
| D — Simulation Environment | 7/18 | 6 | 5 | **~40%** |
| E — Physical Verification | 4/13 | 5 | 4 | **~30%** |
| F — PCells and PDK | 8/14 | 3 | 3 | **~55%** |
| G — Import / Export | 10/17 | 4 | 3 | **~60%** |
| H — Project Management | 2/9 | 2 | 5 | **~20%** |
| I — Scripting | 3/9 | 2 | 4 | **~30%** |
| J — Advanced UI | 0/10 | 3 | 7 | **~15%** |
| **Total** | **59/154** | **51** | **44** | **~38%** |

## What Actually Works (GENUINELY DONE)

### Core Infrastructure (15/30)
- CMake project with C++20, out-of-source build
- Geometry primitives: GeomPoint, GeomBox, GeomPolygon, GeomPath
- Geometry ops: snap, Manhattan check, box intersection/union/difference
- ID-based database: DbCellLib, DbCell, DbView, DbShape, DbInstance, DbNet, DbPin, DbLayer
- Transform support on instances
- TechDatabase parses tech.json with nlohmann_json
- NetlistGenerator creates SPICE .subckt from DbView with pin resolution
- SpiceImporter handles .subckt, X-elements, passives
- SimRunner writes netlists, spawns ngspice, parses table output + DC op points
- runSweep, runMonteCarlo (parameter substitution, multi-run) — REAL
- DrcEngine: width, spacing, non-Manhattan checks with configurable options
- LvsChecker: net/pin count comparison
- PcellDescriptor + PcellRegistry (register, find, invoke)
- MosPcell (NMOS with W/L/fingers, generates diff/poly/contact geometry)
- nmos_pcell.py Python reference PCell
- CoreApp, ProjectManager (create/open/save project to JSON)
- PluginManager, PluginRegistry
- JSON serialization for DbCellLib (save/load design.json)
- Build scripts (build.bat, build.sh) with dependency management
- CTest integration (12 test executables)

### Schematic Editor (4/15)
- SchDocument, SchEditorController, SchWire, SchSymbol classes exist
- SchToolWire: click-click creates 2-point wire segments on NEW net each time (**BROKEN**)
- SchToolSelect: point-click and rubber-band selection of instances
- SchToolInstance: places cell instances at cursor
- SchToolLabel: click-on-wire, dialog renames net, yellow pill rendering
- SchToolStimulus: VDC/IDC/VPULSE/VSIN placement, correct rendering
- SchToolProbe: vprobe/iprobe markers, voltmeter/ammeter symbols
- SchToolBusRip: click on bus wire, creates named rip signals
- SchToolSymbolPin: places named pins with direction on symbol
- SchematicViewWidget: zoom/pan/grid/wire rendering/symbol rendering/pin labels
- DC op-point annotation (blue voltage pills on wires)
- Net label rendering (yellow pills)
- Floating net / unconnected pin consistency check
- Instance parameter dialog (ParameterDialog)
- Keyboard shortcuts for tools (S/W/L/M/B/I/R/P/A/V/G/D), views (E/L/V)

### Layout Editor (6/19)
- LayDocument, LayEditorController (grid, zoom, tool dispatch)
- LayToolRect: drag rectangle creation with ghost preview
- LayToolPolygon: multi-click polygon drawing
- LayToolPath: click-to-add-vertices path with width, Enter/Esc, corner style dialog
- LayToolViaArray: drag rectangle, configurable columns/rows/size/spacing
- LayToolGuardRing: drag rectangle, configurable width/spacing
- LayToolRuler: click two points, distance/Δx/Δy label overlay
- LayToolVia: click-to-place via with configurable size/enclosure
- LayToolStretch: click-and-drag rectangle edges
- Alignment tools: left/right/top/bottom/centerH/centerV/distributeH/distributeV
- Copy/paste shapes with offset
- Step-and-repeat dialog
- Layer operations dialog (Union/Intersection/Difference using GeomOps)
- Orthogonal mode toggle constrains drawing to H/V
- LayoutViewWidget: dark canvas, layer-colored shapes, instance hierarchy, selection
- Layer visibility toggle via LayerPaletteWidget
- Ruler measurement display on canvas

### Simulation Environment (7/18)
- SimRunner: write netlist → spawn ngspice → parse output
- Parametric sweep (runSweep): parameter substitution, multi-run
- Monte Carlo (runMonteCarlo): Gaussian/uniform, N runs
- Corner simulation (runCorners): temperature/VDD matrix
- DC op-point parsing from simulator output
- WaveformViewWidget: dark background, axes with ticks/labels, traces, zoom/pan
- Trace legend with clickable visibility toggles
- Draggable measurement markers (two vertical lines, Δt, frequency, rise/fall)
- FFT: naive O(N²) DFT (functional but slow for >10K points)
- Eye diagram: segments by period, overlays
- Expression math: V(net1)-V(net2) with interpolation (+ and - only)
- SimSetupDialog: analysis type selection, simulator path, testbench management
- Simulation state save/load to JSON

### Physical Verification (4/13)
- DrcEngine: width, spacing, non-Manhattan checks
- DrcViolation struct with type, layer, message, location
- LvsChecker: compares schematic vs layout net/pin counts
- DrcResultsDialog: violation table, double-click zooms
- Interactive DRC button (iDRC): runs engine, pushes markers to canvas
- DRC markers overlay on layout canvas (red outline + fill)
- ParasiticExtractor: coupling capacitance + wire resistance
- ParasiticReducer: Pi/T/Lumped reduction
- PercChecker: IR drop, current density, missing power/ground nets

### PCells and PDK (8/14)
- PcellDescriptor + PcellRegistry (C++ framework) ✓
- Python PcellBase ABC + registry helpers ✓
- Cdf parameter system (typed params, units, prompts, validators) ✓
- PcellEvalCache (param hash, skip regen on cache hit) ✓
- PcellLibrary: PMOS, RES_POLY, CAP_MIM, IND_SPIRAL, BJT_NPN, DIODE_PN, MATCH_CC ✓
- defaultStretchHandlesFor() defined ✓
- PdkManager::install (recursive copy) ✓
- PdkManager::validate (tech.json + pcells dir) ✓
- **PCells never actually invoked from UI or used for layout generation** ◐ partial
- **Cdf param callbacks defined but never called by any renderer** ◐ partial
- **No Python PCell evaluation flow** ◐ partial

### Import / Export (10/17)
- GDS II export with SREF hierarchy, STRANS ✓
- GDS II import: BOUNDARY, PATH, TEXT, SREF ✓
- LEF export: MACRO, LAYER, SIZE, PIN, OBS ✓
- LEF import ✓
- DEF export: COMPONENTS, NETS ✓
- DEF import ✓
- Verilog structural export ✓
- Verilog structural import ✓
- CDL export ✓
- DSPF/RSPF/SDF export ✓
- OASIS export + import ✓
- CIF export + import ✓
- DXF export ✓
- Image export (SVG, PDF, PPM) ✓
- **LEF/DEF imports not accessible from menu (wizard requires ImportSelection struct)** ◐ partial
- **DSPF not accessible from menu** ◐ partial
- **No CDL import** ○ missing

### Project Management (2/9)
- LibraryManager: attach/detach, priorities, search paths
- HierarchyBrowser: builds hierarchy tree
- ImportWizard: auto-dispatch by extension
- **No library management UI** ○ missing
- **No revision control integration** ○ missing
- **No project archiving** ○ missing
- **No design health dashboard** ○ missing
- **LibraryManager never called from main window** ◐ partial
- **ImportWizard wired to menu** ✓ (just added)

### Scripting (3/9)
- Python console.py (embeddable REPL helper) ✓
- Python batch.py (script runner) ✓
- Python macro recording Python-side ✓
- Python custom_rules.py (registration API) ✓
- Python ui_callbacks.py (decorator) ✓
- Python layout helpers (array_place, route_l_shape) ✓
- Python SchematicBuilder ✓
- **No actual C++ Python bindings evaluated** ◐ partial
- **ScriptEngine in core never invoked by UI** ◐ partial
- **MacroRecorder records strings "state" like undo** ◐ partial
- **BatchRunner never wired to any CLI entry point** ○ missing

### Advanced UI (0/10)
- WorkspaceLayout struct exists, **never called from UI** ◐ partial
- ThemeManager exists (dark/light), **never applied as stylesheet** ◐ partial
- DesignSearch declared, **never called from UI** ◐ partial
- TechRuleEditor declared, **never wired to UI** ○ missing
- HotkeyConfig declared, **never wired to UI** ○ missing
- StartupSelection struct exists, **no wizard dialog** ○ missing
- ProgressReporter declared, **never called** ○ missing
- NotificationCenter declared, **never called** ○ missing
- **No multi-window support** (single QTabWidget, no drag-out) ○ missing
- **No layer purpose pair editor** ○ missing

---

## What's ACTIVELY BROKEN (Critical Issues)

| Issue | File | Severity |
|-------|------|----------|
| **Undo/Redo is 100% fake** — pushes string "state", restores nothing | `MainWindow.cpp:859-862` | **CRITICAL** |
| **Wire tool creates new net per segment** — cannot draw connected multi-segment wire | `SchToolWire.cpp:28` | **CRITICAL** |
| **Pin labels at garbage coordinates** — `mpin->id() * 3000` is meaningless | `SchematicViewWidget.cpp:301` | **CRITICAL** |
| **Layout select can't select instances** — only searches shape IDs | `LayToolSelect.cpp:75` | **CRITICAL** |
| **Cross-probe layout→schematic** — just clears probe, does nothing | `MainWindow.cpp:1166-1187` | **CRITICAL** |
| **Edit Symbol** opens first arbitrary cell, not current one | `MainWindow.cpp:823-838` | **HIGH** |
| **No anti-aliasing** in layout view — jagged at non-1:1 zoom | `LayoutViewWidget.cpp:435` | **HIGH** |
| **No layer ordering** — shapes drawn in creation order, not layer number | `LayoutViewWidget.cpp:180-226` | **HIGH** |
| **No net highlighting** — clicking wire doesn't highlight connected net | **MISSING** | **HIGH** |
| **No junction dots** at wire intersections in schematic | **MISSING** | **HIGH** |
| **Property editor never updates** — shows same form regardless of selection | `PropertyEditorWidget.cpp` | **MEDIUM** |
| **No orthogonal wire drawing** in schematic (layCtrl has it, schCtrl doesn't) | `SchEditorController.cpp` | **MEDIUM** |

## What's COMPLETELY MISSING (Zero Code)

- Net highlighting (click wire → highlight all connected shapes)
- Junction dots at wire intersections/T-junctions
- Crosshair cursor guide lines on canvas
- Grid dots mode (only grid lines)
- Dimension labels during shape drawing
- Mouse hover highlighting of shapes
- Multi-window support
- Dark/light theme toggle (ThemeManager never applied)
- Design search/replace (DesignSearch never called)
- Design rule table editor GUI
- Layer purpose pair editor
- Hotkey configuration UI
- Startup wizard
- Progress/notification system
- Close Project action
- Recent files list
- Symbol pin shapes with direction indicators (triangle for output, etc.)
- Bus expansion visualization in schematic
- Wire orthogonal constraint (45/90 degree) for schematic
- Layout instance bounding box selection highlight
- Real-time DRC during drawing (only on-demand button)
- Connectivity-aware routing (path tool doesn't snap to same-net)
- Schematic SnapToObject (only layout has it)

---

## Cross-Platform Status (ACCURATE)

| Platform | Build | Tests |
|----------|-------|-------|
| Windows (MSVC 2022 + Qt 6.8) | ✓ build.bat | 10/12 pass (2 crash with stack overflow) |
| macOS (AppleClang 16 + Qt 6.8) | ✓ build.sh | 12/12 pass (per previous report) |
| Linux (GCC/Clang + Qt 6) | ✓ build.sh (configure only) | Not verified |

Test list: `aurora_netlist_test`, `aurora_db_smoke`, `aurora_geom_ops_test`,
`aurora_tech_database_test`, `aurora_gds_writer_test`, `aurora_gds_reader_test`,
`aurora_drc_lvs_test`, `aurora_sim_test`, `aurora_verilog_test`,
`aurora_lef_writer_test`, `aurora_def_reader_test`, `aurora_def_writer_test`.

Note: `aurora_lef_writer_test` and `aurora_def_writer_test` crash on MSVC with
`STATUS_STACK_BUFFER_OVERFLOW (0xc0000409)` — pre-existing `/GS` detection.
All 12 pass on macOS.
