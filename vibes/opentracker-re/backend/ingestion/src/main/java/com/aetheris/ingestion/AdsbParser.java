package com.aetheris.ingestion;

import com.aetheris.shared.proto.GeoEntity;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.HashMap;
import java.util.Map;

public class AdsbParser {
    private static final Logger logger = LoggerFactory.getLogger(AdsbParser.class);
    
    // State to keep track of aircraft info since SBS-1 splits data across messages
    private final Map<String, EntityState> aircraftStates = new HashMap<>();

    public GeoEntity parseSbs1(String line) {
        try {
            String[] parts = line.split(",");
            if (parts.length < 11) return null;

            String msgType = parts[0];
            String transmissionType = parts[1];
            String hexId = parts[4];

            EntityState state = aircraftStates.computeIfAbsent(hexId, id -> new EntityState(id));
            state.timestamp = System.currentTimeMillis();

            if ("MSG".equals(msgType)) {
                switch (transmissionType) {
                    case "1": // Identification
                        if (parts.length > 10) state.callsign = parts[10].trim();
                        break;
                    case "3": // Airborne Position
                        if (parts.length > 14 && !parts[14].isEmpty()) state.lat = Double.parseDouble(parts[14]);
                        if (parts.length > 15 && !parts[15].isEmpty()) state.lon = Double.parseDouble(parts[15]);
                        if (parts.length > 11 && !parts[11].isEmpty()) state.alt = Float.parseFloat(parts[11]) * 0.3048f; // ft to m
                        break;
                    case "4": // Airborne Velocity
                        if (parts.length > 12 && !parts[12].isEmpty()) state.velocity = Float.parseFloat(parts[12]);
                        if (parts.length > 13 && !parts[13].isEmpty()) state.heading = Float.parseFloat(parts[13]);
                        break;
                }
            }

            // Only return if we have at least position
            if (state.lat != 0 && state.lon != 0) {
                return GeoEntity.newBuilder()
                        .setId(state.hexId)
                        .setType(GeoEntity.EntityType.FLIGHT)
                        .setLatitude(state.lat)
                        .setLongitude(state.lon)
                        .setAltitude(state.alt)
                        .setVelocity(state.velocity)
                        .setHeading(state.heading)
                        .setTimestamp(state.timestamp)
                        .build();
            }
        } catch (Exception e) {
            logger.error("Error parsing SBS-1 line: {}", line, e);
        }
        return null;
    }

    private static class EntityState {
        String hexId;
        String callsign;
        double lat;
        double lon;
        float alt;
        float velocity;
        float heading;
        long timestamp;

        EntityState(String hexId) { this.hexId = hexId; }
    }
}
