# aurora-eda Status

Last updated: 2026-05-03

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

## Pending

- Extend Task 2 geometry and technology work:
  - full polygon boolean operations
  - richer DRC primitive checks beyond rectangles
  - schema versioning and migrations for `tech.json`
- Complete Task 3 UI shell:
  - Menus, actions, toolbars, status readouts
  - Library/design browser backed by `ProjectManager`
  - Database-backed rendering instead of sample schematic/layout content
- Complete Task 4 schematic MVP:
  - Symbols, wires, buses, junctions, properties
  - Hierarchy navigation
  - SPICE/CDL netlisting beyond the placeholder generator
- Complete Task 5 layout MVP:
  - Shape editing tools
  - Layer visibility/selectability state
  - DRC marker overlay
  - GDS import/export
- Complete Task 6 Python bindings and PCells:
  - Expand pybind11 bindings for db/tech/layout
  - Add resistor PCell
  - Add C++ to Python PCell invocation
- Complete Task 7 simulation integration:
  - `SimManager`
  - ngspice/Xyce command invocation
  - result parsing and Python sweep helpers
- Complete Task 8 DRC/LVS hooks:
  - generic `tools.yaml`
  - `DrcRunner`, `LvsRunner`, `ResultImporter`
- Complete Task 9 optional OA/Virtuoso import plugin:
  - `src/plugins/io/openaccess/`
  - optional SDK detection
  - `AURORA_HAVE_OPENACCESS` guarded stubs
- Add CI configuration once the repository is initialized.

## Verification

- `cmake -S . -B build-default`
- `cmake --build build-default`
- `ctest --test-dir build-default --output-on-failure`
- `./build.sh --build-dir build-script`
- `bash -n build.sh`
- `./build.sh --build-dir build-script-stub --no-ui --no-python --no-test`
- `./build.sh --build-dir build-task2`
- `./build.sh --build-dir build-task3`

Latest known result: all current tests passed on macOS with AppleClang and Qt 6
available. The normal script path builds the Qt `aurora-eda` executable and runs
`aurora_db_smoke`, `aurora_geom_ops_test`, and `aurora_tech_database_test`. The
no-UI script path builds the fallback executable. `aurora_python` currently falls
back to its stub target when pybind11 is not discoverable by CMake.
