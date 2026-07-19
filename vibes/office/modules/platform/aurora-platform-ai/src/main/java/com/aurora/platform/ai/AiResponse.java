package com.aurora.platform.ai;

/**
 * Provider response with optional token accounting.
 */
public record AiResponse(String providerId, String text, int promptTokens, int completionTokens) {
    public AiResponse {
        providerId = providerId == null ? "unknown" : providerId;
        text = text == null ? "" : text;
    }
}
