package com.aurora.platform.ai;

import java.util.concurrent.Flow;

/**
 * Provider used when AI is intentionally disabled or not yet configured.
 */
public final class DisabledAiProvider implements AiProvider {
    private final String reason;

    public DisabledAiProvider(String reason) {
        this.reason = reason == null || reason.isBlank() ? "AI provider is not configured." : reason;
    }

    @Override
    public String id() {
        return "disabled";
    }

    @Override
    public boolean supportsStreaming() {
        return false;
    }

    @Override
    public AiResponse complete(AiRequest request) {
        return new AiResponse(id(), reason, 0, 0);
    }

    @Override
    public Flow.Publisher<AiToken> stream(AiRequest request) {
        return subscriber -> subscriber.onSubscribe(new Flow.Subscription() {
            private boolean delivered;

            @Override
            public void request(long n) {
                if (delivered || n <= 0) {
                    return;
                }
                delivered = true;
                subscriber.onNext(new AiToken(reason, true));
                subscriber.onComplete();
            }

            @Override
            public void cancel() {
                delivered = true;
            }
        });
    }
}
