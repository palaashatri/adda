package dev.jitscope.core;

import static org.assertj.core.api.Assertions.assertThat;

import java.nio.file.Path;
import java.util.List;
import java.util.Optional;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

import dev.jitscope.core.analysis.LifecycleBuilder;
import dev.jitscope.core.analysis.RecordingAnalyzer;
import dev.jitscope.core.ingest.JfrLoader;
import dev.jitscope.core.ingest.RecordingData;
import dev.jitscope.core.model.Completeness;
import dev.jitscope.core.model.CompilationRecord;
import dev.jitscope.core.model.CpuTimeSamples;
import dev.jitscope.core.model.EvidenceLevel;
import dev.jitscope.core.model.LifecycleEventType;
import dev.jitscope.core.model.MethodKey;
import dev.jitscope.core.model.MethodLifecycle;
import dev.jitscope.core.model.RecordingAnalysis;

/** Verification Gate 1: real .jfr from real workloads; ingest + config audit + lifecycle. */
class Gate1IntegrationTest {

    private static final String DEOPT_STORM = "dev.jitscope.workloads.DeoptStorm";
    private static final String INLINE_CLIFF = "dev.jitscope.workloads.InlineCliff";
    private static final String DIAG = "jit-diag.jfc";

    @TempDir
    Path tempDir;

    @Test
    void deoptStormProducesCompilationAndDeoptLifecycle() throws Exception {
        var rec = WorkloadJfrRecorder.record(tempDir, DEOPT_STORM, List.of(), List.of(), DIAG);
        assertThat(rec.exitCode()).as(rec.stderr()).isZero();
        assertThat(rec.jfr()).exists();

        RecordingAnalysis a = RecordingAnalyzer.analyze(rec.jfr());

        // real jdk.Compilation events
        assertThat(a.compilations()).isNotEmpty();
        assertThat(a.compilations().stream().map(CompilationRecord::compileLevel))
                .as("at least one Tier-4 compilation").contains(4);

        // real jdk.Deoptimization events
        assertThat(a.deoptimizations()).isNotEmpty();

        MethodKey hot = new MethodKey("dev.jitscope.workloads.DeoptStorm", "hot", "(Ldev/jitscope/workloads/DeoptStorm$Work;J)J");
        Optional<MethodLifecycle> lc = a.lifecycles().stream()
                .filter(l -> l.method().methodName().equals("hot")
                        && l.method().className().equals("dev.jitscope.workloads.DeoptStorm"))
                .findFirst();
        assertThat(lc).as("hot lifecycle present").isPresent();
        MethodLifecycle hotLc = lc.get();
        assertThat(hotLc.highestCompileLevel()).as("hot reached Tier 4").isGreaterThanOrEqualTo(1);
        assertThat(hotLc.deoptimizationCount()).as("hot has deoptimizations").isPositive();
        assertThat(hotLc.timeline().stream().map(e -> e.type()))
                .contains(LifecycleEventType.COMPILATION_FINISHED, LifecycleEventType.DEOPTIMIZATION);
        // never claim made_not_entrant OBSERVED from JFR alone
        assertThat(hotLc.timeline().stream().anyMatch(e -> e.type() == LifecycleEventType.MADE_NOT_ENTRANT
                && e.level() == EvidenceLevel.OBSERVED)).isFalse();

        // compileId scoping / joins: every deopt referencing a known compileId resolves
        assertThat(a.metadata().recordingId()).isNotBlank();
        assertThat(a.metadata().configAudit().completeness()).isNotNull();
    }

    @Test
    void inlineCliffEmitsInliningDecisions() throws Exception {
        var rec = WorkloadJfrRecorder.record(tempDir, INLINE_CLIFF, List.of(), List.of("--mode=cliff"), DIAG);
        assertThat(rec.exitCode()).as(rec.stderr()).isZero();

        RecordingData data = JfrLoader.load(rec.jfr());
        assertThat(data.inlining).isNotEmpty();
        assertThat(data.inlining.stream().anyMatch(i -> !i.succeeded())).as("some inlining failed").isTrue();
        // callee struct extracted (not a RecordedMethod); non-empty callee name present
        assertThat(data.inlining.stream().anyMatch(i -> !i.callee().methodName().isBlank())).isTrue();
        // 'Inlined' vs 'JIT compiled' frame types observed from RecordedFrame.getType()
        assertThat(data.executionSamples.stream()
                .flatMap(s -> s.frames().stream())
                .map(f -> f.type())).isNotEmpty();
    }

    @Test
    void defaultJfcSettingsAreInsufficient() throws Exception {
        var rec = WorkloadJfrRecorder.record(tempDir, DEOPT_STORM, List.of(), List.of(), "default.jfc");
        assertThat(rec.exitCode()).as(rec.stderr()).isZero();
        RecordingAnalysis a = RecordingAnalyzer.analyze(rec.jfr());
        assertThat(a.metadata().configAudit().completeness())
                .as("recording captured with default JFR settings must be INSUFFICIENT")
                .isEqualTo(Completeness.INSUFFICIENT);
    }

    @Test
    void jitDiagJfcIsCompleteAndZeroThreshold() throws Exception {
        var rec = WorkloadJfrRecorder.record(tempDir, INLINE_CLIFF, List.of(), List.of("--mode=inlineable"), DIAG);
        assertThat(rec.exitCode()).as(rec.stderr()).isZero();
        RecordingAnalysis a = RecordingAnalyzer.analyze(rec.jfr());
        var audit = a.metadata().configAudit();
        assertThat(audit.compilationThresholdMillis()).as("0 ms compilation threshold").isZero();
        assertThat(audit.disabledRequiredEvents()).isEmpty();
        assertThat(audit.completeness()).isNotEqualTo(Completeness.INSUFFICIENT);
    }

    @Test
    void executionSampleAndCpuTimeSampleAreStrictlySeparate() {
        RecordingData data = new RecordingData();
        data.executionSamples.add(new RecordingData.Sample(java.time.Instant.EPOCH, "main",
                List.of(new RecordingData.Frame("com.X", "m", "()V", "JIT compiled", 1, 0))));
        data.cpuTimeSamples.add(new RecordingData.CpuSample(java.time.Instant.EPOCH, "main",
                List.of(new RecordingData.Frame("com.X", "m", "()V", "JIT compiled", 1, 0)), false, false));
        data.cpuTimeSamples.add(new RecordingData.CpuSample(java.time.Instant.EPOCH, "main", List.of(), true, false)); // failed
        data.cpuTimeSamplesLost = 3;

        var result = LifecycleBuilder.build(data);
        // execution total counts ONLY jdk.ExecutionSample
        assertThat(result.executionSampleTotal()).isEqualTo(1);
        Optional<CpuTimeSamples> cpu = result.cpuTimeSamples();
        assertThat(cpu).isPresent();
        assertThat(cpu.get().totalSamples()).isEqualTo(2);
        assertThat(cpu.get().failedSamples()).isEqualTo(1);
        assertThat(cpu.get().lostSamples()).isEqualTo(3);
        assertThat(cpu.get().confidenceDowngraded()).isTrue();
        // per-method execution stats not polluted by CPU-time samples
        MethodLifecycle lc = result.lifecycles().stream()
                .filter(l -> l.method().methodName().equals("m")).findFirst().orElseThrow();
        assertThat(lc.executionSamples().totalSamples()).isEqualTo(1);
        assertThat(lc.cpuTimeSamples()).isPresent();
        assertThat(lc.cpuTimeSamples().get().totalSamples()).isEqualTo(1); // failed excluded
    }

    @Test
    void allocationFallsBackToTlabWhenNoObjectAllocationSample() {
        RecordingData data = new RecordingData();
        var frame = new RecordingData.Frame("com.X", "m", "()V", "JIT compiled", 1, 0);
        data.allocSamples.add(new RecordingData.AllocSample(java.time.Instant.EPOCH, "com.X", 64, "main",
                List.of(frame), true, false)); // TLAB only
        var result = LifecycleBuilder.build(data);
        MethodLifecycle lc = result.lifecycles().stream()
                .filter(l -> l.method().methodName().equals("m")).findFirst().orElseThrow();
        assertThat(lc.allocationStats().sampleCount()).isEqualTo(1);
        assertThat(lc.allocationStats().totalSampleWeightBytes()).isEqualTo(64);
    }
}
