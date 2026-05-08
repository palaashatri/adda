package com.aetheris.streaming;

import com.aetheris.shared.GenericSpatialEntity;
import com.aetheris.shared.proto.GeoEntity;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/internal/ingest")
public class InternalIngestionController {
    private final SpatialIndexService spatialIndexService;

    public InternalIngestionController(SpatialIndexService spatialIndexService) {
        this.spatialIndexService = spatialIndexService;
    }

    @PostMapping("/entity")
    public void ingestEntity(@RequestBody byte[] protoBytes) throws Exception {
        GeoEntity proto = GeoEntity.parseFrom(protoBytes);
        spatialIndexService.updateEntity(fromProto(proto));
    }

    private GenericSpatialEntity fromProto(GeoEntity proto) {
        return GenericSpatialEntity.newBuilder()
                .setId(proto.getId())
                .setType(proto.getType())
                .setLatitude(proto.getLatitude())
                .setLongitude(proto.getLongitude())
                .setAltitude(proto.getAltitude())
                .setVelocity(proto.getVelocity())
                .setHeading(proto.getHeading())
                .setTimestamp(proto.getTimestamp())
                .setCompressedMetadata(proto.getCompressedMetadata().toByteArray())
                .build();
    }
}
