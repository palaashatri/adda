package com.digitaltwin.gateway.controller;

import com.digitaltwin.gateway.proxy.GeospatialClient;
import com.digitaltwin.shared.dto.AircraftPositionDto;
import com.digitaltwin.shared.dto.SatellitePositionDto;
import com.digitaltwin.shared.dto.VesselPositionDto;
import org.springframework.http.HttpHeaders;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

@RestController
@RequestMapping("/api/export")
public class ExportController {

    private final GeospatialClient geospatialClient;

    public ExportController(GeospatialClient geospatialClient) {
        this.geospatialClient = geospatialClient;
    }

    @GetMapping(value = "/flights.geojson", produces = "application/geo+json")
    public ResponseEntity<Map<String, Object>> exportFlights(
            @RequestParam(defaultValue = "-90") double minLat,
            @RequestParam(defaultValue = "90") double maxLat,
            @RequestParam(defaultValue = "-180") double minLon,
            @RequestParam(defaultValue = "180") double maxLon) {
        List<AircraftPositionDto> flights = geospatialClient.getFlights(minLat, maxLat, minLon, maxLon);
        List<Map<String, Object>> features = flights.stream()
                .map(f -> feature(f.lon(), f.lat(), Map.of(
                        "id", nullSafe(f.id()),
                        "icao24", nullSafe(f.icao24()),
                        "callsign", nullSafe(f.callsign()),
                        "timestamp", String.valueOf(f.timestamp()),
                        "altitudeMeters", nullSafe(f.altitudeMeters()),
                        "groundSpeedMps", nullSafe(f.groundSpeedMps()),
                        "headingDeg", nullSafe(f.headingDeg()),
                        "verticalRateMps", nullSafe(f.verticalRateMps())
                )))
                .collect(Collectors.toList());
        return download("flights.geojson", featureCollection(features));
    }

    @GetMapping(value = "/ships.geojson", produces = "application/geo+json")
    public ResponseEntity<Map<String, Object>> exportShips(
            @RequestParam(defaultValue = "-90") double minLat,
            @RequestParam(defaultValue = "90") double maxLat,
            @RequestParam(defaultValue = "-180") double minLon,
            @RequestParam(defaultValue = "180") double maxLon) {
        List<VesselPositionDto> ships = geospatialClient.getShips(minLat, maxLat, minLon, maxLon);
        List<Map<String, Object>> features = ships.stream()
                .map(v -> feature(v.lon(), v.lat(), Map.of(
                        "id", nullSafe(v.id()),
                        "mmsi", nullSafe(v.mmsi()),
                        "name", nullSafe(v.name()),
                        "timestamp", String.valueOf(v.timestamp()),
                        "speedKnots", nullSafe(v.speedKnots()),
                        "courseDeg", nullSafe(v.courseDeg())
                )))
                .collect(Collectors.toList());
        return download("ships.geojson", featureCollection(features));
    }

    @GetMapping(value = "/satellites.geojson", produces = "application/geo+json")
    public ResponseEntity<Map<String, Object>> exportSatellites() {
        List<SatellitePositionDto> sats = geospatialClient.getSatellites();
        List<Map<String, Object>> features = sats.stream()
                .map(s -> feature(s.lon(), s.lat(), Map.of(
                        "noradId", nullSafe(s.noradId()),
                        "name", nullSafe(s.name()),
                        "timestamp", String.valueOf(s.timestamp()),
                        "altitudeKm", nullSafe(s.altitudeKm()),
                        "velocityKmS", nullSafe(s.velocityKmS())
                )))
                .collect(Collectors.toList());
        return download("satellites.geojson", featureCollection(features));
    }

    private static Map<String, Object> feature(double lon, double lat, Map<String, Object> props) {
        Map<String, Object> geom = new LinkedHashMap<>();
        geom.put("type", "Point");
        geom.put("coordinates", List.of(lon, lat));
        Map<String, Object> feat = new LinkedHashMap<>();
        feat.put("type", "Feature");
        feat.put("geometry", geom);
        feat.put("properties", props);
        return feat;
    }

    private static Map<String, Object> featureCollection(List<Map<String, Object>> features) {
        Map<String, Object> fc = new LinkedHashMap<>();
        fc.put("type", "FeatureCollection");
        fc.put("features", features);
        return fc;
    }

    private static Object nullSafe(Object value) {
        return value == null ? "" : value;
    }

    private static ResponseEntity<Map<String, Object>> download(String filename, Map<String, Object> body) {
        return ResponseEntity.ok()
                .header(HttpHeaders.CONTENT_DISPOSITION, "attachment; filename=\"" + filename + "\"")
                .contentType(MediaType.parseMediaType("application/geo+json"))
                .body(body);
    }
}
