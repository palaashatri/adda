package dev.jitscope.core.model;

import java.util.List;

/** A prioritized pathology finding with confidence-aware provenance and raw evidence. */
public record PathologyFinding(
        String id,
        PathologyType type,
        Severity severity,
        EvidenceLevel evidenceLevel,
        MethodKey primaryMethod,
        String title,
        String explanation,
        List<String> evidence,
        String recommendation) {

    public PathologyFinding {
        evidence = evidence == null ? List.of() : List.copyOf(evidence);
    }
}
