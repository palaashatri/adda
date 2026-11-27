package dev.jitscope.core.analysis;

import java.util.ArrayList;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import dev.jitscope.core.ingest.RecordingData;
import dev.jitscope.core.model.Completeness;
import dev.jitscope.core.model.ConfigAudit;

/**
 * Audits recording configuration from {@code jdk.ActiveSetting} + event counts. Flags disabled or
 * missing signals and {@code jdk.Compilation#threshold > 0 ms} (default.jfc=1000 ms, profile.jfc=
 * 100 ms hide warmup). Emits exact remediation commands.
 */
public final class ConfigAuditor {

    /** Hard-required events; absence makes a recording INSUFFICIENT. */
    public static final List<String> HARD_REQUIRED = List.of(
            "jdk.Compilation",
            "jdk.CompilerInlining",
            "jdk.ExecutionSample");

    /** Flagged-if-disabled/missing events; absence degrades to PARTIAL. */
    public static final List<String> SOFT_REQUIRED = List.of(
            "jdk.CompilationFailure",
            "jdk.Deoptimization",
            "jdk.CodeCacheFull");

    /** Remediation applied when any signal is missing. */
    public static final String REMEDIATION_SETTINGS =
            "-XX:StartFlightRecording=settings=jit-diag.jfc,filename=out.jfr";
    public static final String REMEDIATION_INIT = "jitscope init-jfc --out jit-diag.jfc";

    private ConfigAuditor() {}

    public static ConfigAudit audit(RecordingData data) {
        List<String> warnings = new ArrayList<>();
        Set<String> disabled = new LinkedHashSet<>();
        List<String> remediation = new ArrayList<>();

        Long thresholdMillis = compilationThresholdMillis(data);
        if (thresholdMillis == null) {
            warnings.add("jdk.Compilation#threshold not reported; cannot confirm 0 ms");
        } else if (thresholdMillis > 0) {
            warnings.add("jdk.Compilation#threshold=" + thresholdMillis + " ms > 0 ms; compilations below "
                    + "threshold are hidden. Known hazards: default.jfc=1000 ms, profile.jfc=100 ms hide warmup.");
            remediation.add("Set jdk.Compilation#threshold=0ms (e.g. -XX:StartFlightRecording=jdk.Compilation#threshold=0ms,... or jit-diag.jfc).");
        }

        boolean anyMissing = thresholdMillis == null || thresholdMillis > 0;

        for (String ev : HARD_REQUIRED) {
            if (!isEventEnabled(data, ev)) {
                disabled.add(ev);
                warnings.add(ev + " is disabled or missing in this recording.");
                anyMissing = true;
            }
        }
        for (String ev : SOFT_REQUIRED) {
            if (!isEventEnabled(data, ev)) {
                disabled.add(ev);
                warnings.add(ev + " is disabled or missing in this recording.");
                anyMissing = true;
            }
        }

        // No compilations at all => cannot assess JIT.
        if (data.compilations.isEmpty()) {
            warnings.add("no jdk.Compilation events captured");
            anyMissing = true;
        }

        Completeness completeness;
        boolean hardMissing = !isEventEnabled(data, "jdk.Compilation")
                || !isEventEnabled(data, "jdk.CompilerInlining")
                || !isEventEnabled(data, "jdk.ExecutionSample")
                || data.compilations.isEmpty();
        if (hardMissing) {
            completeness = Completeness.INSUFFICIENT;
        } else if (anyMissing) {
            completeness = Completeness.PARTIAL;
        } else {
            completeness = Completeness.COMPLETE;
        }

        if (anyMissing || completeness != Completeness.COMPLETE) {
            remediation.add(REMEDIATION_SETTINGS);
            remediation.add(REMEDIATION_INIT);
            for (String ev : disabled) {
                remediation.add("Enable " + ev + "#enabled=true");
            }
        }

        // surface non-fatal parse warnings transparently
        for (String w : data.warnings) {
            warnings.add("parse: " + w);
        }

        return new ConfigAudit(
                completeness,
                warnings,
                flattenActiveSettings(data.activeSettings),
                disabled,
                dedup(remediation),
                data.eventCounts,
                thresholdMillis);
    }

    /** jdk.Compilation#threshold in milliseconds, or null if unknown. */
    public static Long compilationThresholdMillis(RecordingData data) {
        Map<String, String> s = data.activeSettings.get("jdk.Compilation");
        if (s == null) {
            return null;
        }
        String raw = s.get("threshold");
        if (raw == null) {
            return null;
        }
        // formats like "0 ms", "100 ms", "0ms", "1 s"
        String t = raw.trim().toLowerCase();
        try {
            String numPart = t;
            long multiplier = 1L;
            if (t.endsWith("ms")) {
                numPart = t.substring(0, t.length() - 2).trim();
                multiplier = 1L;
            } else if (t.endsWith("s")) {
                numPart = t.substring(0, t.length() - 1).trim();
                multiplier = 1000L;
            } else if (t.endsWith("m")) {
                numPart = t.substring(0, t.length() - 1).trim();
                multiplier = 60_000L;
            }
            // strip possible fractional
            double val = Double.parseDouble(numPart);
            return Math.round(val * multiplier);
        } catch (NumberFormatException ex) {
            return null;
        }
    }

    /** Is an event explicitly enabled, or absent-but-present-with-events? */
    public static boolean isEventEnabled(RecordingData data, String eventName) {
        Map<String, String> s = data.activeSettings.get(eventName);
        if (s != null && s.containsKey("enabled")) {
            return Boolean.parseBoolean(s.get("enabled"));
        }
        return data.eventCounts.getOrDefault(eventName, 0L) > 0L;
    }

    private static Map<String, String> flattenActiveSettings(Map<String, Map<String, String>> in) {
        java.util.TreeMap<String, String> out = new java.util.TreeMap<>();
        for (var e : in.entrySet()) {
            for (var s : e.getValue().entrySet()) {
                out.put(e.getKey() + "#" + s.getKey(), s.getValue());
            }
        }
        return out;
    }

    private static List<String> dedup(List<String> in) {
        return new ArrayList<>(new LinkedHashSet<>(in));
    }
}
