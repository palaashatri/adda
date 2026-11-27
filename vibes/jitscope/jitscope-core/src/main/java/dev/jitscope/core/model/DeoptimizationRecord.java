package dev.jitscope.core.model;

import java.time.Instant;

/**
 * A deoptimization. Relational keys preserved: {@code compileId}, {@code compiler}, {@code bci},
 * {@code lineNumber}, {@code reason}, {@code action}, {@code method} (when present). compileId is
 * scoped to a recording; joins use {@code (recordingId, compileId)}.
 */
public record DeoptimizationRecord(
        int compileId,
        MethodKey method,
        int bci,
        int lineNumber,
        String reason,
        String action,
        String compiler,
        Instant timestamp) {

    public DeoptimizationRecord {
        if (timestamp == null) {
            timestamp = Instant.EPOCH;
        }
    }
}
