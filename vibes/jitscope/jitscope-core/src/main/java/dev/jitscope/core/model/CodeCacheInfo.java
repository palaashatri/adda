package dev.jitscope.core.model;

import java.time.Instant;
import java.util.List;
import java.util.Optional;

/**
 * Code-cache state. Reads JDK 21 (per-segment) and JDK 25 (unified) {@code jdk.CodeCacheStatistics}
 * schemas defensively; per-segment names not fabricated when absent.
 */
public record CodeCacheInfo(
        List<CodeCacheSnapshot> statistics,
        long codeCacheFullCount,
        Optional<CodeCacheConfiguration> configuration) {

    public CodeCacheInfo {
        statistics = statistics == null ? List.of() : List.copyOf(statistics);
        configuration = configuration == null ? Optional.empty() : configuration;
    }

    public static final CodeCacheInfo EMPTY = new CodeCacheInfo(List.of(), 0, Optional.empty());

    /** Peak occupancy across snapshots in [0,1]; -1 if unknown. */
    public double peakOccupancy() {
        return statistics.stream().mapToDouble(CodeCacheSnapshot::occupancy).max().orElse(-1.0);
    }

    public record CodeCacheSnapshot(
            Instant timestamp,
            String codeBlobType,
            long reservedTotalBytes,
            long usedBytes,
            long unallocatedCapacityBytes,
            long fullCount,
            long methodCount,
            double occupancy) {}

    public record CodeCacheConfiguration(
            long reservedSizeBytes,
            long nonProfiledSizeBytes,
            long profiledSizeBytes,
            long nonNmethodSizeBytes,
            long expansionSizeBytes,
            String codeBlobType) {}
}
