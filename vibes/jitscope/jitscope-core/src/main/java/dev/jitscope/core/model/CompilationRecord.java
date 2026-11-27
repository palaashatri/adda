package dev.jitscope.core.model;

import java.time.Duration;
import java.time.Instant;

/**
 * A JIT compilation. Keyed by {@code (recordingId, compileId)}; compileId is never globally unique
 * across recordings. Success field on JFR {@code jdk.Compilation} is spelled {@code succeded} (sic).
 */
public record CompilationRecord(
        int compileId,
        MethodKey method,
        int compileLevel,
        boolean osr,
        boolean succeeded,
        Instant startTime,
        Duration duration,
        int codeSizeBytes,
        int inlinedBytes,
        String failureMessage) {

    public CompilationRecord {
        if (duration == null) {
            duration = Duration.ZERO;
        }
        if (startTime == null) {
            startTime = Instant.EPOCH;
        }
    }
}
