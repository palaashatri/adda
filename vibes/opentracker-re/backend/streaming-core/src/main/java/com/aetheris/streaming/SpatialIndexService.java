package com.aetheris.streaming;

import com.aetheris.shared.SpatialEntity;
import com.uber.h3core.H3Core;
import com.uber.h3core.util.GeoCoord;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;

import java.io.IOException;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.stream.Collectors;

@Service
public class SpatialIndexService {
    private static final Logger logger = LoggerFactory.getLogger(SpatialIndexService.class);
    private final H3Core h3;
    private final int resolution = 7; // Approx 5km resolution

    // Map of H3 Index -> Map of Entity ID -> Entity
    private final ConcurrentMap<Long, ConcurrentMap<String, SpatialEntity>> index = new ConcurrentHashMap<>();
    
    // Reverse map for quick updates: Entity ID -> Last H3 Index
    private final ConcurrentMap<String, Long> entityToH3 = new ConcurrentHashMap<>();

    public SpatialIndexService() throws IOException {
        this.h3 = H3Core.newInstance();
        logger.info("H3 Spatial Index initialized at resolution {}", resolution);
    }

    public void updateEntity(SpatialEntity entity) {
        long h3Index = h3.geoToH3(entity.getLatitude(), entity.getLongitude(), resolution);
        
        Long oldH3Index = entityToH3.put(entity.getId(), h3Index);
        
        if (oldH3Index != null && oldH3Index != h3Index) {
            ConcurrentMap<String, SpatialEntity> cell = index.get(oldH3Index);
            if (cell != null) {
                cell.remove(entity.getId());
                if (cell.isEmpty()) index.remove(oldH3Index, Collections.emptyMap());
            }
        }
        
        index.computeIfAbsent(h3Index, k -> new ConcurrentHashMap<>())
             .put(entity.getId(), entity);
    }

    public void removeEntity(String id) {
        Long h3Index = entityToH3.remove(id);
        if (h3Index != null) {
            ConcurrentMap<String, SpatialEntity> cell = index.get(h3Index);
            if (cell != null) {
                cell.remove(id);
            }
        }
    }

    public Collection<SpatialEntity> getEntitiesInBounds(double minLat, double maxLat, double minLon, double maxLon) {
        // Simple bounding box query using H3: find cells in the box
        // For simplicity, we'll collect all cells that might be in bounds
        // In a real scenario, we'd use h3.polyfill or similar if it's a polygon,
        // or just calculate the range of H3 cells.
        // For now, let's filter the cells we have.
        
        return index.entrySet().parallelStream()
                .filter(entry -> {
                    GeoCoord coord = h3.h3ToGeo(entry.getKey());
                    return isWithin(coord.lat, coord.lng, minLat, maxLat, minLon, maxLon);
                })
                .flatMap(entry -> entry.getValue().values().stream())
                .collect(Collectors.toList());
    }

    private boolean isWithin(double lat, double lon, double minLat, double maxLat, double minLon, double maxLon) {
        return lat >= minLat && lat <= maxLat &&
               lon >= minLon && lon <= maxLon;
    }
    
    public Collection<SpatialEntity> getAllEntities() {
        return index.values().stream()
                .flatMap(m -> m.values().stream())
                .collect(Collectors.toList());
    }
}
