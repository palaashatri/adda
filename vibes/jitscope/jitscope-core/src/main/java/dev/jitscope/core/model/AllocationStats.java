package dev.jitscope.core.model;

import java.util.Map;

/**
 * Observed allocation sampling stats. Reports observed sample weight/frequency, never claims exact
 * heap byte totals.
 */
public record AllocationStats(
        long sampleCount,
        long totalSampleWeightBytes,
        double selfShare,
        double totalShare,
        Map<String, Long> byObjectClass) {

    public static final AllocationStats EMPTY = new AllocationStats(0, 0, 0.0, 0.0, Map.of());

    public AllocationStats {
        byObjectClass = byObjectClass == null ? Map.of() : Map.copyOf(byObjectClass);
    }
}
