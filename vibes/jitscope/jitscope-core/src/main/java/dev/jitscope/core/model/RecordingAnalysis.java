package dev.jitscope.core.model;

import java.util.List;
import java.util.Optional;

/**
 * Complete analysis of one recording. compileIds are scoped to {@code metadata.recordingId()}.
 * {@code executionSampleTotal} = total valid ExecutionSample count (denominator for shares);
 * CPUTimeSample kept separate in {@code cpuTimeSamples}.
 */
public record RecordingAnalysis(
        RecordingMetadata metadata,
        List<CompilationRecord> compilations,
        List<DeoptimizationRecord> deoptimizations,
        List<InliningRecord> inlining,
        List<MethodLifecycle> lifecycles,
        long executionSampleTotal,
        Optional<CpuTimeSamples> cpuTimeSamples,
        List<String> warnings,
        List<PathologyFinding> findings) {

    public RecordingAnalysis {
        compilations = compilations == null ? List.of() : List.copyOf(compilations);
        deoptimizations = deoptimizations == null ? List.of() : List.copyOf(deoptimizations);
        inlining = inlining == null ? List.of() : List.copyOf(inlining);
        lifecycles = lifecycles == null ? List.of() : List.copyOf(lifecycles);
        warnings = warnings == null ? List.of() : List.copyOf(warnings);
        findings = findings == null ? List.of() : List.copyOf(findings);
        cpuTimeSamples = cpuTimeSamples == null ? Optional.empty() : cpuTimeSamples;
    }
}
