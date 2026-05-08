package com.aetheris.shared;

import com.aetheris.shared.proto.GeoEntity;

public class GenericSpatialEntity implements SpatialEntity {
    private final String id;
    private final GeoEntity.EntityType type;
    private final double latitude;
    private final double longitude;
    private final float altitude;
    private final float velocity;
    private final float heading;
    private final long timestamp;
    private final byte[] compressedMetadata;

    public GenericSpatialEntity(String id, GeoEntity.EntityType type, double latitude, double longitude, 
                                float altitude, float velocity, float heading, long timestamp, byte[] compressedMetadata) {
        this.id = id;
        this.type = type;
        this.latitude = latitude;
        this.longitude = longitude;
        this.altitude = altitude;
        this.velocity = velocity;
        this.heading = heading;
        this.timestamp = timestamp;
        this.compressedMetadata = compressedMetadata;
    }

    @Override
    public String getId() { return id; }

    @Override
    public GeoEntity.EntityType getType() { return type; }

    @Override
    public double getLatitude() { return latitude; }

    @Override
    public double getLongitude() { return longitude; }

    @Override
    public float getAltitude() { return altitude; }

    @Override
    public float getVelocity() { return velocity; }

    @Override
    public float getHeading() { return heading; }

    @Override
    public long getTimestamp() { return timestamp; }

    @Override
    public byte[] getCompressedMetadata() { return compressedMetadata; }

    public static Builder newBuilder() {
        return new Builder();
    }

    public static class Builder {
        private String id;
        private GeoEntity.EntityType type = GeoEntity.EntityType.UNKNOWN;
        private double latitude;
        private double longitude;
        private float altitude;
        private float velocity;
        private float heading;
        private long timestamp = System.currentTimeMillis();
        private byte[] compressedMetadata = new byte[0];

        public Builder setId(String id) { this.id = id; return this; }
        public Builder setType(GeoEntity.EntityType type) { this.type = type; return this; }
        public Builder setLatitude(double latitude) { this.latitude = latitude; return this; }
        public Builder setLongitude(double longitude) { this.longitude = longitude; return this; }
        public Builder setAltitude(float altitude) { this.altitude = altitude; return this; }
        public Builder setVelocity(float velocity) { this.velocity = velocity; return this; }
        public Builder setHeading(float heading) { this.heading = heading; return this; }
        public Builder setTimestamp(long timestamp) { this.timestamp = timestamp; return this; }
        public Builder setCompressedMetadata(byte[] compressedMetadata) { this.compressedMetadata = compressedMetadata; return this; }

        public GenericSpatialEntity build() {
            return new GenericSpatialEntity(id, type, latitude, longitude, altitude, velocity, heading, timestamp, compressedMetadata);
        }
    }
}
