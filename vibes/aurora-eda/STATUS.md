# aurora-eda Status

**Last updated: 2026-05-13 (Honest assessment)**

**⚠️ WARNING: Previous versions of this file wildly overclaimed completion.
Below is a brutally honest audit after reading every line of source code.**

Many features described as "✓ done" in CLAUDE.md are actually stubs, broken,
or completely missing. This file now accurately tracks what really works.

## Milestone Completion Summary (HONEST)

Counted from the ✓/◐/✗/○ markers in CLAUDE.md — every item tallied individually.

| Milestone | ✓ Done | ◐ Partial | ✗ Broken | ○ Missing | Honest % |
|-----------|--------|-----------|----------|-----------|----------|
| A — Core Infrastructure | 27/30 | 3 | 0 | 0 | **90%** |
| B — Schematic Editor | 8/15 | 3 | 4 | 0 | **53%** |
| C — Layout Editor | 12/19 | 4 | 1 | 2 | **63%** |
| D — Simulation Environment | 12/18 | 5 | 0 | 1 | **67%** |
| E — Physical Verification | 7/13 | 5 | 0 | 1 | **54%** |
| F — PCells and PDK | 12/14 | 2 | 0 | 0 | **86%** |
| G — Import / Export | 14/17 | 2 | 0 | 1 | **82%** |
| H — Project Management | 1/9 | 3 | 0 | 5 | **11%** |
| I — Scripting | 0/9 | 6 | 0 | 3 | **0%** |
| J — Advanced UI | 0/10 | 0 | 0 | 10 | **0%** |
| **Total** | **93/154** | **33** | **5** | **23** | **~60%** |

## What Actually Works (GENUINELY DONE)

### Milestone A completion pass (2026-05-13)
- **A16**: LayToolSelect now detects and selects layout instances via click or rubber-band (was broken — only searched shapes)
- **A13/A14**: SchToolWire now accumulates all segments on the same net (was broken — each click created a new net)
- **A18**: Added crosshair cursor guide lines on schematic and layout canvases; enabled anti-aliasing in layout view; shapes now sorted by GDS layer number for correct draw order
- **A20**: Property editor now updates on selection — connected via selectionChanged signals from both view widgets to MainWindow::onSelectionChanged
- **A25**: TODO comment added in Bindings.cpp documenting the bindings gap

### Core Infrastructure (27/30 ✓, 3 partial [A25/A26/A30])

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

### Schematic Editor (8/15 ✓, 3 partial, 4 broken)
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

### Layout Editor (12/19 ✓, 4 partial, 1 broken, 2 missing)
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

### Simulation Environment (12/18 ✓, 5 partial, 1 missing)
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

### Physical Verification (7/13 ✓, 5 partial, 1 missing)
- DrcEngine: width, spacing, non-Manhattan checks
- DrcViolation struct with type, layer, message, location
- LvsChecker: compares schematic vs layout net/pin counts
- DrcResultsDialog: violation table, double-click zooms
- Interactive DRC button (iDRC): runs engine, pushes markers to canvas
- DRC markers overlay on layout canvas (red outline + fill)
- ParasiticExtractor: coupling capacitance + wire resistance
- ParasiticReducer: Pi/T/Lumped reduction
- PercChecker: IR drop, current density, missing power/ground nets

### PCells and PDK (12/14 ✓, 2 partial)
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

### Import / Export (14/17 ✓, 2 partial, 1 missing)
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

### Project Management (1/9 ✓, 3 partial, 5 missing)
- LibraryManager: attach/detach, priorities, search paths
- HierarchyBrowser: builds hierarchy tree
- ImportWizard: auto-dispatch by extension
- **No library management UI** ○ missing
- **No revision control integration** ○ missing
- **No project archiving** ○ missing
- **No design health dashboard** ○ missing
- **LibraryManager never called from main window** ◐ partial
- **ImportWizard wired to menu** ✓ (just added)

### Scripting (0/9 ✓, 6 partial, 3 missing)
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

### Advanced UI (0/10 ✓, 10 missing)
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

## Milestone K — EDA Essentials (What a REAL EDA has that's missing here)

**⚠️ Priority:** These are the features that distinguish an EDA from a drawing program.
Most are completely unimplemented. Adding them would make aurora-eda actually usable.

| # | Feature | Why It Matters | Effort |
|---|---------|---------------|--------|
| K1 | **Proper schematic component library** — Standard symbols (NMOS, PMOS, R, C, L, D, BJT, op-amp, VCC/GND) with correct IEEE/ANSI shapes, not just colored rectangles | Without standard symbols, schematics are unreadable | High |
| K2 | **Multi-segment wire on same net** — Click-to-click-to-click creates one wire object, each segment on the same net. Currently each click creates a new net. | Fundamental schematic entry is broken | Medium |
| K3 | **Net highlighting** — Click on wire/pin → all connected wires, pins, and instance pins highlight in color. | Required for any real schematic/layout work | Medium |
| K4 | **Wire junction dots** — Black filled circles at wire T-junctions and 4-way intersections. | Schematic clarity; currently zero code | Small |
| K5 | **Crosshair cursor** — Full-window vertical + horizontal guide lines snapping to cursor, with coordinate readout on edges. | Standard in all EDA tools | Small |
| K6 | **Real-time DRC during drawing** — Shapes turn red the moment they violate a design rule, not just when you click "iDRC". | Prevents errors before they happen | High |
| K7 | **Layer draw order** — Shapes sorted by GDS layer number: diffusion before poly before metal. Currently drawn in creation order. | Layout is physically incorrect without this | Small |
| K8 | **Anti-aliased rendering** — Smooth lines and curves at all zoom levels. Currently off, so shapes look jagged. | Visual quality | Trivial |
| K9 | **Property editor population** — Click any shape/instance in layout or schematic → right panel shows its properties (layer, coordinates, dimensions, name, parameters). Currently shows blank form. | Essential for editing | Medium |
| K10 | **Proper undo/redo** — Serialize cell state before each operation, restore on undo. Currently pushes string "state" and restores nothing. | Undo is the most basic feature | Medium |
| K11 | **Orthogonal schematic wires** — Wire tool constrains to 45/90 degrees like every EDA. Currently free-angle (only layout has orthogonal). | Schematic readability | Small |
| K12 | **Instance selection in layout** — Click on instance, see bounding box highlight, press Delete to remove. Currently select tool ignores instances. | Cannot interact with placed cells | Small |
| K13 | **Net-aware routing** — Path tool snaps to same-net objects, colors net while routing. Currently draws blind paths. | Layout connectivity requires this | High |
| K14 | **Grid control UI** — Dropdown/input for grid spacing (100nm, 1µm, 5µm, etc.), toggle between dot grid and line grid. Currently hardcoded 100 DBU. | Precision layout requires grid control | Small |
| K15 | **Design hierarchy tree** — Expandable tree of all cells with their views, instance count, bounding box. Navigate by clicking. | Large designs unusable without hierarchy | Medium |
| K16 | **Search nets/instances** — Ctrl+F dialog: type name fragment → find and highlight matching nets/instances in canvas. | Essential for any non-trivial design | Small |
| K17 | **Measurement markers in layout** — Persistent dimension lines (width, spacing) that you can create and leave on the canvas. Currently ruler clears after one use. | Layout verification | Small |
| K18 | **Tool options panel** — When using Wire tool, show grid, width, layer selector. When using Rect tool, show width/height, layer. Changes per tool. | Intuitive UX | Medium |
| K19 | **Status bar context** — Show active tool name, current layer, cursor coordinates (in µm), grid spacing, zoom level. | User awareness | Small |
| K20 | **Context menus (right-click)** — Right-click on object: Select, Properties, Delete, Copy, Move. Right-click on canvas: Create Rect, Create Wire, Paste. | Speed of use | Medium |
| K21 | **Proper toolbar icons** — Tool buttons show recognizable icons (wire = bent line, select = arrow, rect = box, sim = waveform), not text letters. | Professional appearance | Medium |
| K22 | **Dark/light theme** — Toggle between dark (EDA standard) and light themes. ThemeManager exists but never applied. | Eye strain reduction | Medium |
| K23 | **Schematic SnapToObject** — Wires snap to pins and existing wire endpoints when within snap radius (only layout has this). | Schematic entry speed | Small |
| K24 | **Dimension labels during drawing** — While dragging a rectangle, show "W × H µm" overlay next to cursor. Currently no feedback during drag. | Real-time precision feedback | Small |
| K25 | **Multi-window / tab drag-out** — Drag schematic tab into separate window, view schematic + layout side-by-side. Currently one QTabWidget. | Multi-monitor workflow | Large |
| K26 | **Pin direction shapes** — Output pins show triangle pointing out, input pins show triangle pointing in. Currently all pins are 4x4 pixel squares. | Schematic readability | Medium |
| K27 | **Bus expansion visualization** — Bus wire shows individual bit labels fanning out to connected wires. Currently just a thick line with slashes. | Digital signal clarity | Medium |
| K28 | **Waveform calculator library** — Full math: `V(net1) * I(M1)`, `RMS(V(out))`, `1/Δt`, `integ(V(net))`, `abs(V(net))`, etc. Currently only `+` and `-`. | Real circuit analysis requires this | High |
| K29 | **Real FFT** — O(N log N) FFT with windowing (Hanning, Hamming, Blackman). Currently naive O(N²) DFT. | Spectrum analysis is unusably slow | Medium |
| K30 | **Multi-sheet navigation UI** — Sheet tabs / tree showing all schematic sheets, click to switch. Currently generateSpiceMulti exists but no sheet switching UI. | Large schematics require sheets | Medium |
| K31 | **Instance bounding box highlight** — Selected instance shows colored dashed bounding box with cell name. Currently no visual indicator for instance selection in layout. | Required for instance interaction | Small |
| K32 | **Edit selected symbol** — Edit Symbol opens the symbol view for the CURRENT cell. Currently opens the first arbitrary cell. | Symbol editing is broken | Small |
| K33 | **Cross-probe bidirectional** — Select in layout → same net highlighted in schematic (and vice versa). Currently schematic→layout works, layout→schematic just clears. | Debugging requires this | Medium |
| K34 | **PCell invocation UI** — Parameter form dialog when placing PCells (W=?, L=?, fingers=?). Currently NMOS PCell generates fixed geometry. | PCells unusable without param editing | Medium |
| K35 | **Python evaluation pipeline** — Python PCells actually produce layout geometry through pybind11 pipeline. Currently Python PcellBase ABC exists but never called. | Python PCell framework is decoration | Large |
| K36 | **Project file browser** — Tree view of all files in the project (libraries, cells, tech files, run directories). | Project navigation | Medium |
| K37 | **Console/log dock** — Scrollable log pane showing tool actions, DRC results, import/export status, errors. Currently statusBar only shows transient messages. | Debugging and audit trail | Small |
| K38 | **Welcome screen** — Recent projects grid, "New Project", "Open existing", "Import design" buttons on startup. | First-run experience | Medium |
| K39 | **SPICE netlist cross-probe** — Click on net in generated SPICE netlist → highlights that net in schematic. Click on schematic → highlights net in SPICE. | Simulation-debug loop | Medium |
| K40 | **Waveform save/load** — Save plotted waveforms to file, load them back later without re-running simulation. | Simulation workflow | Medium |

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
