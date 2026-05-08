package com.aetheris.graphql;

import org.springframework.graphql.data.method.annotation.Argument;
import org.springframework.graphql.data.method.annotation.QueryMapping;
import org.springframework.stereotype.Controller;

@Controller
public class MetadataController {

    @QueryMapping
    public EntityMetadata entityMetadata(@Argument String id) {
        EntityMetadata meta = new EntityMetadata();
        meta.id = id;
        
        if (id.startsWith("A")) { // Mocking some aviation data
            meta.name = "Boeing 737-800";
            meta.country = "Ireland";
            meta.flight = new FlightMetadata();
            meta.flight.airline = "Ryanair";
            meta.flight.origin = "DUB";
            meta.flight.destination = "STN";
            meta.flight.aircraftType = "B738";
        } else if (id.startsWith("M")) { // Mocking maritime
            meta.name = "Maersk Mc-Kinney Moller";
            meta.country = "Denmark";
            meta.ship = new ShipMetadata();
            meta.ship.vesselName = "MAERSK MC-KINNEY MOLLER";
            meta.ship.vesselType = "Container Ship";
            meta.ship.callsign = "OUJW2";
        } else {
            meta.name = "Unknown Object";
            meta.country = "International";
        }
        return meta;
    }

    public static class EntityMetadata {
        public String id;
        public String name;
        public String country;
        public FlightMetadata flight;
        public ShipMetadata ship;
        public SatelliteMetadata satellite;
    }

    public static class FlightMetadata {
        public String airline;
        public String origin;
        public String destination;
        public String aircraftType;
    }

    public static class ShipMetadata {
        public String vesselName;
        public String vesselType;
        public String callsign;
    }

    public static class SatelliteMetadata {
        public String intlDesignator;
        public String launchDate;
        public String owner;
    }
}
