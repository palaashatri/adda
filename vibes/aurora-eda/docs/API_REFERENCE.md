# API Reference

## Core

### `aurora::core::CoreApp`

- `initialize(pluginDirectory = {})`: initializes the app and optionally loads
  plugins from a directory.
- `shutdown()`: closes the current project and marks the app uninitialized.
- `projects()`: returns the `ProjectManager`.
- `plugins()`: returns the `PluginManager`.

### `aurora::core::ProjectManager`

- `createProject(path)`: creates `libraries/`, `pdk/`, and `config/` under a
  project directory and writes `config/project.json`.
- `openProject(path)`: opens an existing directory-based project.
- `workingLibrary()`: returns the in-memory working `DbCellLib`.

### `aurora::core::PluginManager`

- `loadPlugin(path)`: dynamically loads a shared library and calls
  `aurora_register_plugin`.
- `loadPluginsFromDirectory(path)`: attempts to load every regular file in a
  directory as a plugin.

## Database

All database objects use integer IDs. `0` is reserved as `kInvalidId`.

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
