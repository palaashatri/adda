package dev.jitscope.core.analysis;

import java.time.Duration;
import java.time.Instant;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

import dev.jitscope.core.model.CompilationRecord;
import dev.jitscope.core.model.DeoptimizationRecord;
import dev.jitscope.core.model.EvidenceLevel;
import dev.jitscope.core.model.FrameType;
import dev.jitscope.core.model.InliningRecord;
import dev.jitscope.core.model.MethodKey;
import dev.jitscope.core.model.MethodLifecycle;
import dev.jitscope.core.model.PathologyFinding;
import dev.jitscope.core.model.PathologyType;
import dev.jitscope.core.model.RecordingAnalysis;
import dev.jitscope.core.model.Sample;
import dev.jitscope.core.model.SampleFrame;
import dev.jitscope.core.model.Severity;

/**
 * Pathology detectors (§4). Every finding carries {@link EvidenceLevel} + raw evidence + concrete
 * recommendation. Statistical honesty: absence of 'Inlined' frames is NOT proof of inlining failure;
 * correlation is not causation.
 */
public final class PathologyDetectors {

    public static final double DEFAULT_SELF_SHARE_THRESHOLD = 0.02;
    public static final double DEFAULT_TOTAL_SHARE_THRESHOLD = 0.05;
    public static final int DEOPT_STORM_MIN = 3;
    public static final double CODE_CACHE_OCCUPANCY_THRESHOLD = 0.85;
    private static final Duration CORRELATION_WINDOW = Duration.ofMillis(50);

    private PathologyDetectors() {}

    public static List<PathologyFinding> detectAll(RecordingAnalysis a) {
        List<PathologyFinding> out = new ArrayList<>();
        out.addAll(hotUnderoptimized(a, DEFAULT_SELF_SHARE_THRESHOLD, DEFAULT_TOTAL_SHARE_THRESHOLD));
        out.addAll(deoptStorm(a));
        out.addAll(inliningAllocationCliff(a));
        out.addAll(codeCachePressure(a));
        out.sort(Comparator.comparing(PathologyFinding::severity).reversed());
        return out;
    }

    // ---------------- HotUnderoptimizedDetector ----------------
    public static List<PathologyFinding> hotUnderoptimized(RecordingAnalysis a, double selfThreshold, double totalThreshold) {
        List<PathologyFinding> out = new ArrayList<>();
        for (MethodLifecycle l : a.lifecycles()) {
            var fs = l.executionSamples();
            boolean hot = fs.selfShare() >= selfThreshold || fs.totalShare() >= totalThreshold;
            if (!hot) {
                continue;
            }
            long interpreted = fs.byFrameType().getOrDefault(FrameType.INTERPRETED, 0L);
            long totalFrames = fs.byFrameType().values().stream().mapToLong(Long::longValue).sum();
            boolean predominantlyInterpreted = totalFrames > 0 && interpreted * 2 >= totalFrames;
            boolean tierBelow4 = l.highestCompileLevel() < 4;

            if (!predominantlyInterpreted && !tierBelow4) {
                continue;
            }
            String why = predominantlyInterpreted
                    ? "predominantly Interpreted frames (interpreted=" + interpreted + "/" + totalFrames + ")"
                    : "failed to reach Tier 4 (highestCompileLevel=" + l.highestCompileLevel() + ")";
            out.add(new PathologyFinding(
                    "hot-underoptimized:" + l.method().fqn(),
                    PathologyType.HOT_UNDEROPTIMIZED,
                    predominantlyInterpreted ? Severity.HIGH : Severity.MEDIUM,
                    EvidenceLevel.INFERRED, // 'after warmup' cannot be proven strictly; samples are OBSERVED
                    l.method(),
                    "Hot method " + l.method().fqn() + " " + why,
                    "Sample share self=" + pct(fs.selfShare()) + " total=" + pct(fs.totalShare())
                            + "; highestCompileLevel=" + l.highestCompileLevel()
                            + "; compilationCount=" + l.compilationCount()
                            + "; frameTypes=" + fs.byFrameType()
                            + "; thresholds self>=" + pct(selfThreshold) + " total>=" + pct(totalThreshold),
                    List.of(
                            "jdk.ExecutionSample selfShare=" + fs.selfShare(),
                            "jdk.ExecutionSample totalShare=" + fs.totalShare(),
                            "frameTypes=" + fs.byFrameType(),
                            "highestCompileLevel=" + l.highestCompileLevel(),
                            "note=absence of 'Inlined' frames is NOT proof of inlining failure; corroborate with jdk.CompilerInlining"),
                    "Investigate why " + l.method().fqn() + " is hot but " + why
                            + ". Ensure it can reach Tier 4; check inlining (jdk.CompilerInlining), "
                            + "compilation failures (jdk.CompilationFailure), and code-cache pressure. "
                            + "Consider capturing with jit-diag.jfc (jdk.Compilation#threshold=0 ms)."));
        }
        return out;
    }

    // ---------------- DeoptStormDetector ----------------
    public static List<PathologyFinding> deoptStorm(RecordingAnalysis a) {
        List<PathologyFinding> out = new ArrayList<>();
        Map<MethodKey, List<DeoptimizationRecord>> byMethod = new LinkedHashMap<>();
        for (DeoptimizationRecord d : a.deoptimizations()) {
            byMethod.computeIfAbsent(d.method(), k -> new ArrayList<>()).add(d);
        }
        // compileId -> tier join
        Map<Integer, Integer> compileTier = a.compilations().stream()
                .collect(Collectors.toMap(CompilationRecord::compileId, CompilationRecord::compileLevel, (x, y) -> x));

        // JVM-wide storm detection
        int totalDeopts = a.deoptimizations().size();
        boolean jvmWideStorm = totalDeopts >= 10;

        for (var e : byMethod.entrySet()) {
            MethodKey method = e.getKey();
            List<DeoptimizationRecord> deopts = e.getValue();
            // group by (reason,action)
            Map<String, Long> byReasonAction = deopts.stream()
                    .collect(Collectors.groupingBy(d -> d.reason() + "/" + d.action(), Collectors.counting()));
            boolean methodStorm = deopts.size() >= DEOPT_STORM_MIN;
            if (!methodStorm && !jvmWideStorm) {
                continue;
            }

            // Interpreted frame samples increased in window immediately following a deopt (CORRELATED)
            boolean interpretedIncreased = false;
            long beforeSum = 0;
            long afterSum = 0;
            for (DeoptimizationRecord d : deopts) {
                long[] ba = interpretedWindow(a.executionSamples(), method, d.timestamp(), CORRELATION_WINDOW);
                beforeSum += ba[0];
                afterSum += ba[1];
            }
            interpretedIncreased = afterSum > beforeSum;

            List<String> evidence = new ArrayList<>();
            evidence.add("jdk.Deoptimization count=" + deopts.size() + " on " + method.fqn());
            for (var re : byReasonAction.entrySet()) {
                evidence.add("reason/action " + re.getKey() + " x" + re.getValue());
            }
            for (DeoptimizationRecord d : deopts.stream().limit(5).collect(Collectors.toList())) {
                evidence.add("compileId=" + d.compileId() + " tier=" + compileTier.getOrDefault(d.compileId(), -1)
                        + " bci=" + d.bci() + " line=" + d.lineNumber() + " reason=" + d.reason() + " action=" + d.action()
                        + " compiler=" + d.compiler());
            }
            evidence.add("Interpreted frames before=" + beforeSum + " after=" + afterSum
                    + " window=" + CORRELATION_WINDOW.toMillis() + "ms");
            if (jvmWideStorm) {
                evidence.add("JVM-wide deopt storm total=" + totalDeopts);
            }

            EvidenceLevel level = interpretedIncreased ? EvidenceLevel.INFERRED : EvidenceLevel.OBSERVED;
            out.add(new PathologyFinding(
                    "deopt-storm:" + method.fqn(),
                    PathologyType.DEOPT_STORM,
                    methodStorm && interpretedIncreased ? Severity.CRITICAL
                            : methodStorm ? Severity.HIGH : Severity.MEDIUM,
                    level,
                    method,
                    "Deoptimization storm on " + method.fqn(),
                    "repeated deopt-recompile cycles"
                            + (methodStorm ? " (>= " + DEOPT_STORM_MIN + " on method)" : "")
                            + (jvmWideStorm ? " + JVM-wide storm" : "")
                            + (interpretedIncreased ? "; Interpreted frames increased after deopt (CORRELATED)" : "")
                            + "; groups=" + byReasonAction,
                    evidence,
                    "Investigate unstable types/branches (" + byReasonAction.keySet() + ") on " + method.fqn()
                            + ". Look for class_check/unstable_if; stabilize call sites; avoid megamorphic dispatch; "
                            + "join jdk.Deoptimization to jdk.Compilation via compileId to inspect recompilation churn."));
        }
        return out;
    }

    // ---------------- InliningAllocationDetector ----------------
    public static List<PathologyFinding> inliningAllocationCliff(RecordingAnalysis a) {
        List<PathologyFinding> out = new ArrayList<>();
        Map<MethodKey, List<InliningRecord>> failedByCaller = new LinkedHashMap<>();
        for (InliningRecord i : a.inlining()) {
            if (!i.succeeded()) {
                failedByCaller.computeIfAbsent(i.caller(), k -> new ArrayList<>()).add(i);
            }
        }
        Map<MethodKey, MethodLifecycle> lcByMethod = a.lifecycles().stream()
                .collect(Collectors.toMap(MethodLifecycle::method, l -> l, (x, y) -> x));

        for (var e : failedByCaller.entrySet()) {
            MethodKey caller = e.getKey();
            List<InliningRecord> failed = e.getValue();
            MethodLifecycle lc = lcByMethod.get(caller);
            double selfShare = lc == null ? 0 : lc.executionSamples().selfShare();
            double totalShare = lc == null ? 0 : lc.executionSamples().totalShare();
            boolean hot = selfShare >= DEFAULT_SELF_SHARE_THRESHOLD || totalShare >= DEFAULT_TOTAL_SHARE_THRESHOLD;
            long allocSamples = lc == null ? 0 : lc.allocationStats().sampleCount();
            double allocShare = lc == null ? 0 : lc.allocationStats().totalShare();
            // elevated allocation frequency indicating potential missed escape analysis / scalar replacement
            boolean elevatedAlloc = allocSamples > 0 && (allocShare >= 0.01 || allocSamples >= failed.size());

            // Statistical honesty: absence of Inlined isn't proof; require CompilerInlining failure + hotness.
            if (!hot && !elevatedAlloc) {
                continue;
            }

            List<String> messages = failed.stream().map(InliningRecord::message).distinct().sorted().collect(Collectors.toList());
            List<String> evidence = new ArrayList<>();
            evidence.add("jdk.CompilerInlining succeeded=false count=" + failed.size() + " caller=" + caller.fqn());
            for (String m : messages.stream().limit(8).collect(Collectors.toList())) {
                evidence.add("inline_fail message=" + m);
            }
            evidence.add("hot selfShare=" + selfShare + " totalShare=" + totalShare);
            evidence.add("ObjectAllocationSample count=" + allocSamples + " share=" + allocShare
                    + " (observed weight, not exact heap totals)");
            if (lc != null) {
                evidence.add("highestCompileLevel=" + lc.highestCompileLevel());
            }

            out.add(new PathologyFinding(
                    "inlining-allocation-cliff:" + caller.fqn(),
                    PathologyType.INLINING_ALLOCATION_CLIFF,
                    hot && elevatedAlloc ? Severity.HIGH : Severity.MEDIUM,
                    EvidenceLevel.INFERRED, // missed escape analysis/scalar replacement inferred
                    caller,
                    "Inlining cliff on hot method " + caller.fqn(),
                    "failed inlining decisions=" + messages + (elevatedAlloc ? " + elevated allocation" : "")
                            + "; potential missed Escape Analysis / scalar replacement",
                    evidence,
                    "Reduce callee size / megamorphic dispatch to enable inlining on " + caller.fqn()
                            + "; check 'callee is too large','hot method too big','already compiled into a big method',"
                            + " megamorphic/virtual dispatch; verify scalar replacement via allocation sampling."));
        }
        return out;
    }

    // ---------------- CodeCachePressureDetector ----------------
    public static List<PathologyFinding> codeCachePressure(RecordingAnalysis a) {
        var cc = a.codeCache();
        double peak = cc.peakOccupancy();
        long full = cc.codeCacheFullCount();
        boolean pressure = full > 0 || peak >= CODE_CACHE_OCCUPANCY_THRESHOLD;

        // subsequent compilation failures / stalled compilations after pressure
        long compilationFailures = a.compilations().stream().filter(c -> !c.succeeded()).count();
        Instant pressureTime = cc.statistics().stream().filter(s -> s.occupancy() >= CODE_CACHE_OCCUPANCY_THRESHOLD)
                .map(s -> s.timestamp()).min(Instant::compareTo).orElse(null);
        long subsequentFailures = a.compilations().stream()
                .filter(c -> !c.succeeded())
                .filter(c -> pressureTime == null || !c.startTime().isBefore(pressureTime))
                .count();

        if (!pressure && compilationFailures == 0) {
            return List.of();
        }
        if (!pressure) {
            // no cache pressure signal; don't invent finding
            return List.of();
        }

        List<String> evidence = new ArrayList<>();
        evidence.add("jdk.CodeCacheFull count=" + full);
        evidence.add("peak occupancy=" + pct(peak) + " threshold=" + pct(CODE_CACHE_OCCUPANCY_THRESHOLD));
        for (var s : cc.statistics().stream().limit(8).collect(Collectors.toList())) {
            evidence.add("CodeCacheStatistics type=" + s.codeBlobType() + " occ=" + pct(s.occupancy())
                    + " used=" + s.usedBytes() + "/" + s.reservedTotalBytes() + " unallocated=" + s.unallocatedCapacityBytes()
                    + " methodCount=" + s.methodCount() + " fullCount=" + s.fullCount());
        }
        cc.configuration().ifPresent(c -> evidence.add("CodeCacheConfiguration reserved=" + c.reservedSizeBytes()
                + " nonProfiled=" + c.nonProfiledSizeBytes() + " profiled=" + c.profiledSizeBytes()
                + " nonNmethod=" + c.nonNmethodSizeBytes() + " expansion=" + c.expansionSizeBytes()));
        evidence.add("subsequent compilation failures=" + subsequentFailures + " total failed=" + compilationFailures);

        MethodKey method = a.compilations().stream().filter(c -> !c.succeeded()).map(CompilationRecord::method)
                .findFirst().orElse(new MethodKey("", "jdk.CodeCache", ""));
        return List.of(new PathologyFinding(
                "code-cache-pressure",
                PathologyType.CODE_CACHE_PRESSURE,
                full > 0 ? Severity.CRITICAL : Severity.HIGH,
                EvidenceLevel.OBSERVED,
                method,
                "Code-cache pressure" + (full > 0 ? " + CodeCacheFull" : ""),
                "high occupancy>=" + pct(CODE_CACHE_OCCUPANCY_THRESHOLD) + " or jdk.CodeCacheFull; "
                        + "correlated with subsequent compilation failures/stalled compilations=" + subsequentFailures,
                evidence,
                "Increase -XX:ReservedCodeCacheSize; reduce dynamically generated classes / retained class loaders; "
                        + "disable unneeded JIT compilation; inspect codeBlobType segments; correlate with jdk.CompilationFailure."));
    }

    // ---- helpers ----
    private static long[] interpretedWindow(List<Sample> samples, MethodKey method, Instant t, Duration window) {
        Instant before0 = t.minus(window);
        Instant after1 = t.plus(window);
        long before = 0;
        long after = 0;
        for (Sample s : samples) {
            if (s.timestamp().isBefore(before0) || s.timestamp().isAfter(after1)) {
                continue;
            }
            boolean hasInterpreted = s.frames().stream()
                    .anyMatch(f -> f.method().equals(method) && FrameType.fromString(f.type()) == FrameType.INTERPRETED);
            // top-frame interpreted for method = self
            boolean selfInterpreted = !s.frames().isEmpty()
                    && s.frames().get(0).method().equals(method)
                    && FrameType.fromString(s.frames().get(0).type()) == FrameType.INTERPRETED;
            long inc = (hasInterpreted || selfInterpreted) ? 1 : 0;
            if (s.timestamp().isBefore(t)) {
                before += inc;
            } else if (s.timestamp().isAfter(t)) {
                after += inc;
            }
        }
        return new long[] {before, after};
    }

    private static String pct(double v) {
        return String.format(java.util.Locale.ROOT, "%.2f%%", v * 100.0);
    }
}
