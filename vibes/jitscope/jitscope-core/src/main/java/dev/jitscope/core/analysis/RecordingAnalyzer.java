package dev.jitscope.core.analysis;

import java.io.IOException;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

import dev.jitscope.core.ingest.JfrLoader;
import dev.jitscope.core.ingest.RecordingData;
import dev.jitscope.core.model.ConfigAudit;
import dev.jitscope.core.model.RecordingAnalysis;
import dev.jitscope.core.model.RecordingMetadata;

/** Orchestrates ingest + config audit + lifecycle building into a {@link RecordingAnalysis}. */
public final class RecordingAnalyzer {

    private RecordingAnalyzer() {}

    public static RecordingAnalysis analyze(Path jfrPath) throws IOException {
        return analyze(jfrPath, null);
    }

    private static List<dev.jitscope.core.model.Sample> toModelSamples(List<RecordingData.Sample> in) {
        List<dev.jitscope.core.model.Sample> out = new ArrayList<>();
        for (RecordingData.Sample s : in) {
            List<dev.jitscope.core.model.SampleFrame> frames = new ArrayList<>();
            for (RecordingData.Frame f : s.frames()) {
                frames.add(new dev.jitscope.core.model.SampleFrame(
                        new dev.jitscope.core.model.MethodKey(f.className(), f.methodName(), f.descriptor()),
                        f.type(), f.lineNumber(), f.bytecodeIndex()));
            }
            out.add(new dev.jitscope.core.model.Sample(s.timestamp(), s.thread(), frames));
        }
        return out;
    }

    public static RecordingAnalysis analyze(Path jfrPath, Path logCompilationPath) throws IOException {
        RecordingData data = JfrLoader.load(jfrPath);
        ConfigAudit audit = ConfigAuditor.audit(data);
        LifecycleBuilder.LifecycleResult result = LifecycleBuilder.build(data);

        RecordingMetadata metadata = new RecordingMetadata(
                data.recordingId,
                data.sourcePath,
                data.jvmVersion,
                data.jvmArguments,
                data.startTime,
                data.endTime,
                data.duration,
                audit);

        List<String> warnings = new ArrayList<>(data.warnings);
        warnings.addAll(audit.warnings());

        RecordingAnalysis base = new RecordingAnalysis(
                metadata,
                data.compilations,
                data.deoptimizations,
                data.inlining,
                result.lifecycles(),
                result.executionSampleTotal(),
                result.cpuTimeSamples(),
                warnings,
                List.of(),
                toModelSamples(data.executionSamples),
                new dev.jitscope.core.model.CodeCacheInfo(
                        data.codeCacheStatistics,
                        data.codeCacheFullCount,
                        java.util.Optional.ofNullable(data.codeCacheConfiguration)));

        List<dev.jitscope.core.model.PathologyFinding> findings = PathologyDetectors.detectAll(base);
        return new RecordingAnalysis(
                base.metadata(), base.compilations(), base.deoptimizations(), base.inlining(), base.lifecycles(),
                base.executionSampleTotal(), base.cpuTimeSamples(), base.warnings(), findings,
                base.executionSamples(), base.codeCache());
    }
}
