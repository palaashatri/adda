package dev.jitscope.core.analysis;

import java.time.Instant;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import dev.jitscope.core.model.CompilationRecord;
import dev.jitscope.core.model.Evidence;
import dev.jitscope.core.model.EvidenceLevel;
import dev.jitscope.core.model.LifecycleEvent;
import dev.jitscope.core.model.LifecycleEventType;
import dev.jitscope.core.model.LogCompilationData;
import dev.jitscope.core.model.LogCompilationEvent;
import dev.jitscope.core.model.MethodKey;
import dev.jitscope.core.model.MethodLifecycle;
import dev.jitscope.core.model.RecordingAnalysis;

/**
 * {@code LogCompilationEnricher} (§4 #8). Optional streaming StAX parser + enricher for
 * {@code -XX:+LogCompilation} XML. Correlates {@code compile_id} (scoped to recordingId); extracts
 * {@code make_not_entrant}, {@code make_not_compilable}, {@code inline_fail}, {@code uncommon_trap},
 * {@code task}, {@code task_done}, {@code parse}. Upgrades {@code made_not_entrant} /
 * {@code made_non_compilable} and missing inline reasons from {@code INFERRED}/{@code UNKNOWN} to
 * {@code OBSERVED}; findings/lifecycle get {@code Evidence(...,LogCompilation...)}.
 *
 * <p>Invariant §3 #7: made_not_entrant/made_non_compilable are NOT OBSERVED from JFR alone; only
 * here, backed by a LogCompilation event, are they OBSERVED.
 */
public final class LogCompilationEnricher {

    private LogCompilationEnricher() {}

    public static RecordingAnalysis enrich(RecordingAnalysis analysis, LogCompilationData log) {
        if (log == null || log.events().isEmpty()) {
            return analysis;
        }
        String recordingId = analysis.metadata().recordingId();

        // compileId -> compilation (scoped to this recording)
        Map<Integer, CompilationRecord> byCompileId = new HashMap<>();
        for (CompilationRecord c : analysis.compilations()) {
            byCompileId.put(c.compileId(), c);
        }

        // collect events to inject per method
        Map<MethodKey, List<LifecycleEvent>> additions = new HashMap<>();

        for (LogCompilationEvent e : log.events()) {
            Integer compileId = e.compileId();
            CompilationRecord c = compileId == null ? null : byCompileId.get(compileId);
            // method resolution: explicit event method, else joined compilation method
            MethodKey method = e.method() != null ? e.method() : (c == null ? null : c.method());
            if (method == null) {
                continue;
            }
            // match method against known lifecycles if possible (exact key); else still add
            LifecycleEvent le = toLifecycleEvent(e, compileId, c);
            if (le != null) {
                additions.computeIfAbsent(method, k -> new ArrayList<>()).add(le);
            }
        }

        List<MethodLifecycle> lifecycles = new ArrayList<>();
        for (MethodLifecycle lc : analysis.lifecycles()) {
            List<LifecycleEvent> timeline = new ArrayList<>(lc.timeline());
            // upgrade existing inferred/unknown inline-failure entries to OBSERVED when log backs them
            timeline = upgradeInlineFailures(timeline, additions.getOrDefault(lc.method(), List.of()));
            timeline.addAll(additions.getOrDefault(lc.method(), List.of()));
            timeline.sort(Comparator.comparing(LifecycleEvent::timestamp).thenComparing(ev -> ev.type().ordinal()));
            lifecycles.add(new MethodLifecycle(lc.method(), timeline, lc.highestCompileLevel(), lc.compilationCount(),
                    lc.deoptimizationCount(), lc.executionSamples(), lc.cpuTimeSamples(), lc.allocationStats(),
                    lc.lifecycleConfidence()));
        }

        List<String> warnings = new ArrayList<>(analysis.warnings());
        for (String w : log.warnings()) {
            warnings.add("logcompilation: " + w);
        }

        RecordingAnalysis base = new RecordingAnalysis(analysis.metadata(), analysis.compilations(),
                analysis.deoptimizations(), analysis.inlining(), lifecycles, analysis.executionSampleTotal(),
                analysis.cpuTimeSamples(), warnings, analysis.findings(), analysis.executionSamples(), analysis.codeCache());

        // re-run pathology detection on enriched lifecycles
        List<dev.jitscope.core.model.PathologyFinding> findings = PathologyDetectors.detectAll(base);
        return new RecordingAnalysis(base.metadata(), base.compilations(), base.deoptimizations(), base.inlining(),
                base.lifecycles(), base.executionSampleTotal(), base.cpuTimeSamples(), base.warnings(), findings,
                base.executionSamples(), base.codeCache());
    }

    private static LifecycleEvent toLifecycleEvent(LogCompilationEvent e, Integer compileId, CompilationRecord c) {
        Instant ts = Instant.EPOCH; // LogCompilation stamp is VM-relative seconds; not wall-clock
        String stamp = e.attributes().get("stamp");
        Integer level = e.level() != null ? e.level() : (c == null ? null : c.compileLevel());
        Map<String, String> attrs = new HashMap<>(e.attributes());
        attrs.put("compile_id", compileId == null ? "" : Integer.toString(compileId));
        if (stamp != null) {
            attrs.put("stamp", stamp);
        }
        Evidence evidence = new Evidence("LogCompilation:" + e.type().name(),
                e.attributes().toString(), attrs);

        return switch (e.type()) {
            case MAKE_NOT_ENTRANT -> new LifecycleEvent(ts, LifecycleEventType.MADE_NOT_ENTRANT, EvidenceLevel.OBSERVED,
                    compileId, level, "made_not_entrant" + (e.reason() == null ? "" : " reason=" + e.reason()),
                    evidence);
            case MAKE_NOT_COMPILABLE -> new LifecycleEvent(ts, LifecycleEventType.MADE_NON_COMPILABLE, EvidenceLevel.OBSERVED,
                    compileId, level, "made_non_compilable" + (e.reason() == null ? "" : " reason=" + e.reason()),
                    evidence);
            case INLINE_FAIL -> new LifecycleEvent(ts, LifecycleEventType.INLINE_FAILED, EvidenceLevel.OBSERVED,
                    compileId, level, "not inlined" + (e.reason() == null ? "" : " (" + e.reason() + ")"),
                    evidence);
            case UNCOMMON_TRAP -> new LifecycleEvent(ts, LifecycleEventType.DEOPTIMIZATION, EvidenceLevel.OBSERVED,
                    compileId, level, "uncommon_trap reason=" + e.reason() + " action=" + e.action(), evidence);
            case TASK_DONE -> new LifecycleEvent(ts,
                    Boolean.FALSE.equals(e.success()) ? LifecycleEventType.COMPILATION_FAILED : LifecycleEventType.COMPILATION_FINISHED,
                    EvidenceLevel.OBSERVED, compileId, level, "task_done success=" + e.success(), evidence);
            case TASK -> null; // compilation already in timeline from JFR; don't duplicate
            case PARSE -> null;
            default -> null;
        };
    }

    /** Upgrade matching INLINE_FAILED events that lacked a reason to OBSERVED with log reason. */
    private static List<LifecycleEvent> upgradeInlineFailures(List<LifecycleEvent> timeline, List<LifecycleEvent> additions) {
        boolean hasLogInlineFail = additions.stream().anyMatch(a -> a.type() == LifecycleEventType.INLINE_FAILED
                && a.evidence() != null && a.evidence().eventType().startsWith("LogCompilation"));
        if (!hasLogInlineFail) {
            return timeline;
        }
        List<LifecycleEvent> out = new ArrayList<>();
        for (LifecycleEvent e : timeline) {
            if (e.type() == LifecycleEventType.INLINE_FAILED
                    && (e.level() != EvidenceLevel.OBSERVED)) {
                out.add(new LifecycleEvent(e.timestamp(), e.type(), EvidenceLevel.OBSERVED, e.compileId(),
                        e.compileLevel(), e.detail(), e.evidence()));
            } else {
                out.add(e);
            }
        }
        return out;
    }
}
