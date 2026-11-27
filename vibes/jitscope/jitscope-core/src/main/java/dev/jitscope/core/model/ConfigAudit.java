package dev.jitscope.core.model;

import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * Recording configuration audit from {@code jdk.ActiveSetting} + event counts. Flags missing/disabled
 * signals and {@code jdk.Compilation#threshold > 0 ms}, with exact remediation commands.
 */
public record ConfigAudit(
        Completeness completeness,
        List<String> warnings,
        Map<String, String> activeSettings,
        Set<String> disabledRequiredEvents,
        List<String> remediation,
        Map<String, Long> eventCounts,
        Long compilationThresholdMillis) {

    public ConfigAudit {
        warnings = warnings == null ? List.of() : List.copyOf(warnings);
        activeSettings = activeSettings == null ? Map.of() : Map.copyOf(activeSettings);
        disabledRequiredEvents = disabledRequiredEvents == null ? Set.of() : Set.copyOf(disabledRequiredEvents);
        remediation = remediation == null ? List.of() : List.copyOf(remediation);
        eventCounts = eventCounts == null ? Map.of() : Map.copyOf(eventCounts);
    }
}
