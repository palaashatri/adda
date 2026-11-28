package dev.jitscope.core;

import static org.assertj.core.api.Assertions.assertThat;

import java.nio.file.Path;
import java.util.List;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

import dev.jitscope.core.analysis.DiffEngine;
import dev.jitscope.core.analysis.RecordingAnalyzer;
import dev.jitscope.core.model.DiffCategory;
import dev.jitscope.core.model.PathologyType;
import dev.jitscope.core.model.RecordingAnalysis;

/** Verification Gate 2: real-workload JFR -> pathology findings + regression diff. */
class Gate2IntegrationTest {

    private static final String DEOPT_STORM = "dev.jitscope.workloads.DeoptStorm";
    private static final String INLINE_CLIFF = "dev.jitscope.workloads.InlineCliff";
    private static final String CODE_CACHE_CLIFF = "dev.jitscope.workloads.CodeCacheCliff";
    private static final String DIAG = "jit-diag.jfc";

    @TempDir
    Path tempDir;

    @Test
    void deoptStormTriggersDeoptStormOnExactMethod() throws Exception {
        var rec = WorkloadJfrRecorder.record(tempDir, DEOPT_STORM, List.of(), List.of(), DIAG);
        assertThat(rec.exitCode()).as(rec.stderr()).isZero();
        RecordingAnalysis a = RecordingAnalyzer.analyze(rec.jfr());

        var finding = a.findings().stream()
                .filter(f -> f.type() == PathologyType.DEOPT_STORM)
                .filter(f -> f.primaryMethod().methodName().equals("hot"))
                .filter(f -> f.primaryMethod().className().equals("dev.jitscope.workloads.DeoptStorm"))
                .findFirst();
        assertThat(finding).as("DEOPT_STORM on exact target method dev.jitscope.workloads.DeoptStorm.hot").isPresent();
        assertThat(finding.get().evidence()).isNotEmpty();
        assertThat(finding.get().recommendation()).isNotBlank();
        assertThat(finding.get().evidenceLevel()).isNotNull();
    }

    @Test
    void inlineCliffCliffTriggersInliningAllocationCliff() throws Exception {
        var rec = WorkloadJfrRecorder.record(tempDir, INLINE_CLIFF, List.of(), List.of("--mode=cliff"), DIAG);
        assertThat(rec.exitCode()).as(rec.stderr()).isZero();
        RecordingAnalysis a = RecordingAnalyzer.analyze(rec.jfr());

        var finding = a.findings().stream().filter(f -> f.type() == PathologyType.INLINING_ALLOCATION_CLIFF).findFirst();
        assertThat(finding).as("INLINING_ALLOCATION_CLIFF on cliff mode").isPresent();
        assertThat(finding.get().evidence()).isNotEmpty();
        assertThat(finding.get().evidence().toString()).contains("jdk.CompilerInlining");
    }

    @Test
    void codeCacheCliffEmitsCodeCacheSignalsAndPressureWhenPresent() throws Exception {
        var rec = WorkloadJfrRecorder.record(tempDir, CODE_CACHE_CLIFF,
                List.of("-XX:ReservedCodeCacheSize=4m"), List.of(), DIAG);
        assertThat(rec.exitCode()).as(rec.stderr()).isZero();
        RecordingAnalysis a = RecordingAnalyzer.analyze(rec.jfr());

        assertThat(a.codeCache().statistics()).isNotEmpty();
        boolean pressureSignal = a.codeCache().codeCacheFullCount() > 0 || a.codeCache().peakOccupancy() >= 0.85;
        var finding = a.findings().stream().filter(f -> f.type() == PathologyType.CODE_CACHE_PRESSURE).findFirst();
        if (pressureSignal) {
            assertThat(finding).as("CODE_CACHE_PRESSURE when real pressure signal present").isPresent();
        } else {
            assertThat(finding).as("no CODE_CACHE_PRESSURE without real signal").isEmpty();
        }
    }

    @Test
    void codeCachePressureDetectorTriggersOnRealSignalAndNotOtherwise() {
        var pressureCc = new dev.jitscope.core.model.CodeCacheInfo(
                List.of(new dev.jitscope.core.model.CodeCacheInfo.CodeCacheSnapshot(java.time.Instant.EPOCH, "CodeCache",
                        4_000_000, 4_000_000, 0, 1, 100, 1.0)),
                1, java.util.Optional.empty());
        assertThat(dev.jitscope.core.analysis.PathologyDetectors.codeCachePressure(emptyWithCache(pressureCc)))
                .anyMatch(f -> f.type() == PathologyType.CODE_CACHE_PRESSURE);
        var calmCc = new dev.jitscope.core.model.CodeCacheInfo(
                List.of(new dev.jitscope.core.model.CodeCacheInfo.CodeCacheSnapshot(java.time.Instant.EPOCH, "CodeCache",
                        4_000_000, 500_000, 3_500_000, 0, 50, 0.125)),
                0, java.util.Optional.empty());
        assertThat(dev.jitscope.core.analysis.PathologyDetectors.codeCachePressure(emptyWithCache(calmCc))).isEmpty();
    }

    private static RecordingAnalysis emptyWithCache(dev.jitscope.core.model.CodeCacheInfo cc) {
        var audit = new dev.jitscope.core.model.ConfigAudit(dev.jitscope.core.model.Completeness.COMPLETE,
                List.of(), java.util.Map.of(), java.util.Set.of(), List.of(), java.util.Map.of(), 0L);
        var meta = new dev.jitscope.core.model.RecordingMetadata("x", java.nio.file.Path.of("x"), "21", "",
                java.time.Instant.EPOCH, java.time.Instant.EPOCH, java.time.Duration.ZERO, audit);
        return new RecordingAnalysis(meta, List.of(), List.of(), List.of(), List.of(), 0, java.util.Optional.empty(),
                List.of(), List.of(), List.of(), cc);
    }

    @Test
    void diffDetectsRegressionBetweenInlineableAndCliff() throws Exception {
        var inlineable = WorkloadJfrRecorder.record(tempDir, INLINE_CLIFF, List.of(), List.of("--mode=inlineable"), DIAG);
        var cliff = WorkloadJfrRecorder.record(tempDir, INLINE_CLIFF, List.of(), List.of("--mode=cliff"), DIAG);
        assertThat(inlineable.exitCode()).isZero();
        assertThat(cliff.exitCode()).isZero();

        RecordingAnalysis base = RecordingAnalyzer.analyze(inlineable.jfr());
        RecordingAnalysis cand = RecordingAnalyzer.analyze(cliff.jfr());

        var diffs = DiffEngine.diff(base, cand, 0.0); // surface all shifts
        assertThat(diffs).isNotEmpty();
        // inlining regression on an exact method with evidence
        assertThat(diffs.stream().anyMatch(d -> d.category() == DiffCategory.INLINING_REGRESSION)).isTrue();
        assertThat(diffs.stream().anyMatch(d ->
                d.category() == DiffCategory.INLINING_REGRESSION
                        && d.method().className().contains("InlineCliff"))).isTrue();
        // hotness shift uses normalized share delta
        assertThat(diffs.stream().anyMatch(d -> d.category() == DiffCategory.HOTNESS_SHIFT)).isTrue();
    }

    @Test
    void diffAnnotatesJvmVersionDifference() {
        // synthetic minimal analyses
        RecordingAnalysis a = emptyAnalysis("JDK 21.0.5", "x.jfr");
        RecordingAnalysis b = emptyAnalysis("JDK 25.0.0", "y.jfr");
        var diffs = DiffEngine.diff(a, b);
        assertThat(diffs.stream().anyMatch(d -> d.category() == DiffCategory.JVM_VERSION_DIFFERENCE)).isTrue();
        assertThat(DiffEngine.jvmMajor("21.0.5")).isEqualTo(21);
        assertThat(DiffEngine.jvmMajor("25")).isEqualTo(25);
        assertThat(DiffEngine.jvmMajor("1.8.0_392")).isEqualTo(8);
    }

    private static RecordingAnalysis emptyAnalysis(String jvmVersion, String name) {
        var audit = new dev.jitscope.core.model.ConfigAudit(
                dev.jitscope.core.model.Completeness.COMPLETE, List.of(), java.util.Map.of(), java.util.Set.of(),
                List.of(), java.util.Map.of(), 0L);
        var meta = new dev.jitscope.core.model.RecordingMetadata(name, Path.of(name), jvmVersion, "",
                java.time.Instant.EPOCH, java.time.Instant.EPOCH, java.time.Duration.ZERO, audit);
        return new RecordingAnalysis(meta, List.of(), List.of(), List.of(), List.of(), 0,
                java.util.Optional.empty(), List.of(), List.of(), List.of(), dev.jitscope.core.model.CodeCacheInfo.EMPTY);
    }
}
