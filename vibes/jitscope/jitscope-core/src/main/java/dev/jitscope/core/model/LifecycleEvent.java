package dev.jitscope.core.model;

import java.time.Instant;

/** A single per-method lifecycle transition with confidence-aware provenance. */
public record LifecycleEvent(
        Instant timestamp,
        LifecycleEventType type,
        EvidenceLevel level,
        Integer compileId,
        Integer compileLevel,
        String detail,
        Evidence evidence) {

    public LifecycleEvent {
        if (timestamp == null) {
            timestamp = Instant.EPOCH;
        }
    }
}
