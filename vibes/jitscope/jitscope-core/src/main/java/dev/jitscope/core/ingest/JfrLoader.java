package dev.jitscope.core.ingest;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.time.Duration;
import java.time.Instant;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import jdk.jfr.consumer.RecordedEvent;
import jdk.jfr.consumer.RecordedFrame;
import jdk.jfr.consumer.RecordedMethod;
import jdk.jfr.consumer.RecordingFile;
import jdk.jfr.EventType;

import dev.jitscope.core.model.CompilationRecord;
import dev.jitscope.core.model.DeoptimizationRecord;
import dev.jitscope.core.model.FrameType;
import dev.jitscope.core.model.InliningRecord;
import dev.jitscope.core.model.MethodKey;

/**
 * Offline JFR loader built only on {@code jdk.jfr.consumer}. Read-only. Malformed/missing fields
 * are recorded in {@link RecordingData#warnings}; parsing exceptions are never silently swallowed.
 */
public final class JfrLoader {

    private JfrLoader() {}

    public static RecordingData load(Path jfrPath) throws IOException {
        if (jfrPath == null) {
            throw new IOException("jfrPath is null");
        }
        Path abs = jfrPath.toAbsolutePath().normalize();
        if (!Files.exists(abs)) {
            throw new IOException("recording not found: " + abs);
        }

        RecordingData data = new RecordingData();
        data.sourcePath = abs;
        data.recordingId = abs.getFileName().toString() + "@" + abs.hashCode();

        Instant minStart = null;
        Instant maxEnd = null;

        try (RecordingFile rf = new RecordingFile(abs)) {
            Map<Long, String> eventIdToName = readEventIdToName(rf);

            while (rf.hasMoreEvents()) {
                RecordedEvent e;
                try {
                    e = rf.readEvent();
                } catch (IOException | RuntimeException ex) {
                    data.warnings.add("failed to read event: " + ex.getMessage());
                    continue;
                }
                String type = e.getEventType() == null ? "" : e.getEventType().getName();
                data.eventCounts.merge(type, 1L, Long::sum);

                Instant start = safeInstant(e);
                if (start != null) {
                    if (minStart == null || start.isBefore(minStart)) {
                        minStart = start;
                    }
                    Instant end = start.plus(safeDuration(e));
                    if (maxEnd == null || end.isAfter(maxEnd)) {
                        maxEnd = end;
                    }
                }

                try {
                    dispatch(data, e, type, eventIdToName);
                } catch (RuntimeException ex) {
                    data.warnings.add("malformed " + type + " event ignored: " + ex.getMessage());
                }
            }
        }

        data.startTime = minStart == null ? Instant.EPOCH : minStart;
        data.endTime = maxEnd == null ? data.startTime : maxEnd;
        data.duration = Duration.between(data.startTime, data.endTime);
        if (data.duration.isNegative()) {
            data.duration = Duration.ZERO;
        }
        return data;
    }

    private static Map<Long, String> readEventIdToName(RecordingFile rf) {
        Map<Long, String> map = new HashMap<>();
        try {
            List<EventType> types = rf.readEventTypes();
            for (EventType et : types) {
                map.put(et.getId(), et.getName());
            }
        } catch (IOException | RuntimeException ex) {
            // best effort; ActiveSetting ids then can't be resolved to event names
        }
        return map;
    }

    private static void dispatch(RecordingData data, RecordedEvent e, String type, Map<Long, String> eventIdToName) {
        switch (type) {
            case "jdk.Compilation" -> readCompilation(data, e);
            case "jdk.CompilationFailure" -> readCompilationFailure(data, e);
            case "jdk.CompilerInlining" -> readInlining(data, e);
            case "jdk.Deoptimization" -> readDeoptimization(data, e);
            case "jdk.CompilerPhase" -> readCompilerPhase(data, e);
            case "jdk.ExecutionSample" -> readExecutionSample(data, e);
            case "jdk.CPUTimeSample" -> readCpuTimeSample(data, e);
            case "jdk.CPUTimeSamplesLost" -> data.cpuTimeSamplesLost += JfrFields.getLong(e, "lostSamples", 1L);
            case "jdk.ObjectAllocationSample" -> readObjectAllocationSample(data, e);
            case "jdk.ObjectAllocationInNewTLAB" -> readTlabAllocation(data, e, true);
            case "jdk.ObjectAllocationOutsideTLAB" -> readTlabAllocation(data, e, false);
            case "jdk.ActiveSetting" -> readActiveSetting(data, e, eventIdToName);
            case "jdk.JVMInformation" -> readJvmInfo(data, e);
            default -> { /* ignore other event types */ }
        }
    }

    private static void readCompilation(RecordingData data, RecordedEvent e) {
        int compileId = JfrFields.getInt(e, "compileId", -1);
        MethodKey method = JfrFields.methodKey(getRecordedMethod(e, "method"));
        int level = JfrFields.getInt(e, "compileLevel", -1);
        boolean osr = JfrFields.getBoolean(e, "isOsr", JfrFields.getBoolean(e, "osr", false));
        boolean succeeded = JfrFields.compilationSucceeded(e);
        Instant start = safeInstant(e);
        Duration dur = safeDuration(e);
        int codeSize = JfrFields.getInt(e, "codeSize", 0);
        int inlinedBytes = JfrFields.getInt(e, "inlinedBytes", 0);
        String failureMessage = JfrFields.getString(e, "failureMessage", null);
        if (compileId < 0) {
            data.warnings.add("jdk.Compilation missing/invalid compileId");
        }
        data.compilations.add(new CompilationRecord(compileId, method, level, osr, succeeded,
                start == null ? Instant.EPOCH : start, dur, codeSize, inlinedBytes, failureMessage));
    }

    private static void readCompilationFailure(RecordingData data, RecordedEvent e) {
        int compileId = JfrFields.getInt(e, "compileId", -1);
        String compiler = JfrFields.getString(e, "compiler", "");
        String message = JfrFields.getString(e, "message", "");
        Instant ts = safeInstant(e);
        data.compilationFailures.add(new RecordingData.CompilationFailure(
                compileId, compiler, message, ts == null ? Instant.EPOCH : ts));
    }

    private static void readInlining(RecordingData data, RecordedEvent e) {
        int compileId = JfrFields.getInt(e, "compileId", -1);
        MethodKey caller = JfrFields.callerKey(e);
        MethodKey callee = JfrFields.calleeKey(e);
        int bci = JfrFields.getInt(e, "bci", -1);
        boolean succeeded = JfrFields.getBoolean(e, "succeeded", false);
        String message = JfrFields.getString(e, "message", "");
        int bytes = JfrFields.getInt(e, "bytes", 0);
        data.inlining.add(new InliningRecord(compileId, caller, callee, bci, succeeded, message, bytes));
    }

    private static void readDeoptimization(RecordingData data, RecordedEvent e) {
        int compileId = JfrFields.getInt(e, "compileId", -1);
        MethodKey method = JfrFields.methodKey(getRecordedMethod(e, "method"));
        int bci = JfrFields.getInt(e, "bci", -1);
        int lineNumber = JfrFields.getInt(e, "lineNumber", -1);
        String reason = JfrFields.getString(e, "reason", "");
        String action = JfrFields.getString(e, "action", "");
        String compiler = JfrFields.getString(e, "compiler", "");
        Instant ts = safeInstant(e);
        data.deoptimizations.add(new DeoptimizationRecord(compileId, method, bci, lineNumber, reason, action,
                compiler, ts == null ? Instant.EPOCH : ts));
    }

    private static void readCompilerPhase(RecordingData data, RecordedEvent e) {
        int compileId = JfrFields.getInt(e, "compileId", -1);
        String phase = JfrFields.getString(e, "phase", "");
        Duration dur = safeDuration(e);
        Instant ts = safeInstant(e);
        data.compilerPhases.add(new RecordingData.CompilerPhase(compileId, phase, dur, ts == null ? Instant.EPOCH : ts));
    }

    private static void readExecutionSample(RecordingData data, RecordedEvent e) {
        Instant ts = safeInstant(e);
        String thread = threadName(e);
        List<RecordingData.Frame> frames = readFrames(e, data, "jdk.ExecutionSample");
        data.executionSamples.add(new RecordingData.Sample(ts == null ? Instant.EPOCH : ts, thread, frames));
    }

    private static void readCpuTimeSample(RecordingData data, RecordedEvent e) {
        Instant ts = safeInstant(e);
        String thread = threadName(e);
        List<RecordingData.Frame> frames = readFrames(e, data, "jdk.CPUTimeSample");
        boolean failed = JfrFields.getBoolean(e, "failed", frames.isEmpty());
        boolean biased = JfrFields.getBoolean(e, "biased", false);
        data.cpuTimeSamples.add(new RecordingData.CpuSample(ts == null ? Instant.EPOCH : ts, thread, frames, failed, biased));
    }

    private static void readObjectAllocationSample(RecordingData data, RecordedEvent e) {
        Instant ts = safeInstant(e);
        String obj = JfrFields.objectClassName(e);
        long weight = JfrFields.getLong(e, "weight", JfrFields.getLong(e, "allocationSize", 0L));
        String thread = threadName(e);
        List<RecordingData.Frame> frames = readFrames(e, data, "jdk.ObjectAllocationSample");
        data.allocSamples.add(new RecordingData.AllocSample(ts == null ? Instant.EPOCH : ts, obj, weight, thread, frames, false, false));
    }

    private static void readTlabAllocation(RecordingData data, RecordedEvent e, boolean inNewTlab) {
        Instant ts = safeInstant(e);
        String obj = JfrFields.objectClassName(e);
        long size = JfrFields.getLong(e, "allocationSize", 0L);
        String thread = threadName(e);
        List<RecordingData.Frame> frames = readFrames(e, data, "jdk.ObjectAllocation*");
        data.allocSamples.add(new RecordingData.AllocSample(ts == null ? Instant.EPOCH : ts, obj, size, thread, frames,
                inNewTlab, !inNewTlab));
    }

    private static void readActiveSetting(RecordingData data, RecordedEvent e, Map<Long, String> eventIdToName) {
        long id = JfrFields.getLong(e, "id", -1L);
        String name = JfrFields.getString(e, "name", "");
        String value = JfrFields.getString(e, "value", "");
        String eventName = eventIdToName.getOrDefault(id, "event#" + id);
        data.activeSettings.computeIfAbsent(eventName, k -> new java.util.TreeMap<>()).put(name, value);
    }

    private static void readJvmInfo(RecordingData data, RecordedEvent e) {
        String v = JfrFields.getString(e, "jvmVersion", null);
        if (v != null && !v.isBlank()) {
            data.jvmVersion = v;
        }
        String args = JfrFields.getString(e, "jvmArguments", null);
        if (args != null && !args.isBlank()) {
            data.jvmArguments = args;
        }
    }

    private static List<RecordingData.Frame> readFrames(RecordedEvent e, RecordingData data, String type) {
        List<RecordingData.Frame> frames = new ArrayList<>();
        try {
            if (!e.hasField("stackTrace")) {
                return frames;
            }
            var stack = e.getStackTrace();
            if (stack == null) {
                return frames;
            }
            for (RecordedFrame f : stack.getFrames()) {
                if (f == null) {
                    continue;
                }
                RecordedMethod m = f.getMethod();
                String className = "";
                String mName = "";
                String desc = "";
                if (m != null) {
                    try {
                        if (m.getType() != null && m.getType().getName() != null) {
                            className = JfrFields.normalizeClass(m.getType().getName());
                        }
                        mName = m.getName() == null ? "" : m.getName();
                        desc = m.getDescriptor() == null ? "" : m.getDescriptor();
                    } catch (RuntimeException ex) {
                        data.warnings.add(type + " frame method unreadable: " + ex.getMessage());
                    }
                }
                String frameType = "";
                try {
                    // Use RecordedFrame.getType() directly.
                    Object t = f.getType();
                    frameType = t == null ? FrameType.UNKNOWN.label() : t.toString();
                } catch (RuntimeException ex) {
                    frameType = FrameType.UNKNOWN.label();
                }
                int line = -1;
                int bci = -1;
                try {
                    line = f.getLineNumber();
                } catch (RuntimeException ignored) { }
                try {
                    bci = f.getBytecodeIndex();
                } catch (RuntimeException ignored) { }
                frames.add(new RecordingData.Frame(className, mName, desc, frameType, line, bci));
            }
        } catch (RuntimeException ex) {
            data.warnings.add(type + " stack trace unreadable: " + ex.getMessage());
        }
        return frames;
    }

    private static RecordedMethod getRecordedMethod(RecordedEvent e, String field) {
        try {
            if (e.hasField(field)) {
                Object v = e.getValue(field);
                if (v instanceof RecordedMethod rm) {
                    return rm;
                }
            }
        } catch (RuntimeException ex) {
            // fall through
        }
        return null;
    }

    private static String threadName(RecordedEvent e) {
        try {
            Object t;
            if (e.hasField("sampledThread")) {
                t = e.getValue("sampledThread");
            } else if (e.hasField("eventThread")) {
                t = e.getValue("eventThread");
            } else if (e.hasField("thread")) {
                t = e.getValue("thread");
            } else {
                return "";
            }
            if (t == null) {
                return "";
            }
            // RecordedThread toString; prefer name if available
            Object name = null;
            if (t instanceof jdk.jfr.consumer.RecordedThread rt) {
                name = rt.getJavaName() != null ? rt.getJavaName() : rt.getOSName();
            }
            return name == null ? t.toString() : name.toString();
        } catch (RuntimeException ex) {
            return "";
        }
    }

    private static Instant safeInstant(RecordedEvent e) {
        try {
            return e.getStartTime();
        } catch (RuntimeException ex) {
            return null;
        }
    }

    private static Duration safeDuration(RecordedEvent e) {
        try {
            if (e.hasField("duration")) {
                Duration d = e.getDuration("duration");
                return d == null ? Duration.ZERO : d;
            }
        } catch (RuntimeException ex) {
            // fall through
        }
        return Duration.ZERO;
    }
}
