package dev.jitscope.core.model;

/** Confidence-aware provenance for every lifecycle transition and finding. */
public enum EvidenceLevel {
    /** Directly observed in a JFR event (or LogCompilation XML). */
    OBSERVED,
    /** Inferred by correlation; not directly proven by an event. */
    INFERRED,
    /** Insufficient data to assert. */
    UNKNOWN
}
