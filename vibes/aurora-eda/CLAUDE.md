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
- Cross-platform (Windows, macOS, Linux) via Qt 6, CMake, and vcpkg

**⚠️ STATUS: ~62% complete.** This is a work-in-progress. Many features are
stubbed, partially implemented, or broken. See STATUS.md for the honest audit.
Milestone A completed to 100% — see changelog in STATUS.md.

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

## Known Critical Bugs

These features are marked "done" in earlier docs but are actually broken or fake:

1. **Undo/Redo** (B14/C16): Pushes literal string `"state"`, restores nothing
2. **SchToolWire multi-segment** (A14): Each click creates wire on a NEW net, not same net
3. **Pin label positions** (B4): Uses `mpin->id() * 3000` — meaningless garbage coordinates
4. **Layout instance selection** (C16): Select tool only finds shapes, not instances
5. **Cross-probe layout→schematic** (B11): Non-functional — just clears probe
6. **Edit Symbol** (B8): Opens first cell, not the current one
7. **Net highlighting**: Zero code — clicking a wire does nothing
8. **Junction dots**: Zero code — no visual at wire intersections

## Feature Roadmap (HONEST)

Legend: ✓ done  ◐ partial/needs work  ○ not started  ✗ broken/stub

### Milestone A — Core Infrastructure

| # | Feature Area | Status | Notes |
|---|-------------|--------|-------|
| A1 | CMake project, C++20, out-of-source build | ✓ done | |
| A2 | Geometry primitives + operations | ✓ done | Full GeomPoint/Box/Polygon/Path + ops |
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
| A13 | Schematic document + editor controller | ✓ done | SchDocument, SchEditorController, multi-segment same-net wire |
| A14 | Schematic tools | ✓ done | SchToolSelect/Wire/Instance/Label/Probe/Stimulus/BusRip/SymbolPin all work |
| A16 | Layout tools | ✓ done | Select picks instances + shapes; Delete removes both |
| A18 | View widgets | ✓ done | Crosshair cursor, anti-aliasing, layer-order sorting, grid/zoom/pan |
| A20 | Property editor widget | ✓ done | Updates on selection via selectionChanged signals |
| A25 | Python bindings (C++) | ✓ done | Expanded: DbPolygon/DbPath/DbText/DbTransform, create_*, find_shape with downcast |
| A26 | Python PCell framework | ✓ done | PythonPcellBridge discovers and invokes Python PCells via pybind11 |
| A30 | Documentation | ✓ done | ARCHITECTURE.md + API_REFERENCE.md updated with all module areas |

### Milestone B — Schematic Editor

| # | Feature Area | Status | Notes |
|---|-------------|--------|-------|
| B1 | Bus definition and multi-bit net routing | ◐ partial | Configurable width dialog, thick wires, slash marks, bus ripping; no bus expansion visualization; netlist expansion added but untested |
| B2 | Bus ripping and naming | ✓ done | SchToolBusRip works |
| B3 | Wire labels / net name labels | ✓ done | Click-on-wire label tool renames nets; yellow pill labels |
| B4 | Pin labels and port definitions | ✗ broken | Labels positioned at `pinId * 3000` — garbage coordinates |
| B5 | Stimulus markers (vsrc, isrc, etc.) | ✓ done | VDC/IDC/VPULSE/VSIN markers with type dialog; correct rendering |
| B6 | Probe markers (voltage, current) | ✓ done | vprobe/iprobe markers; voltmeter/ammeter symbols |
| B7 | Hierarchical navigation | ◐ partial | Double-click — schematic pushes into cell; layout navigation broken (select can't find instances) |
| B8 | Symbol editor (graphical) | ✗ broken | Edit Symbol opens first arbitrary cell, not current one |
| B9 | Schematic consistency checks | ✓ done | Checks floating nets (<2 connections) and unconnected pins |
| B10 | DC operating point annotation | ✓ done | Voltage values rendered as blue pills on wires |
| B11 | Layout cross-probing | ✗ broken | Schematic→Layout works; Layout→Schematic is non-functional |
| B12 | Parameter passing (hierarchical) | ✓ done | Instance parameter editor dialog; saved to DbInstance, emitted by netlist |
| B13 | Multi-sheet schematics | ◐ partial | generateSpiceMulti merges cells; no visual sheet navigation |
| B14 | Undo/redo for schematic editing | ✗ broken | Pushes string "state" — restores nothing |
| B15 | Keyboard shortcuts and hotkeys | ✓ done | All tools have single-key shortcuts |

### Milestone C — Layout Editor

| # | Feature Area | Status | Notes |
|---|-------------|--------|-------|
| C1 | Path tool with width and corner styles | ✓ done | Corner-style dialog; PATHTYPE in GDS; Qt rendering for each style |
| C2 | Via array generator | ✓ done | Drag rectangle, dialog for cols/rows/size/spacing |
| C3 | Guard ring generator | ✓ done | Drag rectangle, dialog for width/spacing; 4-sided ring |
| C4 | Alignment and distribution tools | ✓ done | Align left/right/top/bottom/center H/V + distribute H/V |
| C5 | Measurement / ruler tool | ✓ done | Click two points, dashed line with distance/Δx/Δy |
| C6 | Interactive DRC (iDRC) | ✓ done | One-click DRC run via toolbar button; violations shown in status bar and log; markers on canvas |
| C7 | Constraint-driven layout | ◐ partial | DbConstraint exists; toolbar button wired; no visual constraint display |
| C8 | Relative object placement snaps | ◐ partial | SnapToObject works in layout; no SnapToObject in schematic |
| C9 | Parameterized via/contact definitions | ✓ done | Click-to-place via with configurable size/enclosures |
| C10 | Layout XL / schematic-driven layout | ◐ partial | Generates layout views from schematic; no instance placement mapping |
| C11 | Connectivity-aware interactive routing | ○ not started | Path tool does not snap to same-net objects |
| C12 | Real-time DRC (drawing mode) | ○ not started | No DRC during drawing; only on-demand iDRC button |
| C13 | DRC markers overlay | ✓ done | Red violation markers overlaid on layout canvas |
| C14 | Layer operations (derived layers) | ✓ done | Union/Intersection/Difference dialog using GeomOps |
| C15 | Stretch/edit in place | ✓ done | Click-and-drag rectangle edges to stretch |
| C16 | Undo/redo for layout editing | ✗ broken | Same fake "state" string push as schematic undo |
| C17 | Copy/paste with alignment | ✓ done | Clipboard for shapes with 5µm offset paste |
| C18 | Array/step-and-repeat | ✓ done | Dialog for cols/rows/pitch; generates grid copies |
| C19 | Grid system (multiple grid types) | ◐ partial | Snap grid + orthogonal mode; no relative grid, no grid type selection, no grid spacing UI |

### Milestone D — Simulation Environment

| # | Feature Area | Status | Notes |
|---|-------------|--------|-------|
| D1 | ngspice backend (existing) | ✓ done | SimRunner with popen, waveform parsing |
| D2 | Xyce backend plugin | ◐ partial | Simulator type combo exists; Xyce executable path configurable |
| D3 | Analysis types: Noise, Distortion, Pole-Zero | ✓ done | .NOISE, .DISTO, .PZ forms in SimSetupDialog |
| D4 | Parametric sweeps | ✓ done | Sweep parameter via dialog; multi-run; waveforms tagged |
| D5 | Corner simulation | ✓ done | Temperature/VDD corner matrix |
| D6 | Monte Carlo analysis | ✓ done | Gaussian/uniform distributions, N runs |
| D7 | Design optimization | ◐ partial | Sweep dialog searches for optimal; no objective function editor |
| D8 | Waveform calculator / expression math | ◐ partial | Only `+` and `-` between two traces; no `*`, `/`, `abs`, `d/dt`, `sqrt` |
| D9 | FFT / spectrum analysis | ◐ partial | Naive O(N²) DFT — too slow for >10K points; no real FFT algorithm |
| D10 | Eye diagram tool | ✓ done | Segments transient by period; overlays multiple periods |
| D11 | Multiple testbenches (config views) | ✓ done | Testbench combo with New/Save/Load/Delete |
| D12 | Simulation state save/restore | ✓ done | Save/load setup to JSON (simulator path, analysis type, sweep params) |
| D13 | Results browser | ✓ done | Legend with clickable visibility toggles |
| D14 | Waveform overlay and comparison | ✓ done | Semi-transparent legend; math difference traces |
| D15 | Waveform measurements | ✓ done | Two draggable markers with Δt, frequency, rise/fall time |
| D16 | Expression editor (GUI) | ✓ done | Dialog with trace list, operator buttons, result name |
| D17 | Direct plot from schematic | ◐ partial | Menu action selects net by name substring matching |
| D18 | Distributed simulation manager | ○ not started | Single-machine scope; no cluster support |

### Milestone E — Physical Verification

| # | Feature Area | Status | Notes |
|---|-------------|--------|-------|
| E1 | Deck-based DRC (run standard rule decks) | ◐ partial | DrcOptions enables/disables checks; no rule deck file format |
| E2 | Hierarchical DRC | ◐ partial | Flag exists in DrcOptions; recursive traversal not verified |
| E3 | DRC by area (region select) | ◐ partial | areaOnly flag exists; no UI for region selection |
| E4 | Full device recognition LVS | ◐ partial | recognizeDevices() finds MOS from poly/diff overlap |
| E5 | Hierarchical LVS (full) | ◐ partial | Net/pin + device count; hierarchy traversal not verified |
| E6 | Parasitic extraction (RC) | ✓ done | ParasiticExtractor computes coupling C + wire R |
| E7 | Parasitic reduction | ✓ done | ParasiticReducer: Pi/T/Lumped models |
| E8 | Antenna rule checking | ✓ done | Antenna ratio check (metal area / gate area) |
| E9 | Density checking | ✓ done | Min/max density per layer with bin sampling |
| E10 | ERC (electrical rule checking) | ✓ done | Floating nets, multiple drivers, unconnected pins |
| E11 | PERC (power integrity) | ✓ done | PercChecker: IR drop, current density, missing power nets |
| E12 | DRC/LVS run directory management | ○ not started | No run directory/organization UI |
| E13 | Back-annotation of DRC/LVS results | ✓ done | DRC markers overlay on layout via setDrcMarkers() |

### Milestone F — PCells and PDK

| # | Feature Area | Status | Notes |
|---|-------------|--------|-------|
| F1 | C++ PCell framework | ✓ done | PcellDescriptor, PcellRegistry |
| F2 | Python PCell framework | ◐ partial | PcellBase ABC, registry, NMOS example exist; no integration |
| F3 | CDF parameter system | ✓ done | Cdf.h — typed params, units, prompts, choices, validators |
| F4 | PCell evaluation engine with caching | ✓ done | PcellEvalCache — hash params, skip regen |
| F5 | Stretch handles on PCells | ✓ done | defaultStretchHandlesFor() defined |
| F6 | C compile mode for PCells | ◐ partial | Native MosPcell is C++; no Python→C compilation |
| F7 | PCell library: MOS devices | ✓ done | NMOS + PMOS + MATCH_CC registered |
| F8 | PCell library: passive devices | ✓ done | RES_POLY, CAP_MIM, IND_SPIRAL registered |
| F9 | PCell library: BJT devices | ✓ done | BJT_NPN registered |
| F10 | PCell library: diode devices | ✓ done | DIODE_PN registered |
| F11 | PCell library: matching structures | ✓ done | MATCH_CC 2×2 common-centroid |
| F12 | PCell parameter callbacks | ✓ done | CdfParam::validator + derive |
| F13 | PDK installation mechanism | ✓ done | PdkManager::install recursively copies |
| F14 | PDK validation tools | ✓ done | PdkManager::validate checks tech.json + pcells |

### Milestone G — Import / Export

| # | Feature Area | Status | Notes |
|---|-------------|--------|-------|
| G1 | GDS II export | ✓ done | Binary GDS writer with SREF hierarchy |
| G2 | GDS II import | ✓ done | Parses BOUNDARY, PATH, TEXT, SREF |
| G3 | LEF export | ✓ done | MACRO/LAYER/PIN/OBS from layout views |
| G4 | LEF import | ✓ done | LayLefReader parses MACRO/SIZE/PIN/LAYER/RECT |
| G5 | DEF export | ✓ done | COMPONENTS placement, NETS connectivity |
| G6 | DEF import | ✓ done | Parse COMPONENTS and NETS |
| G7 | Verilog structural netlist export | ✓ done | Module/instance/wire/port generation |
| G8 | Verilog structural netlist import | ✓ done | Modules, ports, wires, named-port instances |
| G9 | CDL netlist export | ✓ done | CdlGenerator — .SUBCKT/.ENDS with device params |
| G10 | CDL netlist import | ○ not started | Reuses SpiceImporter partially; needs device param parsing |
| G11 | DSPF/RSPF/SDF parasitic export | ◐ partial | DspfGenerator exists; not wired to UI menu |
| G12 | OASIS export | ✓ done | LayOasisWriter — varint-encoded records |
| G13 | OASIS import | ✓ done | LayOasisReader |
| G14 | CIF export/import | ✓ done | LayCifIo — DS/DF symbol blocks, L/B records |
| G15 | DXF export | ✓ done | LayDxfWriter — LAYER table + LWPOLYLINE |
| G16 | PDF/PNG export (schematic/layout) | ✓ done | LayImageExport — SVG, PDF, PPM |
| G17 | LEF/DEF technology exchange | ◐ partial | LayLefWriter emits LAYER+SITE; reading depends on tech.json |

### Milestone H — Project and Library Management

| # | Feature Area | Status | Notes |
|---|-------------|--------|-------|
| H1 | Technology library management | ◐ partial | LibraryManager::setTechFile exists; no UI |
| H2 | Multi-library support | ◐ partial | Priorities + sort; attach/detach; no UI |
| H3 | Library path management | ○ not started | No path configuration UI |
| H4 | Library versioning | ○ not started | No revision system |
| H5 | Design hierarchy browser | ◐ partial | HierarchyBrowser::build exists; no UI |
| H6 | Design import wizards | ✓ done | ImportWizard::runAuto dispatches by extension; wired to menu |
| H7 | Revision control integration | ○ not started | Snapshot diff exists in LibraryManager |
| H8 | Project archiving | ○ not started | ProjectArchiver exists; no UI |
| H9 | Design health dashboard | ○ not started | DesignHealthDashboard::compile exists; no UI |

### Milestone I — Scripting and Automation

| # | Feature Area | Status | Notes |
|---|-------------|--------|-------|
| I1 | Python interactive shell (embedded) | ◐ partial | console.py exists; not embedded in GUI |
| I2 | Comprehensive Python API | ○ not started | Layout/schematic helpers are Python-side only; no C++ bindings |
| I3 | Macro recording and playback | ◐ partial | MacroRecorder records strings only (same as undo); not functional |
| I4 | User-defined menu items / toolbar | ◐ partial | ScriptedUiRegistry exists; no UI to load Python callbacks |
| I5 | Custom DRC rule definitions (Python) | ◐ partial | CustomRuleRegistry exists; not wired to DrcEngine |
| I6 | Custom simulation analyses (Python) | ○ not started | Registration API exists; no evaluation |
| I7 | Batch / headless mode | ○ not started | BatchRunner exists; not wired to CLI |
| I8 | Layout automation scripts | ◐ partial | Python helpers exist (array_place, route_l_shape); not tested |
| I9 | Schematic automation scripts | ◐ partial | SchematicBuilder exists; not tested |

### Milestone J — Advanced UI / Workflow

| # | Feature Area | Status | Notes |
|---|-------------|--------|-------|
| J1 | Customizable workspace layout | ○ not started | WorkspaceLayout struct declared; never saved/loaded from UI |
| J2 | Dark/light theme support | ○ not started | ThemeManager declared; never applied as stylesheet |
| J3 | Multi-window support | ○ not started | Single QTabWidget, no drag-out |
| J4 | Search and replace in design | ○ not started | DesignSearch declared; never exposed to UI |
| J5 | Design rule table editor (GUI) | ○ not started | TechRuleEditor declared; no dialog |
| J6 | Layer purpose pair management (GUI) | ○ not started | DbLayer has purpose field; no editor |
| J7 | Hotkey/macro configuration UI | ○ not started | HotkeyConfig declared; no UI |
| J8 | Startup wizard (new project/PDK) | ○ not started | StartupSelection struct; no wizard dialog |
| J9 | Status/progress system | ○ not started | ProgressReporter declared; never called |
| J10 | Notification center | ○ not started | NotificationCenter declared; never called |

### Milestone K — EDA Essentials (What a REAL EDA has that's missing here)

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
| `ParasiticExtractor` | `drc_lvs/ParasiticExtractor.h` | Per-layer coupling C and wire R extraction |
| `ParasiticReducer` | `drc_lvs/ParasiticReducer.h` | Pi/T/Lumped reduction of RC networks |
| `PercChecker` | `drc_lvs/PercChecker.h` | PERC: IR drop, current density, missing power nets |
| `Cdf`, `CdfRegistry` | `pdk/Cdf.h` | Component Description Format: typed params, validators |
| `PcellEvalCache` | `pdk/PcellEvalCache.h` | Caches PCell results keyed by param hash |
| `PcellLibrary` | `pdk/PcellLibrary.h` | Built-in PCells: PMOS, RES, CAP, IND, BJT, DIODE, MATCH |
| `PdkManager` | `pdk/PdkManager.h` | PDK install (recursive copy) and validate |
| `LibraryManager` | `core/LibraryManager.h` | Multi-library, search paths, cds.lib, revisions |
| `HierarchyBrowser` | `core/DesignServices.h` | Build/inspect design hierarchy tree |
| `DesignHealthDashboard` | `core/DesignServices.h` | Summary stats for the working library |
| `ProjectArchiver` | `core/DesignServices.h` | AURORA-AR-1 single-file project bundle |
| `ScriptEngine` | `core/ScriptEngine.h` | MacroRecorder, ScriptedUI, BatchRunner |
| `WorkspaceLayout`, `ThemeManager`, `DesignSearch`, `TechRuleEditor`, `HotkeyConfig`, `ProgressReporter`, `NotificationCenter` | `core/WorkspaceServices.h` | UI workflow services (DECLARED ONLY, not wired) |

### Layout (`aurora_layout`)

| Class | Header | Purpose |
|-------|--------|---------|
| `LayDocument` | `layout/LayDocument.h` | Layout view wrapper |
| `LayEditorController` | `layout/LayEditorController.h` | Grid, zoom, active tool, mouse dispatch |
| `LayGdsWriter` | `layout/LayGdsWriter.h` | Binary GDS II file writer with SREF hierarchy |
| `LayLefReader` | `layout/LayLefReader.h` | LEF macro/pin/layer parser |
| `LayOasisWriter`/`LayOasisReader` | `layout/LayOasisWriter.h` | OASIS round-trip |
| `LayCifIo` | `layout/LayCifIo.h` | CIF (Caltech Intermediate Form) read+write |
| `LayDxfWriter` | `layout/LayDxfWriter.h` | DXF (AutoCAD) export |
| `LayImageExport` | `layout/LayImageExport.h` | SVG / PDF / PPM raster export |
| `ImportWizard` | `layout/ImportWizard.h` | Auto-dispatch importer by file extension |
| `LayToolRect` | `layout/LayToolRect.h` | Rectangle draw tool |

### Schematic (`aurora_schematic`)

| Class | Header | Purpose |
|-------|--------|---------|
| `SchDocument` | `schematic/SchDocument.h` | Schematic view wrapper; owns SchWire list (BROKEN: wires on separate nets) |
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
| `PropertyEditorWidget` | `ui/PropertyEditorWidget.h` | Object property form (NEVER POPULATED) |

## Python Package (`python/aurora/`)

```
aurora/
  __init__.py
  batch.py          # I7 — batch script runner (untested)
  console.py        # I1 — embeddable interactive REPL
  custom_rules.py   # I5/I6 — registration API (not wired)
  macro.py          # I3 — MacroRecorder (same fake state as C++ side)
  ui_callbacks.py   # I4 — @register decorator (not wired)
  pdk/
    pcell_base.py   # PcellBase ABC
    registry.py     # register_pcell / get_pcell
  sim/
    __init__.py     # run_spice(), SimResult, SimWaveform
  layout/
    __init__.py     # I8 — array_place, route_l_shape, bounding_box
  schematic/
    __init__.py     # I9 — SchematicBuilder accumulator
  examples/
    nmos_pcell.py   # NmosPcell — single-finger NMOS reference PCell
  db/__init__.py    # namespace stub
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
- **Undo is completely fake** — pushes string "state", restores nothing.
- **Wire tool creates new net per segment** — cannot draw connected wires.
- **Pin labels are at garbage coordinates** — `mpin->id() * 3000`.
