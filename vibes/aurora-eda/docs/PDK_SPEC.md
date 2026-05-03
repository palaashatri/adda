# PDK Specification

PDKs are directory-based and will contain:

```text
pdk.yaml
tech.json
devices/
pcells/
```

## `tech.json`

The current structured loader accepts this initial schema:

```json
{
  "name": "demo45",
  "units": {
    "database_unit": "nm",
    "dbu_per_micron": 1000
  },
  "layers": [
    {
      "id": 1,
      "name": "metal1",
      "purpose": "drawing",
      "color": "#3fbf7f",
      "gds": {"layer": 68, "datatype": 20},
      "rules": {"min_width": 140, "min_spacing": 140}
    }
  ],
  "rules": [
    {"layer": "metal1", "type": "width", "value": 140}
  ]
}
```

Layer entries require `name`. `purpose` defaults to `drawing`; color defaults to
`#808080`. GDS mapping can be expressed either as `gds.layer` /
`gds.datatype` or as `gds_layer` / `gds_datatype`. Width and spacing defaults can
be supplied as `default_width` / `default_spacing`, `min_width` /
`min_spacing`, or inside a layer `rules` object.

The top-level `rules` array is generic and stores `layer`, `type`, `value`, and
optional `applies_to`.

## PCells

Python PCells inherit from `aurora.pdk.PcellBase` and implement:

- `parameters() -> dict`
- `generate_layout(cell, tech, params)`
- `generate_schematic(cell, tech, params)` optionally

The initial repository includes only the base PCell interface and registry.
