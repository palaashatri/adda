package dev.jitscope.core.model;

import java.util.Map;

/**
 * Per-method sampled frame statistics. {@code totalSamples} counts any occurrence in a stack;
 * {@code selfSamples} counts top frame (index 0); {@code byFrameType} breaks down observed
 * {@link FrameType}s. {@code selfShare}/{@code totalShare} are relative to total valid samples in
 * the recording (normalized; raw counts are never compared across recordings).
 */
public record FrameStats(
        long totalSamples,
        long selfSamples,
        Map<FrameType, Long> byFrameType,
        double selfShare,
        double totalShare) {

    public static final FrameStats EMPTY = new FrameStats(0, 0, Map.of(), 0.0, 0.0);

    public FrameStats {
        byFrameType = byFrameType == null ? Map.of() : Map.copyOf(byFrameType);
    }
}
