# API Reference

## Core

### `aurora::core::CoreApp`

- `initialize(pluginDirectory = {})`: initializes the app, loads plugins.
- `shutdown()`: closes the project, marks uninitialized.
- `projects()`: returns the `ProjectManager`.
- `plugins()`: returns the `PluginManager`.
- `tech()`: returns the global `TechDatabase`.

### `aurora::core::ProjectManager`

- `createProject(path)`: creates project directory structure + manifest.
- `openProject(path)`: loads a directory-based project with design.json.
- `saveProject()`: serializes working library + manifest.
- `workingLibrary()`: returns the in-memory `DbCellLib`.
- `addLibrarySearchPath(path)`: adds a library search path.

### `aurora::core::PluginManager`

- `loadPlugin(path)`: loads a shared library plugin.
- `loadPluginsFromDirectory(path)`: loads all regular files as plugins.
- `registeredPlugins()`: returns list of loaded plugin descriptors.

### `aurora::core::LibraryManager`

- `addSearchPath(path)`: adds library search path.
- `attachLibrary(path)`: attaches a library by path.
- `detachLibrary(name)`: detaches a library.
- `setTechFile(path)`: associates a tech file with the library.
- `readCdsLib(path)`: reads cds.lib format.
- `diffSnapshots(a, b)`: line-diff between two cell-lib JSON snapshots.

### `aurora::core::DesignServices`

- `HierarchyBrowser::build(lib)`: builds a hierarchy tree from the library.
- `ProjectArchiver::archive(project, outputPath)`: creates AURORA-AR-1 bundle.
- `DesignHealthDashboard::compile(lib)`: cell count, warnings, errors summary.

### `aurora::core::ScriptEngine`

- `MacroRecorder`: records actions as Python strings.
- `ScriptedUiRegistry`: registers Python callbacks for menu/toolbar items.
- `CustomRuleRegistry`: registers Python DRC rules and simulation analyses.
- `BatchRunner`: runs a Python script in headless mode.

## Database

All database objects use integer IDs (`uint64_t`). `0` is reserved as `kInvalidId`.

### `DbCellLib`

Owns cells and technology layers for a library. Provides name-based and ID-based
cell lookup.

### `DbCell`

Owns views of a design cell and cell-level parameters. Each view type is unique
per cell in the current skeleton.

### `DbView`

Owns shapes, instances, nets, pins, and constraints for a concrete view such as
`schematic` or `layout`.

### Shapes

`DbShape` is the base class. The initial derived classes are `DbRect`,
`DbPolygon`, `DbPath`, and `DbText`.

## Geometry

### Primitives

- `GeomPoint`: integer database coordinate pair.
- `GeomBox`: axis-aligned rectangle with intersection and translation helpers.
- `GeomPolygon`: ordered point list with bounding-box support.
- `GeomPath`: ordered point list with integer width.

### `GeomOps`

- `snapCoordinate(value, grid)`: snaps one coordinate to the nearest grid point.
- `snapToGrid(...)`: snaps points, boxes, polygons, and paths.
- `isManhattan(polygon)`: checks whether every polygon edge is horizontal or
  vertical.
- `boxIntersection(lhs, rhs)`: rectangle intersection.
- `boxUnion(lhs, rhs)`: exact single-rectangle union for mergeable boxes,
  otherwise returns the input boxes as separate regions.
- `boxDifference(subject, cutter)`: rectangle subtraction, returning up to four
  boxes.
- `meetsMinimumWidth(box, minWidth)`: checks the smaller box dimension.
- `meetsMinimumSpacing(lhs, rhs, minSpacing)`: checks Manhattan edge spacing.
- `hasEnclosure(outer, inner, enclosure)`: checks symmetric rectangular
  enclosure.

## Technology

### `aurora::tech::TechDatabase`

- `loadFromJsonFile(path)`: parses a `tech.json` file.
- `name()`: technology name.
- `units()`: database unit metadata.
- `layerIds()`: loaded layer IDs.
- `findLayerById(id)` / `findLayerByName(name)`: layer lookup.
- `rules()`: generic rule list loaded from the top-level `rules` array.
- `defaultWidthForLayer(name)` / `defaultSpacingForLayer(name)`: convenience
  accessors for layer defaults.
- `lastError()`: parse or file-loading error text after a failed load.

## Python

The pure-Python PCell API starts with:

- `aurora.pdk.PcellBase`
- `aurora.pdk.register_pcell(name, cls)`
- `aurora.pdk.get_pcell(name)`
- `aurora.pdk.registry.get_all_pcells()` — returns list of (name, module, class)
  for C++ bridge discovery.

### PythonPcellBridge (C++, requires pybind11)

- `makePythonPcellGenerator(modulePath, className)` — creates a PcellGenerator
  from a Python PCell class.
- `registerPythonPcells(registry)` — discovers all registered Python PCells
  and registers them with the C++ PcellRegistry.

## Simulation

### `SimRunner`

- `writeSpiceNetlist(lib, cell, view)`: writes the SPICE file for ngspice.
- `run()`: spawns ngspice via popen, parses output.
- `result()`: returns `SimResult` with waveforms and DC op-point map.
- `runSweep(lib, cell, view, paramName, values)`: parametric sweep over values.
- `runMonteCarlo(lib, cell, view, dist, params, n)`: N runs with Gaussian/uniform.
- `runCorners(lib, cell, view, temps, vdds)`: temperature/VDD corner matrix.

### `SimResult`

- `waveforms`: vector of `SimWaveform` (name, time[], values[]).
- `dcOperatingPoint`: map of node name → voltage.

## DRC / LVS

### `DrcEngine`

- `run(view, lib, options={})`: runs width, spacing, Manhattan, antenna,
  density, and ERC checks against TechDatabase rules.
- Returns `std::vector<DrcViolation>`.

### `LvsChecker`

- `compare(schematicView, layoutView, lib)`: compares net names, pin counts,
  and device counts. Returns `LvsResult` with `matched` flag and error messages.

### `ParasiticExtractor` / `ParasiticReducer` / `PercChecker`

- Per-layer coupling C + wire R extraction (ParasiticExtractor).
- Pi/T/Lumped model reduction (ParasiticReducer).
- IR drop, current density, missing net detection (PercChecker).
