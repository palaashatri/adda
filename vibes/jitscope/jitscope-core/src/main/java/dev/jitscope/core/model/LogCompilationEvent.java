package dev.jitscope.core.model;

import java.util.Map;

/** A single event from -XX:+LogCompilation XML. */
public record LogCompilationEvent(
        Type type,
        Integer compileId,
        MethodKey method,
        String reason,
        String action,
        Boolean success,
        Integer level,
        Map<String, String> attributes) {

    public enum Type {
        TASK,
        TASK_DONE,
        MAKE_NOT_ENTRANT,
        MAKE_NOT_COMPILABLE,
        PARSE,
        INLINE_FAIL,
        UNCOMMON_TRAP,
        COMPILE_LOG,
        OTHER
    }

    public LogCompilationEvent {
        attributes = attributes == null ? Map.of() : Map.copyOf(attributes);
    }
}
