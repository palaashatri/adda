package dev.jitscope.core.report;

import java.util.List;
import java.util.stream.Collectors;

import dev.jitscope.core.model.ConfigAudit;
import dev.jitscope.core.model.DiffFinding;
import dev.jitscope.core.model.FrameStats;
import dev.jitscope.core.model.MethodLifecycle;
import dev.jitscope.core.model.PathologyFinding;
import dev.jitscope.core.model.RecordingAnalysis;

/** Self-contained readable Markdown. No diagnostic logic. */
public final class MarkdownRenderer {

    private MarkdownRenderer() {}

    public static String analysis(RecordingAnalysis a, int topN) {
        StringBuilder b = new StringBuilder();
        b.append("# JITScope analysis\n\n");
        b.append("Recording: `").append(a.metadata().sourcePath()).append("`\n\n");
        b.append("- recordingId: `").append(a.metadata().recordingId()).append("`\n");
        b.append("- JVM: `").append(a.metadata().jvmVersion()).append("`\n");
        b.append("- duration: ").append(a.metadata().duration().toMillis()).append(" ms\n");
        b.append("- execution samples (jdk.ExecutionSample): ").append(a.executionSampleTotal()).append('\n');
        a.cpuTimeSamples().ifPresent(c -> b.append("- CPU-time samples (jdk.CPUTimeSample, separate): ")
                .append(c.totalSamples()).append(" total, ").append(c.failedSamples()).append(" failed, ")
                .append(c.lostSamples()).append(" lost, confidenceDowngraded=").append(c.confidenceDowngraded()).append('\n'));
        b.append("- code-cache: full=").append(a.codeCache().codeCacheFullCount())
                .append(" peakOccupancy=").append(pct(a.codeCache().peakOccupancy())).append('\n');
        b.append('\n').append(configAudit(a.metadata().configAudit())).append('\n');

        // top hot methods
        b.append("## Top hot methods\n\n");
        b.append("| method | self% | total% | frames | highestLevel | deopts | compilations |\n");
        b.append("|---|---|---|---|---|---|---|\n");
        a.lifecycles().stream()
                .sorted((x, y) -> Double.compare(y.executionSamples().selfShare(), x.executionSamples().selfShare()))
                .limit(topN)
                .forEach(l -> {
                    FrameStats fs = l.executionSamples();
                    b.append("| `").append(l.method().fqn()).append("` | ")
                            .append(pct(fs.selfShare())).append(" | ").append(pct(fs.totalShare())).append(" | ")
                            .append(fs.byFrameType()).append(" | ").append(l.highestCompileLevel()).append(" | ")
                            .append(l.deoptimizationCount()).append(" | ").append(l.compilationCount()).append(" |\n");
                });
        b.append('\n');

        // findings
        b.append("## Findings\n\n");
        if (a.findings().isEmpty()) {
            b.append("No pathologies detected.\n");
        } else {
            for (PathologyFinding f : a.findings()) {
                b.append(finding(f));
            }
        }

        if (!a.warnings().isEmpty()) {
            b.append("\n## Warnings\n\n");
            for (String w : a.warnings()) {
                b.append("- ").append(w).append('\n');
            }
        }
        return b.toString();
    }

    public static String method(List<MethodLifecycle> ls, String pattern) {
        StringBuilder b = new StringBuilder();
        b.append("# JITScope method report\n\npattern: `").append(pattern).append("`\n\n");
        if (ls.isEmpty()) {
            b.append("No matching methods.\n");
            return b.toString();
        }
        for (MethodLifecycle l : ls) {
            b.append("## `").append(l.method().fqn()).append("`\n\n");
            b.append("- highestCompileLevel: ").append(l.highestCompileLevel()).append('\n');
            b.append("- compilationCount: ").append(l.compilationCount()).append('\n');
            b.append("- deoptimizationCount: ").append(l.deoptimizationCount()).append('\n');
            b.append("- lifecycleConfidence: ").append(l.lifecycleConfidence()).append('\n');
            FrameStats fs = l.executionSamples();
            b.append("- executionSamples self=").append(pct(fs.selfShare())).append(" total=").append(pct(fs.totalShare()))
                    .append(" frames=").append(fs.byFrameType()).append('\n');
            b.append("- allocationSamples=").append(l.allocationStats().sampleCount())
                    .append(" weight=").append(l.allocationStats().totalSampleWeightBytes()).append('\n');
            l.cpuTimeSamples().ifPresent(c -> b.append("- cpuTimeSamples total=").append(c.totalSamples())
                    .append(" self=").append(pct(c.selfShare())).append('\n'));

            b.append("\n### Compilation / deopt / inlining timeline\n\n");
            b.append("| time | type | level | compileId | detail | evidence |\n");
            b.append("|---|---|---|---|---|---|\n");
            for (var e : l.timeline()) {
                b.append("| ").append(e.timestamp()).append(" | ").append(e.type()).append(" | ")
                        .append(e.level()).append(" | ").append(e.compileId() == null ? "" : e.compileId()).append(" | ")
                        .append(md(e.detail())).append(" | ")
                        .append(e.evidence() == null ? "" : e.evidence().eventType()).append(" |\n");
            }
            b.append('\n');
        }
        return b.toString();
    }

    public static String diff(List<DiffFinding> diffs, String note) {
        StringBuilder b = new StringBuilder("# JITScope diff\n\n");
        if (note != null && !note.isBlank()) {
            b.append("> ").append(note).append("\n\n");
        }
        if (diffs.isEmpty()) {
            b.append("No regressions detected.\n");
            return b.toString();
        }
        b.append("| method | category | severity | evidence | baseline | candidate | shareDelta | explanation |\n");
        b.append("|---|---|---|---|---|---|---|---|\n");
        for (DiffFinding d : diffs) {
            b.append("| `").append(d.method().fqn()).append("` | ").append(d.category()).append(" | ")
                    .append(d.severity()).append(" | ").append(d.evidenceLevel()).append(" | ")
                    .append(md(d.baselineSummary())).append(" | ").append(md(d.candidateSummary())).append(" | ")
                    .append(String.format(java.util.Locale.ROOT, "%.4f", d.sampleShareDelta())).append(" | ")
                    .append(md(d.explanation())).append(" |\n");
        }
        return b.toString();
    }

    public static String configCheck(ConfigAudit audit) {
        return "# JITScope config check\n\n" + configAudit(audit);
    }

    public static String finding(PathologyFinding f) {
        StringBuilder b = new StringBuilder();
        b.append("### [").append(f.severity()).append("] ").append(f.type()).append(" - ").append(f.title()).append('\n');
        b.append("- method: `").append(f.primaryMethod().fqn()).append("`\n");
        b.append("- evidenceLevel: ").append(f.evidenceLevel()).append('\n');
        b.append("- explanation: ").append(f.explanation()).append('\n');
        b.append("- evidence:\n");
        for (String e : f.evidence()) {
            b.append("  - ").append(e).append('\n');
        }
        b.append("- recommendation: ").append(f.recommendation()).append("\n\n");
        return b.toString();
    }

    private static String configAudit(ConfigAudit a) {
        StringBuilder b = new StringBuilder("## Recording audit\n\n");
        b.append("- completeness: **").append(a.completeness()).append("**\n");
        b.append("- jdk.Compilation#threshold(ms): ").append(a.compilationThresholdMillis() == null ? "unknown"
                : a.compilationThresholdMillis()).append('\n');
        b.append("- disabled/missing required events: ")
                .append(a.disabledRequiredEvents().isEmpty() ? "none" : a.disabledRequiredEvents().toString()).append('\n');
        for (String w : a.warnings()) {
            b.append("  - WARN: ").append(w).append('\n');
        }
        if (!a.remediation().isEmpty()) {
            b.append("- remediation:\n");
            for (String r : a.remediation()) {
                b.append("  - `").append(r).append("`\n");
            }
        }
        return b.toString();
    }

    private static String pct(double v) {
        return String.format(java.util.Locale.ROOT, "%.2f%%", v * 100.0);
    }

    private static String md(String s) {
        return s == null ? "" : s.replace("|", "\\|").replace("\n", " ");
    }
}
