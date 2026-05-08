# Open-Source Analog/Custom IC Environment

## Goal

Build `aurora-eda`, an open-source, extensible analog/custom IC design environment
that approximates Cadence Virtuoso-style workflows and can import
Virtuoso/OpenAccess-based design data for academic use without copying Cadence
code, SKILL, or proprietary assets.

## Stack

- C++20 core for geometry, database, layout engine, and performance-critical code.
- Python for scripting, PCells, automation, flows, and academic experiments.
- Qt 6 Widgets for a deterministic cross-platform GUI.
- CMake with out-of-source builds and strict target boundaries.

## Legal And Scope Rules

- Do not reimplement SKILL or copy SKILL code.
- Do not reverse engineer or copy OpenAccess internals.
- Do not ship OA SDK, Cadence headers, or proprietary assets.
- OA/Virtuoso support must live in an optional plugin that builds only when the
  user has a legal OA installation, or through neutral intermediate formats such
  as GDS, LEF/DEF, CDL, SPICE, Verilog, and netlists.

## Repository Layout

```text
aurora-eda/
  CMakeLists.txt
  cmake/
  external/
  src/
    core/
    db/
    geom/
    tech/
    pdk/
    schematic/
    layout/
    netlist/
    sim/
    drc_lvs/
    ui/
    plugins/
  python/aurora/
  scripts/
  tests/
  docs/
```

## Core Targets

- `aurora_core`: core lifecycle, project management, plugins, database, geometry,
  tech, and netlist primitives.
- `aurora_schematic`: schematic document/controller classes.
- `aurora_layout`: layout document/controller/tool classes.
- `aurora_ui`: Qt Widgets shell when Qt is available, otherwise a buildable stub.
- `aurora_python`: pybind11 module when pybind11 is available, otherwise a
  buildable stub.
- `aurora_plugins_*`: plugin shared libraries as features are added.

## Development Rules

- Never create new top-level directories without explicit instruction.
- All C++ lives under `src/` with matching `CMakeLists.txt` files.
- All Python package code lives under `python/aurora/`.
- Tools and plugins live under `src/plugins/` with one subdirectory per plugin.
- Keep C++ headers and sources in sync.
- Add new classes to the appropriate `CMakeLists.txt`.
- Update `docs/ARCHITECTURE.md` and relevant API docs when adding features.
- Update `docs/PDK_SPEC.md` for PDK/PCell changes.
- Update `docs/FILE_FORMAT.md` for file formats and import/export behavior.

## Initial Task Order

1. Core skeleton: CMake targets, `CoreApp`, `ProjectManager`, `PluginManager`,
   and the ID-based database classes.
2. Geometry and technology database loading.
3. Qt UI shell.
4. Schematic MVP.
5. Layout MVP.
6. Python bindings and PCells.
7. Simulation integration.
8. DRC/LVS hooks.
9. Optional OpenAccess import plugin.
