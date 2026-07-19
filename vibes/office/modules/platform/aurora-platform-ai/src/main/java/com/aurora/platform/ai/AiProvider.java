package com.aurora.platform.ai;

import java.util.concurrent.Flow;

/**
 * Provider-neutral contract for local or OpenAI-compatible AI completions.
 */
public interface AiProvider {
    /**
     * Stable provider identifier shown in settings and telemetry-free status UI.
     */
    String id();

    /**
     * Whether the provider can stream token chunks.
     */
    boolean supportsStreaming();

    /**
     * Completes a request without mutating document content.
     */
    AiResponse complete(AiRequest request);

    /**
     * Streams a request as token chunks.
     */
    Flow.Publisher<AiToken> stream(AiRequest request);
}
