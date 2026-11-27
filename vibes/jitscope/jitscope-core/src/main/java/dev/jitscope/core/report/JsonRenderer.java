package dev.jitscope.core.report;

import java.util.ArrayList;
import java.util.List;

import dev.jitscope.core.model.AllocationStats;
import dev.jitscope.core.model.CodeCacheInfo;
import dev.jitscope.core.model.CompilationRecord;
import dev.jitscope.core.model.ConfigAudit;
import dev.jitscope.core.model.DeoptimizationRecord;
import dev.jitscope.core.model.DiffFinding;
import dev.jitscope.core.model.Evidence;
import dev.jitscope.core.model.FrameStats;
import dev.jitscope.core.model.InliningRecord;
import dev.jitscope.core.model.LifecycleEvent;
import dev.jitscope.core.model.MethodKey;
import dev.jitscope.core.model.MethodLifecycle;
import dev.jitscope.core.model.PathologyFinding;
import dev.jitscope.core.model.RecordingAnalysis;
import dev.jitscope.core.model.RecordingMetadata;

import static dev.jitscope.core.report.JsonWriter.*;

/** Structured JSON matching jitscope-core DTOs. No diagnostic logic. */
public final class JsonRenderer {

    private JsonRenderer() {}

    public static String analysis(RecordingAnalysis a) {
        return obj(
                "metadata", metadata(a.metadata()),
                "executionSampleTotal", num(a.executionSampleTotal()),
                "cpuTimeSamples", a.cpuTimeSamples().map(c -> obj(
                        "totalSamples", num(c.totalSamples()),
                        "failedSamples", num(c.failedSamples()),
                        "biasedSamples", num(c.biasedSamples()),
                        "lostSamples", num(c.lostSamples()),
                        "confidenceDowngraded", bool(c.confidenceDowngraded()))).orElse("null"),
                "compilations", arr(map(a.compilations(), JsonRenderer::compilation)),
                "deoptimizations", arr(map(a.deoptimizations(), JsonRenderer::deopt)),
                "inlining", arr(map(a.inlining(), JsonRenderer::inlining)),
                "lifecycles", arr(map(a.lifecycles(), JsonRenderer::lifecycle)),
                "codeCache", codeCache(a.codeCache()),
                "warnings", arr(map(a.warnings(), JsonWriter::str)),
                "findings", arr(map(a.findings(), JsonRenderer::finding)));
    }

    public static String method(MethodLifecycle l) {
        return lifecycle(l);
    }

    public static String methodList(List<MethodLifecycle> ls) {
        return obj("methods", arr(map(ls, JsonRenderer::lifecycle)));
    }

    public static String diff(List<DiffFinding> diffs, String note) {
        return obj("note", str(note == null ? "" : note), "findings", arr(map(diffs, JsonRenderer::diffFinding)));
    }

    public static String configCheck(ConfigAudit audit) {
        return obj(
                "completeness", str(audit.completeness().name()),
                "compilationThresholdMillis", audit.compilationThresholdMillis() == null ? "null"
                        : num(audit.compilationThresholdMillis()),
                "disabledRequiredEvents", arr(map(new ArrayList<>(audit.disabledRequiredEvents()), JsonWriter::str)),
                "warnings", arr(map(audit.warnings(), JsonWriter::str)),
                "remediation", arr(map(audit.remediation(), JsonWriter::str)),
                "activeSettings", stringMap(audit.activeSettings()),
                "eventCounts", eventCounts(audit.eventCounts()));
    }

    public static String findings(List<PathologyFinding> findings) {
        return obj("findings", arr(map(findings, JsonRenderer::finding)));
    }

    // ---- helpers ----
    public static String methodKey(MethodKey k) {
        return obj("className", str(k.className()), "methodName", str(k.methodName()),
                "descriptor", str(k.descriptor()), "fqn", str(k.fqn()));
    }

    private static String metadata(RecordingMetadata m) {
        return obj(
                "recordingId", str(m.recordingId()),
                "sourcePath", str(m.sourcePath() == null ? "" : m.sourcePath().toString()),
                "jvmVersion", str(m.jvmVersion()),
                "jvmArguments", str(m.jvmArguments()),
                "startTime", str(m.startTime() == null ? "" : m.startTime().toString()),
                "endTime", str(m.endTime() == null ? "" : m.endTime().toString()),
                "durationMillis", num(m.duration().toMillis()),
                "configAudit", configCheck(m.configAudit()));
    }

    private static String compilation(CompilationRecord c) {
        return obj("compileId", num(c.compileId()), "method", methodKey(c.method()),
                "compileLevel", num(c.compileLevel()), "osr", bool(c.osr()), "succeeded", bool(c.succeeded()),
                "startTime", str(c.startTime().toString()), "durationMillis", num(c.duration().toMillis()),
                "codeSizeBytes", num(c.codeSizeBytes()), "inlinedBytes", num(c.inlinedBytes()),
                "failureMessage", c.failureMessage() == null ? "null" : str(c.failureMessage()));
    }

    private static String deopt(DeoptimizationRecord d) {
        return obj("compileId", num(d.compileId()), "method", methodKey(d.method()), "bci", num(d.bci()),
                "lineNumber", num(d.lineNumber()), "reason", str(d.reason()), "action", str(d.action()),
                "compiler", str(d.compiler()), "timestamp", str(d.timestamp().toString()));
    }

    private static String inlining(InliningRecord i) {
        return obj("compileId", num(i.compileId()), "caller", methodKey(i.caller()), "callee", methodKey(i.callee()),
                "bci", num(i.bci()), "succeeded", bool(i.succeeded()), "message", str(i.message()), "bytes", num(i.bytes()));
    }

    private static String frameStats(FrameStats f) {
        StringBuilder types = new StringBuilder("{");
        boolean first = true;
        for (var e : f.byFrameType().entrySet()) {
            if (!first) {
                types.append(',');
            }
            first = false;
            types.append(str(e.getKey().name())).append(':').append(num(e.getValue()));
        }
        types.append('}');
        return obj("totalSamples", num(f.totalSamples()), "selfSamples", num(f.selfSamples()),
                "byFrameType", types.toString(), "selfShare", num(f.selfShare()), "totalShare", num(f.totalShare()));
    }

    private static String allocation(AllocationStats a) {
        return obj("sampleCount", num(a.sampleCount()), "totalSampleWeightBytes", num(a.totalSampleWeightBytes()),
                "selfShare", num(a.selfShare()), "totalShare", num(a.totalShare()));
    }

    private static String lifecycleEvent(LifecycleEvent e) {
        return obj("timestamp", str(e.timestamp().toString()), "type", str(e.type().name()), "level", str(e.level().name()),
                "compileId", e.compileId() == null ? "null" : num(e.compileId()),
                "compileLevel", e.compileLevel() == null ? "null" : num(e.compileLevel()),
                "detail", str(e.detail() == null ? "" : e.detail()), "evidence", evidence(e.evidence()));
    }

    private static String evidence(Evidence ev) {
        if (ev == null) {
            return "null";
        }
        return obj("eventType", str(ev.eventType()), "summary", str(ev.summary() == null ? "" : ev.summary()),
                "attributes", stringMap(ev.attributes()));
    }

    private static String lifecycle(MethodLifecycle l) {
        return obj("method", methodKey(l.method()),
                "highestCompileLevel", num(l.highestCompileLevel()),
                "compilationCount", num(l.compilationCount()), "deoptimizationCount", num(l.deoptimizationCount()),
                "executionSamples", frameStats(l.executionSamples()),
                "cpuTimeSamples", l.cpuTimeSamples().map(JsonRenderer::frameStats).orElse("null"),
                "allocationStats", allocation(l.allocationStats()),
                "lifecycleConfidence", str(l.lifecycleConfidence().name()),
                "timeline", arr(map(l.timeline(), JsonRenderer::lifecycleEvent)));
    }

    private static String finding(PathologyFinding f) {
        return obj("id", str(f.id()), "type", str(f.type().name()), "severity", str(f.severity().name()),
                "evidenceLevel", str(f.evidenceLevel().name()), "primaryMethod", methodKey(f.primaryMethod()),
                "title", str(f.title()), "explanation", str(f.explanation()),
                "evidence", arr(map(f.evidence(), JsonWriter::str)), "recommendation", str(f.recommendation()));
    }

    private static String diffFinding(DiffFinding d) {
        return obj("method", methodKey(d.method()), "category", str(d.category().name()), "severity", str(d.severity().name()),
                "evidenceLevel", str(d.evidenceLevel().name()), "baselineSummary", str(d.baselineSummary()),
                "candidateSummary", str(d.candidateSummary()), "sampleShareDelta", num(d.sampleShareDelta()),
                "explanation", str(d.explanation()));
    }

    private static String codeCache(CodeCacheInfo c) {
        return obj("codeCacheFullCount", num(c.codeCacheFullCount()), "peakOccupancy", num(c.peakOccupancy()),
                "statistics", arr(map(c.statistics(), s -> obj("timestamp", str(s.timestamp().toString()),
                        "codeBlobType", str(s.codeBlobType()), "reservedTotalBytes", num(s.reservedTotalBytes()),
                        "usedBytes", num(s.usedBytes()), "unallocatedCapacityBytes", num(s.unallocatedCapacityBytes()),
                        "fullCount", num(s.fullCount()), "methodCount", num(s.methodCount()), "occupancy", num(s.occupancy())))),
                "configuration", c.configuration().map(cfg -> obj("reservedSizeBytes", num(cfg.reservedSizeBytes()),
                        "nonProfiledSizeBytes", num(cfg.nonProfiledSizeBytes()), "profiledSizeBytes", num(cfg.profiledSizeBytes()),
                        "nonNmethodSizeBytes", num(cfg.nonNmethodSizeBytes()), "expansionSizeBytes", num(cfg.expansionSizeBytes()),
                        "codeBlobType", str(cfg.codeBlobType()))).orElse("null"));
    }

    private static String eventCounts(java.util.Map<String, Long> m) {
        StringBuilder b = new StringBuilder("{");
        boolean first = true;
        for (var e : m.entrySet()) {
            if (!first) {
                b.append(',');
            }
            first = false;
            b.append(str(e.getKey())).append(':').append(num(e.getValue()));
        }
        return b.append('}').toString();
    }

    private static <T> List<String> map(List<T> in, java.util.function.Function<T, String> fn) {
        List<String> out = new ArrayList<>();
        for (T t : in) {
            out.add(fn.apply(t));
        }
        return out;
    }
}
