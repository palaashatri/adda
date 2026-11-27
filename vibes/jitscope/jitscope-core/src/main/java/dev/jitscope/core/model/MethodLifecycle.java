package dev.jitscope.core.model;

import java.util.List;
import java.util.Optional;

/** Full per-method lifecycle. ExecutionSample and CPUTimeSample kept strictly separate. */
public record MethodLifecycle(
        MethodKey method,
        List<LifecycleEvent> timeline,
        int highestCompileLevel,
        int compilationCount,
        int deoptimizationCount,
        FrameStats executionSamples,
        Optional<FrameStats> cpuTimeSamples,
        AllocationStats allocationStats,
        EvidenceLevel lifecycleConfidence) {

    public MethodLifecycle {
        timeline = timeline == null ? List.of() : List.copyOf(timeline);
        cpuTimeSamples = cpuTimeSamples == null ? Optional.empty() : cpuTimeSamples;
    }
}
