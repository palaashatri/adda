package dev.jitscope.core.ingest;

import java.nio.file.Path;
import java.time.Duration;
import java.time.Instant;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import dev.jitscope.core.model.CompilationRecord;
import dev.jitscope.core.model.DeoptimizationRecord;
import dev.jitscope.core.model.InliningRecord;

/** Raw ingested recording. Sample data kept in separate lists; never merged. */
public final class RecordingData {

    public String recordingId;
    public Path sourcePath;
    public String jvmVersion = "";
    public String jvmArguments = "";
    public Instant startTime;
    public Instant endTime;
    public Duration duration = Duration.ZERO;

    public final List<CompilationRecord> compilations = new ArrayList<>();
    public final List<DeoptimizationRecord> deoptimizations = new ArrayList<>();
    public final List<InliningRecord> inlining = new ArrayList<>();
    /** jdk.CompilationFailure keyed by compileId. */
    public final List<CompilationFailure> compilationFailures = new ArrayList<>();

    /** Sampled execution samples (jdk.ExecutionSample). */
    public final List<Sample> executionSamples = new ArrayList<>();
    /** JDK25+ Linux experimental jdk.CPUTimeSample (separate dataset). */
    public final List<CpuSample> cpuTimeSamples = new ArrayList<>();
    public long cpuTimeSamplesLost;
    /** Allocation samples. */
    public final List<AllocSample> allocSamples = new ArrayList<>();

    /** jdk.ActiveSetting: eventName -> (setting -> value). */
    public final Map<String, Map<String, String>> activeSettings = new TreeMap<>();
    /** Event type name -> count. */
    public final Map<String, Long> eventCounts = new TreeMap<>();

    /** Non-fatal parsing warnings (malformed/missing fields); never silently swallowed. */
    public final List<String> warnings = new ArrayList<>();

    /** Raw compilation phase events keyed by compileId. */
    public final List<CompilerPhase> compilerPhases = new ArrayList<>();

    /** jdk.CompilationFailure. */
    public record CompilationFailure(int compileId, String compiler, String message, Instant timestamp) {}

    /** jdk.CompilerPhase. */
    public record CompilerPhase(int compileId, String phase, Duration duration, Instant timestamp) {}

    /** A sampled stack. */
    public record Sample(Instant timestamp, String thread, List<Frame> frames) {
        public Sample {
            frames = frames == null ? List.of() : List.copyOf(frames);
        }
    }

    /** CPUTimeSample with confidence fields. */
    public record CpuSample(Instant timestamp, String thread, List<Frame> frames, boolean failed, boolean biased) {
        public CpuSample {
            frames = frames == null ? List.of() : List.copyOf(frames);
        }
    }

    /** A frame with method + execution type (from RecordedFrame.getType()). */
    public record Frame(String className, String methodName, String descriptor, String type, int lineNumber, int bytecodeIndex) {}

    /** Allocation sample. */
    public record AllocSample(Instant timestamp, String objectClass, long weightBytes, String thread, List<Frame> frames, boolean inNewTlab, boolean outsideTlab) {
        public AllocSample {
            frames = frames == null ? List.of() : List.copyOf(frames);
        }
    }
}
