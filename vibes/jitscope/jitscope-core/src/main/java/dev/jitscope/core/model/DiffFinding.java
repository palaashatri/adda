package dev.jitscope.core.model;

/** A single regression between baseline and candidate recordings. */
public record DiffFinding(
        MethodKey method,
        DiffCategory category,
        Severity severity,
        EvidenceLevel evidenceLevel,
        String baselineSummary,
        String candidateSummary,
        double sampleShareDelta,
        String explanation) {}
