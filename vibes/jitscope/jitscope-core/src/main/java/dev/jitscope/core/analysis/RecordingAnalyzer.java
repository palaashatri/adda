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

        return new RecordingAnalysis(
                metadata,
                data.compilations,
                data.deoptimizations,
                data.inlining,
                result.lifecycles(),
                result.executionSampleTotal(),
                result.cpuTimeSamples(),
                warnings,
                List.of());
    }
}
