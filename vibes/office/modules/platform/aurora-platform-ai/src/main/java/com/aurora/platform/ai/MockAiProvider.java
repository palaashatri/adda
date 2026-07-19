package com.aurora.platform.ai;

import java.util.concurrent.Flow;

/**
 * Deterministic local provider for tests and privacy-first default UI states.
 */
public final class MockAiProvider implements AiProvider {
    @Override
    public String id() {
        return "mock-local";
    }

    @Override
    public boolean supportsStreaming() {
        return true;
    }

    @Override
    public AiResponse complete(AiRequest request) {
        String prompt = request.prompt().isBlank() ? "No prompt supplied." : request.prompt();
        String text = "Mock suggestion: " + prompt;
        return new AiResponse(id(), text, tokenEstimate(prompt), tokenEstimate(text));
    }

    @Override
    public Flow.Publisher<AiToken> stream(AiRequest request) {
        AiResponse response = complete(request);
        return subscriber -> subscriber.onSubscribe(new Flow.Subscription() {
            private boolean delivered;

            @Override
            public void request(long n) {
                if (delivered || n <= 0) {
                    return;
                }
                delivered = true;
                subscriber.onNext(new AiToken(response.text(), false));
                subscriber.onNext(new AiToken("", true));
                subscriber.onComplete();
            }

            @Override
            public void cancel() {
                delivered = true;
            }
        });
    }

    private static int tokenEstimate(String text) {
        if (text == null || text.isBlank()) {
            return 0;
        }
        return Math.max(1, text.trim().split("\\s+").length);
    }
}
