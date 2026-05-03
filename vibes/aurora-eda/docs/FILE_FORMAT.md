# File Format

Native project storage is directory-based:

```text
project.aurora/
  libraries/
  pdk/
  config/
```

The current skeleton writes `config/project.json` with a format marker, version,
and working library name. Cell/view JSON and binary geometry schemas will be
defined as the database serializer is implemented.

Future schema work must define:

- Cells, views, instances, nets, pins, layers, shapes, and constraints.
- Versioning and migration behavior.
- Mapping to and from GDS, LEF/DEF, SPICE/CDL, and optional OA import/export.
