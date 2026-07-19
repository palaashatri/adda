package com.aurora.platform.ai;

import java.util.Map;

/**
 * Immutable request passed to an AI provider.
 */
public record AiRequest(String prompt, Map<String, String> options) {
    public AiRequest {
        prompt = prompt == null ? "" : prompt;
        options = options == null ? Map.of() : Map.copyOf(options);
    }
}
