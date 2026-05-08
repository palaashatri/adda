# OpenTracker — Implementation Status

_Last updated: 2026-05-08_

This document tracks the state of every roadmap item from `AGENTS.MD §9` plus
the surrounding feature inventory. It is the single source of truth for what
ships in the current branch.

---

## Legend

| Mark | Meaning                                              |
| ---- | ---------------------------------------------------- |
| ✅   | Implemented and exercised in this build              |
| 🟡   | Partially implemented (works in default config only) |
| ⏭️   | Documented future work, deliberately deferred        |

---

## Core stack (already shipped on `main`)

| Area                      | Status | Notes                                                            |
| ------------------------- | ------ | ---------------------------------------------------------------- |
| Gateway service (`:8080`) | ✅     | REST + SSE fan-out, API-key filter, rate limit filter            |
| Ingestion service         | ✅     | Mock + OpenSky + ADS-B + AISStream + AISHub + Celestrak TLE      |
| Stream processor          | ✅     | Kafka consumer → PostgreSQL + Redis cache                        |
| Geospatial query service  | ✅     | PostGIS bounding-box queries + track queries                     |
| Angular SPA               | ✅     | CesiumJS 1.116, layer panel, stats panel, entity detail, search  |
| Mock data simulators      | ✅     | Deterministic flights / ships / satellites with `MOCK_SEED`      |
| TLE propagator            | ✅     | Pure-Java SGP4-style Keplerian propagator, no external deps      |
| Docker compose            | ✅     | `./run.sh mock up` / `up` / `down` / `clean`                     |
| CI                        | ✅     | `.github/workflows/ci.yml` + `build-and-push.yml`                |

---

## Roadmap items (AGENTS.MD §9)

### 9.1 Viewport-aware SSE filtering — ⏭️

**Status:** Deferred.
**Why deferred:** The current SSE broadcaster forwards raw Kafka payloads as
strings. Adding bbox filtering requires either parsing every event in the hot
path or shifting filtering into Cesium-side culling. The frontend already
performs entity-level frustum culling via `EntityCluster`, which mitigates the
rendering cost at typical loads (≤2k entities). A first-class viewport filter
should be revisited when the project ships a 10k+ entity benchmark scenario.

### 9.2 Entity clustering at low zoom — ✅

`GlobeEngineService.init()` enables `viewer.entities.cluster` with
`pixelRange = 48` and `minimumClusterSize = 4`. The cluster event handler
recolors and counts grouped flight/ship/satellite entities so the global view
remains legible at any zoom level.

### 9.3 3D aircraft and vessel models (GLTF) — ⏭️

**Status:** Deferred.
**Why deferred:** Requires bundling CC0 GLTF assets and a zoom-tier swap
between `point:` and `model:` entity graphics. Out of scope for this pass to
keep bundle size and licensing footprint stable.

### 9.4 Satellite ground track visualization — ✅

`GlobeEngineService.showSatelliteFootprint()` renders a footprint ellipse and
a nadir line for the selected satellite. Track polylines are rendered for
flights and vessels (see 9.6).

### 9.5 Real-time search / geosearch — ✅

`GlobeComponent.onSearchInput()` (via the `/` keyboard shortcut) does local
entity search first, then falls back to Nominatim geocoding for places.
Selecting a hit calls `globe.flyTo(lat, lon, alt)`.

### 9.6 Track polylines on entity click — ✅

`EntityDetailComponent.loadFlightTrack()` / `loadShipTrack()` calls
`/api/{flights|ships}/{id}/track`, then `globe.renderTrack()` draws the
returned points as a polyline. Tracks are cleared when the detail panel closes.

### 9.7 TimescaleDB for track history — ✅ _(new this pass)_

`backend/stream-processor/src/main/resources/db/migration/V3__timescaledb_optional.sql`
converts `aircraft_positions` and `vessel_positions` into hypertables and
attaches a 7-day compression policy when the TimescaleDB extension is
available. The migration is a guarded no-op on stock PostgreSQL, so the
default `postgres:16-3.4-alpine` image continues to work unchanged. To enable
TimescaleDB, swap the postgres service image in `deploy/docker-compose.yml`
to `timescale/timescaledb-ha:pg16`; Flyway will pick up the V3 migration on
next boot.

### 9.8 WebSocket upgrade path — ⏭️

**Status:** Deferred.
**Why deferred:** SSE meets every current client requirement and is simpler
to scale horizontally behind nginx. WebSocket should land alongside the
time-scrubbing playback feature that motivates a bidirectional channel.

### 9.9 Airport and port layer — ✅ _(new this pass)_

`frontend/digital-twin-earth-app/src/app/services/airports.data.ts` ships a
curated, public-domain subset of 70+ major international airports.
`GlobeEngineService.setAirportsVisible()` toggles them as IATA-labeled
billboards with the orange Apple HIG accent. The toggle is wired into the
layers panel and the `A` keyboard shortcut. The full OurAirports CSV can
be loaded in the same shape later without UI changes.

### 9.10 Keyboard shortcuts — ✅

`GlobeComponent.onKeydown()` listens on `document:keydown`:

| Key       | Action                       |
| --------- | ---------------------------- |
| `F`       | Toggle flights               |
| `S`       | Toggle ships                 |
| `T`       | Toggle satellites            |
| `A`       | Toggle airports _(new)_      |
| `H`       | Fly to home                  |
| `R`       | Recover Earth view           |
| `+` / `-` | Zoom in / out                |
| `/`       | Focus search                 |
| `Escape`  | Close panel / dismiss search |

### 9.11 Cesium Ion terrain (optional) — ✅

`GlobeEngineService.applyTerrainProvider()` checks `window.CESIUM_ION_TOKEN`
or `localStorage.cesiumIonToken`. When present, it loads
`Cesium.createWorldTerrainAsync()` and enables `depthTestAgainstTerrain`. The
default zero-key path uses `EllipsoidTerrainProvider`. Setup guidance lives
in `docs/IMAGERY_SETUP.md`.

### 9.12 Helm chart for Kubernetes — ⏭️

**Status:** Deferred.
**Why deferred:** docker-compose covers the local stress-test story today.
Helm should land paired with the Zing benchmark publication so charts and
manifests are validated together rather than written speculatively.

### 9.13 GeoJSON / CSV export — ✅ _(new this pass)_

`backend/gateway/.../controller/ExportController.java` exposes:

| Endpoint                                  | Returns                          |
| ----------------------------------------- | -------------------------------- |
| `GET /api/export/flights.geojson?bbox=…`  | `FeatureCollection` of flights   |
| `GET /api/export/ships.geojson?bbox=…`    | `FeatureCollection` of vessels   |
| `GET /api/export/satellites.geojson`      | `FeatureCollection` of satellites |

Each response sets `Content-Disposition: attachment` and uses the
`application/geo+json` MIME type so QGIS, Mapbox, and Kepler.gl consume it
out of the box. The frontend exposes one-click downloads in the stats panel
(`Export · Flights · Ships · Sats`).

---

## Quick verification checklist

```bash
# Build everything (skips tests)
./gradlew build -x test

# Bring the stack up in mock mode
./run.sh mock up

# Smoke-test the new export endpoints
curl -H 'X-Api-Key: dev-key' -o flights.geojson \
     'http://localhost:8080/api/export/flights.geojson'
curl -H 'X-Api-Key: dev-key' -o ships.geojson \
     'http://localhost:8080/api/export/ships.geojson'

# Open the SPA, hit "A" to toggle the airports layer,
# click "Export · Flights" in the stats panel.
```

---

## Files added or modified in this pass

```
backend/gateway/src/main/java/com/digitaltwin/gateway/controller/ExportController.java   (new)
backend/stream-processor/src/main/resources/db/migration/V3__timescaledb_optional.sql    (new)
frontend/digital-twin-earth-app/src/app/services/airports.data.ts                        (new)
frontend/digital-twin-earth-app/src/app/services/globe-engine.service.ts                 (airport layer + lifecycle)
frontend/digital-twin-earth-app/src/app/globe/globe.component.ts                         (airport toggle + 'A' shortcut)
frontend/digital-twin-earth-app/src/app/globe/globe.component.html                       (pass showAirports input)
frontend/digital-twin-earth-app/src/app/globe/layer-toggle/layer-toggle.component.ts     (showAirports input)
frontend/digital-twin-earth-app/src/app/globe/layer-toggle/layer-toggle.component.html   (airport toggle row)
frontend/digital-twin-earth-app/src/app/globe/layer-toggle/layer-toggle.component.scss   (airport indicator styling)
frontend/digital-twin-earth-app/src/app/globe/stats-panel/stats-panel.component.ts       (export action)
frontend/digital-twin-earth-app/src/app/globe/stats-panel/stats-panel.component.html     (export row)
frontend/digital-twin-earth-app/src/app/globe/stats-panel/stats-panel.component.scss     (export styling)
status.md                                                                                (this file)
```

---

## Next-up suggestions

1. **9.1 Viewport-aware SSE filtering** — biggest scalability lift; cleanest
   implementation introduces a typed `PositionEnvelope` Kafka payload so the
   gateway can filter without re-parsing JSON.
2. **9.3 3D models** — most user-facing visual upgrade; bundle a single
   shared CC0 jet GLTF and one hull GLTF, pick by zoom altitude.
3. **9.12 Helm chart** — required to land the Zing benchmark publication.
