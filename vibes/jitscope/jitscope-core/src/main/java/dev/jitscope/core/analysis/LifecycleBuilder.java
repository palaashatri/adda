package dev.jitscope.core.analysis;

import java.time.Instant;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.TreeMap;
import java.util.stream.Collectors;

import dev.jitscope.core.ingest.RecordingData;
import dev.jitscope.core.model.AllocationStats;
import dev.jitscope.core.model.CompilationRecord;
import dev.jitscope.core.model.CpuTimeSamples;
import dev.jitscope.core.model.DeoptimizationRecord;
import dev.jitscope.core.model.Evidence;
import dev.jitscope.core.model.EvidenceLevel;
import dev.jitscope.core.model.FrameStats;
import dev.jitscope.core.model.FrameType;
import dev.jitscope.core.model.InliningRecord;
import dev.jitscope.core.model.LifecycleEvent;
import dev.jitscope.core.model.LifecycleEventType;
import dev.jitscope.core.model.MethodKey;
import dev.jitscope.core.model.MethodLifecycle;

/**
 * Joins {@code jdk.Compilation}, {@code jdk.CompilationFailure}, {@code jdk.CompilerInlining},
 * {@code jdk.Deoptimization}, {@code jdk.CompilerPhase} on {@code (recordingId, compileId)}; overlays
 * {@code jdk.ExecutionSample}, {@code jdk.CPUTimeSample} (separate), {@code jdk.ObjectAllocationSample}
 * (fallback TLAB) per {@link MethodKey}. Confidence-aware; no fabricated certainty.
 */
public final class LifecycleBuilder {

    private LifecycleBuilder() {}

    public record LifecycleResult(
            List<MethodLifecycle> lifecycles,
            long executionSampleTotal,
            Optional<CpuTimeSamples> cpuTimeSamples) {}

    public static LifecycleResult build(RecordingData data) {
        // ---- compileId -> compilation join ----
        Map<Integer, CompilationRecord> byCompileId = new HashMap<>();
        for (CompilationRecord c : data.compilations) {
            byCompileId.put(c.compileId(), c);
        }

        // ---- methods universe ----
        Set<MethodKey> methods = data.compilations.stream().map(CompilationRecord::method)
                .collect(Collectors.toCollection(java.util.LinkedHashSet::new));
        for (InliningRecord i : data.inlining) {
            methods.add(i.caller());
            methods.add(i.callee());
        }
        for (DeoptimizationRecord d : data.deoptimizations) {
            methods.add(d.method());
        }
        // include methods that appear only in samples/allocations
        for (RecordingData.Sample s : data.executionSamples) {
            for (RecordingData.Frame f : s.frames()) {
                methods.add(frameKey(f));
            }
        }
        for (RecordingData.CpuSample s : data.cpuTimeSamples) {
            for (RecordingData.Frame f : s.frames()) {
                methods.add(frameKey(f));
            }
        }
        for (RecordingData.AllocSample a : data.allocSamples) {
            for (RecordingData.Frame f : a.frames()) {
                methods.add(frameKey(f));
            }
        }

        // ---- execution samples ----
        long execTotal = data.executionSamples.size();
        Map<MethodKey, long[]> execCounts = new HashMap<>(); // [total, self]
        Map<MethodKey, Map<FrameType, Long>> execTypes = new HashMap<>();
        for (RecordingData.Sample s : data.executionSamples) {
            accumulateSample(s.frames(), execCounts, execTypes);
        }

        // ---- CPU time samples (separate) ----
        long cpuTotal = data.cpuTimeSamples.size();
        long cpuFailed = 0;
        long cpuBiased = 0;
        Map<MethodKey, long[]> cpuCounts = new HashMap<>();
        Map<MethodKey, Map<FrameType, Long>> cpuTypes = new HashMap<>();
        for (RecordingData.CpuSample s : data.cpuTimeSamples) {
            if (s.failed()) {
                cpuFailed++;
            }
            if (s.biased()) {
                cpuBiased++;
            }
            if (!s.failed()) {
                accumulateSample(s.frames(), cpuCounts, cpuTypes);
            }
        }
        long cpuValid = cpuTotal - cpuFailed;
        Optional<CpuTimeSamples> cpuTimeSamples;
        if (data.cpuTimeSamples.isEmpty() && data.cpuTimeSamplesLost == 0) {
            cpuTimeSamples = Optional.empty();
        } else {
            boolean downgraded = cpuFailed > 0 || data.cpuTimeSamplesLost > 0;
            FrameStats stats = buildFrameStats(cpuCounts, cpuTypes, cpuValid, null);
            // compute shares per-method not needed for aggregate; fill aggregate shares
            cpuTimeSamples = Optional.of(new CpuTimeSamples(stats, cpuTotal, cpuFailed, cpuBiased,
                    data.cpuTimeSamplesLost, downgraded));
        }

        // ---- allocations ----
        // Prefer jdk.ObjectAllocationSample; fall back to TLAB events if no ObjectAllocationSample.
        boolean hasObjectAllocationSample = data.allocSamples.stream().anyMatch(a -> !a.inNewTlab() && !a.outsideTlab());
        List<RecordingData.AllocSample> allocs = data.allocSamples.stream()
                .filter(a -> hasObjectAllocationSample ? (!a.inNewTlab() && !a.outsideTlab()) : true)
                .collect(Collectors.toList());
        long allocTotal = allocs.size();
        // Per-method allocation stats. Count any-frame for total; top-frame (self) for self.
        Map<MethodKey, long[]> allocCounts = new HashMap<>(); // [total, self]
        Map<MethodKey, Long> allocWeight = new HashMap<>();
        Map<MethodKey, Map<String, Long>> allocByClass = new HashMap<>();
        for (RecordingData.AllocSample a : allocs) {
            java.util.Set<MethodKey> seen = new java.util.HashSet<>();
            boolean firstFrame = true;
            for (RecordingData.Frame f : a.frames()) {
                MethodKey key = frameKey(f);
                long[] c = allocCounts.computeIfAbsent(key, k -> new long[2]);
                if (seen.add(key)) {
                    c[0]++;
                }
                if (firstFrame) {
                    c[1]++;
                }
                firstFrame = false;
            }
            if (!a.frames().isEmpty()) {
                MethodKey self = frameKey(a.frames().get(0));
                allocWeight.merge(self, a.weightBytes(), Long::sum);
                allocByClass.computeIfAbsent(self, k -> new TreeMap<>())
                        .merge(a.objectClass() == null ? "" : a.objectClass(), a.weightBytes(), Long::sum);
            }
        }

        // ---- per-method timelines ----
        List<MethodLifecycle> out = new ArrayList<>();
        for (MethodKey m : methods) {
            List<LifecycleEvent> timeline = new ArrayList<>();

            for (CompilationRecord c : data.compilations) {
                if (!c.method().equals(m)) {
                    continue;
                }
                Map<String, String> attrs = new LinkedHashMap<>();
                attrs.put("compileId", Integer.toString(c.compileId()));
                attrs.put("compileLevel", Integer.toString(c.compileLevel()));
                attrs.put("osr", Boolean.toString(c.osr()));
                attrs.put("codeSizeBytes", Integer.toString(c.codeSizeBytes()));
                attrs.put("inlinedBytes", Integer.toString(c.inlinedBytes()));
                timeline.add(new LifecycleEvent(c.startTime(), LifecycleEventType.COMPILATION_STARTED,
                        EvidenceLevel.OBSERVED, c.compileId(), c.compileLevel(), "compilation started",
                        new Evidence("jdk.Compilation", "start " + c.method().fqn(), attrs)));
                Instant end = c.startTime().plus(c.duration());
                if (c.succeeded()) {
                    timeline.add(new LifecycleEvent(end, LifecycleEventType.COMPILATION_FINISHED,
                            EvidenceLevel.OBSERVED, c.compileId(), c.compileLevel(),
                            "compiled at level " + c.compileLevel(),
                            new Evidence("jdk.Compilation", "succeded=true", attrs)));
                } else {
                    timeline.add(new LifecycleEvent(end, LifecycleEventType.COMPILATION_FAILED,
                            EvidenceLevel.OBSERVED, c.compileId(), c.compileLevel(),
                            c.failureMessage() == null ? "compilation failed" : c.failureMessage(),
                            new Evidence("jdk.Compilation", "succeded=false", attrs)));
                }
            }

            for (RecordingData.CompilationFailure f : data.compilationFailures) {
                CompilationRecord c = byCompileId.get(f.compileId());
                MethodKey target = c == null ? m : c.method();
                if (!target.equals(m)) {
                    continue;
                }
                timeline.add(new LifecycleEvent(f.timestamp(), LifecycleEventType.COMPILATION_FAILED,
                        EvidenceLevel.OBSERVED, f.compileId(), c == null ? null : c.compileLevel(),
                        f.message(),
                        Evidence.of("jdk.CompilationFailure", f.message())));
            }

            for (InliningRecord i : data.inlining) {
                MethodKey anchor = i.caller();
                if (!anchor.equals(m) && !i.callee().equals(m)) {
                    continue;
                }
                CompilationRecord c = byCompileId.get(i.compileId());
                LifecycleEventType t = i.succeeded() ? LifecycleEventType.INLINED : LifecycleEventType.INLINE_FAILED;
                // "no inlining failure" claims require CompilerInlining/LogCompilation; absence != failure.
                Map<String, String> attrs = new LinkedHashMap<>();
                attrs.put("compileId", Integer.toString(i.compileId()));
                attrs.put("caller", i.caller().fqn());
                attrs.put("callee", i.callee().fqn());
                attrs.put("bci", Integer.toString(i.bci()));
                attrs.put("bytes", Integer.toString(i.bytes()));
                timeline.add(new LifecycleEvent(c == null ? Instant.EPOCH : c.startTime(), t, EvidenceLevel.OBSERVED,
                        i.compileId(), c == null ? null : c.compileLevel(),
                        (i.succeeded() ? "inlined " : "not inlined ") + i.callee().fqn()
                                + (i.message() == null || i.message().isBlank() ? "" : " (" + i.message() + ")"),
                        new Evidence("jdk.CompilerInlining", i.message(), attrs)));
            }

            for (DeoptimizationRecord d : data.deoptimizations) {
                if (!d.method().equals(m)) {
                    continue;
                }
                CompilationRecord c = byCompileId.get(d.compileId());
                Map<String, String> attrs = new LinkedHashMap<>();
                attrs.put("compileId", Integer.toString(d.compileId()));
                attrs.put("compiler", d.compiler());
                attrs.put("bci", Integer.toString(d.bci()));
                attrs.put("lineNumber", Integer.toString(d.lineNumber()));
                attrs.put("reason", d.reason());
                attrs.put("action", d.action());
                attrs.put("compileLevel", c == null ? "unknown" : Integer.toString(c.compileLevel()));
                // NOTE: made_not_entrant/made_non_compilable are NOT claimed OBSERVED from JFR alone.
                timeline.add(new LifecycleEvent(d.timestamp(), LifecycleEventType.DEOPTIMIZATION, EvidenceLevel.OBSERVED,
                        d.compileId(), c == null ? null : c.compileLevel(),
                        "deopt reason=" + d.reason() + " action=" + d.action(),
                        new Evidence("jdk.Deoptimization", d.reason() + "/" + d.action(), attrs)));
            }

            for (RecordingData.CompilerPhase p : data.compilerPhases) {
                CompilationRecord c = byCompileId.get(p.compileId());
                if (c == null || !c.method().equals(m)) {
                    continue;
                }
                timeline.add(new LifecycleEvent(p.timestamp(), LifecycleEventType.COMPILER_PHASE, EvidenceLevel.OBSERVED,
                        p.compileId(), c.compileLevel(), p.phase(),
                        Evidence.of("jdk.CompilerPhase", p.phase())));
            }

            timeline.sort(Comparator.comparing(LifecycleEvent::timestamp)
                    .thenComparing(ev -> ev.type().ordinal()));

            int highest = data.compilations.stream()
                    .filter(c -> c.method().equals(m) && c.succeeded())
                    .mapToInt(CompilationRecord::compileLevel).max().orElse(-1);
            int compilationCount = (int) data.compilations.stream().filter(c -> c.method().equals(m)).count();
            int deoptCount = (int) data.deoptimizations.stream().filter(d -> d.method().equals(m)).count();

            long[] ec = execCounts.getOrDefault(m, new long[2]);
            FrameStats execStats = new FrameStats(ec[0], ec[1], execTypes.getOrDefault(m, Map.of()),
                    execTotal == 0 ? 0.0 : (double) ec[1] / execTotal, execTotal == 0 ? 0.0 : (double) ec[0] / execTotal);

            Optional<FrameStats> cpuStats = Optional.empty();
            if (cpuTimeSamples.isPresent()) {
                long[] cc = cpuCounts.getOrDefault(m, new long[2]);
                cpuStats = Optional.of(new FrameStats(cc[0], cc[1], cpuTypes.getOrDefault(m, Map.of()),
                        cpuValid == 0 ? 0.0 : (double) cc[1] / cpuValid, cpuValid == 0 ? 0.0 : (double) cc[0] / cpuValid));
            }

            long[] ac = allocCounts.getOrDefault(m, new long[2]);
            AllocationStats allocStats = new AllocationStats(ac[0], allocWeight.getOrDefault(m, 0L),
                    allocTotal == 0 ? 0.0 : (double) ac[1] / allocTotal, allocTotal == 0 ? 0.0 : (double) ac[0] / allocTotal,
                    allocByClass.getOrDefault(m, Map.of()));

            EvidenceLevel conf = compilationCount > 0 ? EvidenceLevel.OBSERVED
                    : (!timeline.isEmpty() ? EvidenceLevel.INFERRED : EvidenceLevel.UNKNOWN);

            out.add(new MethodLifecycle(m, timeline, highest, compilationCount, deoptCount,
                    execStats, cpuStats, allocStats, conf));
        }

        out.sort(Comparator.comparing((MethodLifecycle l) -> l.method().fqn()));
        return new LifecycleResult(out, execTotal, cpuTimeSamples);
    }

    private static void accumulateSample(List<RecordingData.Frame> frames,
            Map<MethodKey, long[]> counts, Map<MethodKey, Map<FrameType, Long>> types) {
        // count each method once per sample for total; self = frame index 0
        Set<MethodKey> seen = new java.util.HashSet<>();
        boolean first = true;
        for (RecordingData.Frame f : frames) {
            MethodKey key = frameKey(f);
            FrameType ft = FrameType.fromString(f.type());
            if (types != null) {
                types.computeIfAbsent(key, k -> new EnumMap2()).merge(ft, 1L, Long::sum);
            }
            long[] c = counts.computeIfAbsent(key, k -> new long[2]);
            if (seen.add(key)) {
                c[0]++; // total once per sample
            }
            if (first) {
                c[1]++; // self frame index 0
            }
            first = false;
        }
    }

    private static MethodKey frameKey(RecordingData.Frame f) {
        return new MethodKey(f.className(), f.methodName(), f.descriptor());
    }

    private static FrameStats buildFrameStats(Map<MethodKey, long[]> counts, Map<MethodKey, Map<FrameType, Long>> types,
            long total, MethodKey key) {
        if (key == null) {
            return FrameStats.EMPTY;
        }
        long[] c = counts.getOrDefault(key, new long[2]);
        return new FrameStats(c[0], c[1], types.getOrDefault(key, Map.of()),
                total == 0 ? 0.0 : (double) c[1] / total, total == 0 ? 0.0 : (double) c[0] / total);
    }

    /** Small EnumMap helper typed as Map<FrameType,Long>. */
    private static final class EnumMap2 extends java.util.EnumMap<FrameType, Long> {
        EnumMap2() {
            super(FrameType.class);
        }
    }
}
