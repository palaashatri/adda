package dev.jitscope.core.model;

import java.util.Map;

/** Structured evidence item backing a lifecycle transition or finding. */
public record Evidence(String eventType, String summary, Map<String, String> attributes) {

    public Evidence {
        attributes = attributes == null ? Map.of() : Map.copyOf(attributes);
    }

    public static Evidence of(String eventType, String summary) {
        return new Evidence(eventType, summary, Map.of());
    }
}
