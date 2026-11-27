package dev.jitscope.core.model;

/**
 * Strictly-separate JDK 25+ Linux experimental {@code jdk.CPUTimeSample} (JEP 509) data.
 * Never merged with {@code jdk.ExecutionSample}. Confidence downgraded when sample loss or
 * {@code failed} (null stack trace) counts are non-zero.
 */
public record CpuTimeSamples(
        FrameStats frameStats,
        long totalSamples,
        long failedSamples,
        long biasedSamples,
        long lostSamples,
        boolean confidenceDowngraded) {

    public static final CpuTimeSamples EMPTY = new CpuTimeSamples(FrameStats.EMPTY, 0, 0, 0, 0, false);
}
