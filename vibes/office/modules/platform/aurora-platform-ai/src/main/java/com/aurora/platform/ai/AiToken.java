package com.aurora.platform.ai;

/**
 * Streaming token chunk from an AI provider.
 */
public record AiToken(String text, boolean done) {
    public AiToken {
        text = text == null ? "" : text;
    }
}
