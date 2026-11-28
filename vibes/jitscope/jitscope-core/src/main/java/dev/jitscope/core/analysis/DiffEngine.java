package dev.jitscope.core.analysis;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

import dev.jitscope.core.model.DiffCategory;
import dev.jitscope.core.model.DiffFinding;
import dev.jitscope.core.model.EvidenceLevel;
import dev.jitscope.core.model.InliningRecord;
import dev.jitscope.core.model.MethodKey;
import dev.jitscope.core.model.MethodLifecycle;
import dev.jitscope.core.model.RecordingAnalysis;
import dev.jitscope.core.model.Severity;

/**
 * Regression diff engine. Compares baseline vs candidate {@link RecordingAnalysis}. Normalized
 * relative sample shares (never raw cross-recording counts; JDK 25 JEP 518 hygiene). Detects tier
 * regression, inlining regression, deopt regression, hotness shift; annotates JVM-major-version and
 * sampling-configuration differences.
 */
public final class DiffEngine {

    public static final double DEFAULT_MIN_SAMPLE_SHARE_DELTA = 0.01;

    private DiffEngine() {}

    public static List<DiffFinding> diff(RecordingAnalysis baseline, RecordingAnalysis candidate) {
        return diff(baseline, candidate, DEFAULT_MIN_SAMPLE_SHARE_DELTA);
    }

    public static List<DiffFinding> diff(RecordingAnalysis baseline, RecordingAnalysis candidate, double minSampleShareDelta) {
        List<DiffFinding> out = new ArrayList<>();

        Map<MethodKey, MethodLifecycle> base = byMethod(baseline);
        Map<MethodKey, MethodLifecycle> cand = byMethod(candidate);

        // ---- per-method regressions ----
        for (var e : cand.entrySet()) {
            MethodKey m = e.getKey();
            MethodLifecycle c = e.getValue();
            MethodLifecycle b = base.get(m);

            // hotness shift (normalized sample share delta)
            double baseShare = b == null ? 0.0 : b.executionSamples().selfShare();
            double candShare = c.executionSamples().selfShare();
            double delta = candShare - baseShare;
            if (Math.abs(delta) >= minSampleShareDelta) {
                out.add(new DiffFinding(m, DiffCategory.HOTNESS_SHIFT,
                        delta > 0 ? Severity.MEDIUM : Severity.LOW, EvidenceLevel.OBSERVED,
                        "selfShare=" + baseShare, "selfShare=" + candShare, delta,
                        "normalized self sample share changed by " + fmt(delta)
                                + " (raw cross-recording counts are not compared)"));
            }

            if (b == null) {
                // new hot method in candidate
                if (candShare >= 0.02) {
                    out.add(new DiffFinding(m, DiffCategory.HOTNESS_SHIFT, Severity.MEDIUM, EvidenceLevel.OBSERVED,
                            "absent", "selfShare=" + candShare, candShare, "method newly hot in candidate"));
                }
            } else {
                // tier regression
                if (c.highestCompileLevel() < b.highestCompileLevel() && b.highestCompileLevel() >= 0) {
                    out.add(new DiffFinding(m, DiffCategory.TIER_REGRESSION, Severity.HIGH, EvidenceLevel.OBSERVED,
                            "highestCompileLevel=" + b.highestCompileLevel(), "highestCompileLevel=" + c.highestCompileLevel(),
                            delta, "tier regression Tier" + b.highestCompileLevel() + " -> Tier" + c.highestCompileLevel()));
                }
                // deopt regression
                if (c.deoptimizationCount() > b.deoptimizationCount()) {
                    int deltaDeopt = c.deoptimizationCount() - b.deoptimizationCount();
                    out.add(new DiffFinding(m, DiffCategory.DEOPT_REGRESSION,
                            deltaDeopt >= 3 ? Severity.HIGH : Severity.MEDIUM, EvidenceLevel.OBSERVED,
                            "deoptimizationCount=" + b.deoptimizationCount(), "deoptimizationCount=" + c.deoptimizationCount(),
                            delta, "deopt count increased by " + deltaDeopt));
                }
                // compilation regression
                if (c.compilationCount() > b.compilationCount() && c.deoptimizationCount() >= b.deoptimizationCount()) {
                    // recompilation churn without fewer deopts
                    if (c.compilationCount() - b.compilationCount() >= 2 && c.deoptimizationCount() > 0) {
                        out.add(new DiffFinding(m, DiffCategory.COMPILATION_REGRESSION, Severity.MEDIUM, EvidenceLevel.OBSERVED,
                                "compilationCount=" + b.compilationCount(), "compilationCount=" + c.compilationCount(),
                                delta, "recompilation churn increased"));
                    }
                }
            }
        }

        // ---- inlining regression ----
        Map<MethodKey, long[]> baseInline = inlineCounts(baseline); // [failed, succeeded]
        Map<MethodKey, long[]> candInline = inlineCounts(candidate);
        for (var e : candInline.entrySet()) {
            MethodKey m = e.getKey();
            long[] c = e.getValue();
            long[] b = baseInline.getOrDefault(m, new long[2]);
            boolean newFailures = c[0] > b[0];
            boolean wasInlinedNowNot = b[1] > 0 && c[1] == 0 && c[0] > 0; // Inlined -> JIT compiled
            if (newFailures || wasInlinedNowNot) {
                out.add(new DiffFinding(m, DiffCategory.INLINING_REGRESSION,
                        wasInlinedNowNot ? Severity.HIGH : Severity.MEDIUM, EvidenceLevel.OBSERVED,
                        "failed=" + b[0] + " inlined=" + b[1], "failed=" + c[0] + " inlined=" + c[1],
                        0.0, wasInlinedNowNot
                                ? "callee shifted from Inlined -> JIT compiled / inlining now fails"
                                : "new CompilerInlining failures in candidate"));
            }
        }

        // ---- recording-level annotations ----
        Integer baseMajor = jvmMajor(baseline.metadata().jvmVersion());
        Integer candMajor = jvmMajor(candidate.metadata().jvmVersion());
        if (baseMajor != null && candMajor != null && !baseMajor.equals(candMajor)) {
            out.add(new DiffFinding(new MethodKey("", "recording", ""), DiffCategory.JVM_VERSION_DIFFERENCE, Severity.LOW,
                    EvidenceLevel.OBSERVED, "jvmMajor=" + baseMajor, "jvmMajor=" + candMajor, 0.0,
                    "JVM major version differs; sampling semantics differ (JDK25 JEP518); never compare raw sample counts; "
                            + "normalize to relative share; treat absolute comparisons cautiously"));
        }

        String baseSampling = samplingConfigKey(baseline);
        String candSampling = samplingConfigKey(candidate);
        if (!baseSampling.equals(candSampling)) {
            out.add(new DiffFinding(new MethodKey("", "recording", ""), DiffCategory.SAMPLING_CONFIG_DIFFERENCE, Severity.LOW,
                    EvidenceLevel.OBSERVED, baseSampling, candSampling, 0.0,
                    "sampling configuration differs; sample shares are still normalized but capture fidelity differs"));
        }

        out.sort(Comparator.comparing(DiffFinding::severity).reversed());
        return out;
    }

    private static Map<MethodKey, MethodLifecycle> byMethod(RecordingAnalysis a) {
        return a.lifecycles().stream().collect(Collectors.toMap(MethodLifecycle::method, l -> l, (x, y) -> x, LinkedHashMap::new));
    }

    /** per-callee inlining counts [failed, succeeded]; detects Inlined -> JIT compiled shift. */
    private static Map<MethodKey, long[]> inlineCounts(RecordingAnalysis a) {
        Map<MethodKey, long[]> m = new LinkedHashMap<>();
        for (InliningRecord i : a.inlining()) {
            long[] c = m.computeIfAbsent(i.callee(), k -> new long[2]);
            if (i.succeeded()) {
                c[1]++;
            } else {
                c[0]++;
            }
        }
        return m;
    }

    private static String samplingConfigKey(RecordingAnalysis a) {
        var settings = a.metadata().configAudit().activeSettings();
        String period = settings.getOrDefault("jdk.ExecutionSample#period", "?");
        String enabled = settings.getOrDefault("jdk.ExecutionSample#enabled", "?");
        String compThreshold = settings.getOrDefault("jdk.Compilation#threshold", "?");
        return "execSample.enabled=" + enabled + ",period=" + period + ",compilation.threshold=" + compThreshold;
    }

    private static final Pattern MAJOR = Pattern.compile("(\\d+)");

    public static Integer jvmMajor(String jvmVersion) {
        if (jvmVersion == null || jvmVersion.isBlank()) {
            return null;
        }
        // OpenJDK version strings like "21.0.5", "25", "1.8.0_392", "JDK 21.0.1..."
        Matcher m = MAJOR.matcher(jvmVersion);
        Integer first = null;
        Integer second = null;
        if (m.find()) {
            first = Integer.parseInt(m.group(1));
            if (m.find()) {
                second = Integer.parseInt(m.group(1));
            }
        }
        if (first == null) {
            return null;
        }
        // legacy 1.8.x -> 8
        if (first == 1 && second != null) {
            return second;
        }
        return first;
    }

    private static String fmt(double v) {
        return String.format(java.util.Locale.ROOT, "%.4f", v);
    }
}
