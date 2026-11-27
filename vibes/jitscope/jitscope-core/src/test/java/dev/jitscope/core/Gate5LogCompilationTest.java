package dev.jitscope.core;

import static org.assertj.core.api.Assertions.assertThat;

import java.io.ByteArrayInputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;
import java.util.concurrent.TimeUnit;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

import dev.jitscope.core.analysis.RecordingAnalyzer;
import dev.jitscope.core.ingest.LogCompilationParser;
import dev.jitscope.core.model.EvidenceLevel;
import dev.jitscope.core.model.LifecycleEventType;
import dev.jitscope.core.model.LogCompilationEvent;
import dev.jitscope.core.model.RecordingAnalysis;

/** Verification Gate 5 (Phase 5): LogCompilation streaming StAX + enrichment to OBSERVED. */
class Gate5LogCompilationTest {

    @TempDir
    Path tempDir;

    private static final String DEOPT_STORM = "dev.jitscope.workloads.DeoptStorm";
    private static final String DIAG = "jit-diag.jfc";

    @Test
    void staxParsesCoreEventsAndCorrelatesCompileId() throws Exception {
        String xml = """
                <compilation>
                <task compile_id='14' method='dev.jitscope.workloads.DeoptStorm hot (Ldev/jitscope/workloads/DeoptStorm$Work;J)J' bytes='20' level='4' stamp='0.027'>
                <inline_fail reason='callee is too large'/>
                <uncommon_trap reason='unstable_if' action='reinterpret' compile_id='16' compiler='c2' level='4' stamp='0.031'/>
                </task>
                <task_done success='1' nmsize='288' stamp='0.023'/>
                <make_not_entrant thread='28419' reason='not used' compile_id='14' compiler='c1' level='3' stamp='0.028'/>
                <parse method='1386' stamp='0.023'/>
                </compilation>""";
        var data = LogCompilationParser.parse(new ByteArrayInputStream(xml.getBytes(StandardCharsets.UTF_8)));

        assertThat(data.of(LogCompilationEvent.Type.TASK)).isNotEmpty();
        assertThat(data.of(LogCompilationEvent.Type.TASK).get(0).compileId()).isEqualTo(14);
        assertThat(data.of(LogCompilationEvent.Type.TASK).get(0).method().methodName()).isEqualTo("hot");
        // inline_fail inherits current task compile_id + method
        assertThat(data.of(LogCompilationEvent.Type.INLINE_FAIL)).hasSize(1);
        assertThat(data.of(LogCompilationEvent.Type.INLINE_FAIL).get(0).reason()).isEqualTo("callee is too large");
        assertThat(data.of(LogCompilationEvent.Type.INLINE_FAIL).get(0).compileId()).isEqualTo(14);
        // uncommon_trap preserved
        assertThat(data.of(LogCompilationEvent.Type.UNCOMMON_TRAP).get(0).reason()).isEqualTo("unstable_if");
        assertThat(data.of(LogCompilationEvent.Type.UNCOMMON_TRAP).get(0).action()).isEqualTo("reinterpret");
        // make_not_entrant preserved with compile_id
        assertThat(data.of(LogCompilationEvent.Type.MAKE_NOT_ENTRANT).get(0).compileId()).isEqualTo(14);
        assertThat(data.of(LogCompilationEvent.Type.TASK_DONE).get(0).success()).isTrue();
        // no silent swallowing
        assertThat(data.warnings()).isNotNull();
    }

    @Test
    void realJfrPlusLogCompilationEnrichesObserved() throws Exception {
        Path jfr = tempDir.resolve("d.jfr");
        Path log = tempDir.resolve("hotspot_pid.log");
        recordWithLogCompilation(DEOPT_STORM, jfr, log);

        // baseline: JFR-only must NOT claim made_not_entrant OBSERVED
        RecordingAnalysis jfrOnly = RecordingAnalyzer.analyze(jfr);
        assertThat(jfrOnly.lifecycles().stream().flatMap(l -> l.timeline().stream())
                .anyMatch(e -> e.type() == LifecycleEventType.MADE_NOT_ENTRANT && e.level() == EvidenceLevel.OBSERVED))
                .as("made_not_entrant NOT OBSERVED from JFR alone").isFalse();

        // enriched: LogCompilation backs made_not_entrant -> OBSERVED
        RecordingAnalysis enriched = RecordingAnalyzer.analyze(jfr, log);

        // log produced real events
        var logData = LogCompilationParser.parse(log);
        assertThat(logData.of(LogCompilationEvent.Type.MAKE_NOT_ENTRANT)).isNotEmpty();
        assertThat(logData.of(LogCompilationEvent.Type.INLINE_FAIL)).isNotEmpty();

        assertThat(enriched.lifecycles().stream().flatMap(l -> l.timeline().stream())
                .anyMatch(e -> e.type() == LifecycleEventType.MADE_NOT_ENTRANT && e.level() == EvidenceLevel.OBSERVED))
                .as("made_not_entrant OBSERVED with LogCompilation").isTrue();
        // inline-fail reason upgraded to OBSERVED
        assertThat(enriched.lifecycles().stream().flatMap(l -> l.timeline().stream())
                .anyMatch(e -> e.type() == LifecycleEventType.INLINE_FAILED && e.level() == EvidenceLevel.OBSERVED
                        && e.evidence() != null && e.evidence().eventType().startsWith("LogCompilation")))
                .isTrue();
        // made_non_compilable NOT claimed (no such event in this log)
        assertThat(enriched.lifecycles().stream().flatMap(l -> l.timeline().stream())
                .anyMatch(e -> e.type() == LifecycleEventType.MADE_NON_COMPILABLE && e.level() == EvidenceLevel.OBSERVED))
                .isFalse();
    }

    // ---- real capture ----
    private static void recordWithLogCompilation(String mainClass, Path jfr, Path log) throws Exception {
        Path jfc = WorkloadJfrRecorder.resolveJfc(DIAG);
        String javaBin = Path.of(System.getProperty("java.home"), "bin", "java").toString();
        List<String> cmd = new java.util.ArrayList<>(List.of(
                javaBin,
                "-XX:+UnlockDiagnosticVMOptions",
                "-XX:+LogCompilation",
                "-XX:LogFile=" + log.toAbsolutePath(),
                "-XX:StartFlightRecording=settings=" + jfc + ",filename=" + jfr.toAbsolutePath(),
                "-cp", System.getProperty("java.class.path"),
                mainClass));
        Process p = new ProcessBuilder(cmd).redirectErrorStream(true).start();
        String output = new String(p.getInputStream().readAllBytes());
        assertThat(p.waitFor(120, TimeUnit.SECONDS)).as(output).isTrue();
        assertThat(p.exitValue()).as(output).isZero();
        assertThat(jfr).exists();
        assertThat(log).exists();
    }
}
