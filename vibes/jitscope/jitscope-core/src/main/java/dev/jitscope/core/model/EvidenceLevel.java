package dev.jitscope.core.model;

/** Confidence-aware provenance for every lifecycle transition and finding. */
public enum EvidenceLevel {
    /** Directly observed in a JFR event (or LogCompilation XML). */
    OBSERVED,
    /** Inferred by correlation; not directly proven by an event. */
    INFERRED,
    /** Temporally correlated (e.g. samples increased after a deopt); not proven causation. */
    CORRELATED,
    /** Insufficient data to assert. */
    UNKNOWN
}
