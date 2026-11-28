package dev.jitscope.core.model;

import java.util.List;

/** Parsed -XX:+LogCompilation XML (streaming StAX). Never silently swallows; warnings recorded. */
public record LogCompilationData(List<LogCompilationEvent> events, List<String> warnings) {
    public LogCompilationData {
        events = events == null ? List.of() : List.copyOf(events);
        warnings = warnings == null ? List.of() : List.copyOf(warnings);
    }

    public List<LogCompilationEvent> of(LogCompilationEvent.Type type) {
        return events.stream().filter(e -> e.type() == type).toList();
    }
}
