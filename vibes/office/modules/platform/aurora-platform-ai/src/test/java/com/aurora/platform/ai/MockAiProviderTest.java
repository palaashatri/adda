package com.aurora.platform.ai;

import org.junit.jupiter.api.Test;

import java.util.Map;
import java.util.concurrent.Flow;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicReference;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

class MockAiProviderTest {
    @Test
    void completeReturnsDeterministicLocalSuggestion() {
        MockAiProvider provider = new MockAiProvider();

        AiResponse response = provider.complete(new AiRequest("Rewrite this", Map.of()));

        assertEquals("mock-local", response.providerId());
        assertEquals("Mock suggestion: Rewrite this", response.text());
        assertTrue(response.promptTokens() > 0);
        assertTrue(response.completionTokens() > 0);
    }

    @Test
    void streamEmitsResponseAndCompletionToken() {
        MockAiProvider provider = new MockAiProvider();
        AtomicReference<String> text = new AtomicReference<>("");
        AtomicBoolean done = new AtomicBoolean(false);

        provider.stream(new AiRequest("Explain", null)).subscribe(new Flow.Subscriber<>() {
            @Override
            public void onSubscribe(Flow.Subscription subscription) {
                subscription.request(Long.MAX_VALUE);
            }

            @Override
            public void onNext(AiToken item) {
                if (item.done()) {
                    done.set(true);
                } else {
                    text.set(item.text());
                }
            }

            @Override
            public void onError(Throwable throwable) {
                throw new AssertionError(throwable);
            }

            @Override
            public void onComplete() {
            }
        });

        assertEquals("Mock suggestion: Explain", text.get());
        assertTrue(done.get());
    }
}
