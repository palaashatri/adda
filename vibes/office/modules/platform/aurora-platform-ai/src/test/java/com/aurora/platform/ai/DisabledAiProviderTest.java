package com.aurora.platform.ai;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;

class DisabledAiProviderTest {
    @Test
    void returnsConfigurationMessageWithoutSendingPrompt() {
        DisabledAiProvider provider = new DisabledAiProvider("Consent required");

        AiResponse response = provider.complete(new AiRequest("private selected text", null));

        assertEquals("disabled", response.providerId());
        assertEquals("Consent required", response.text());
        assertEquals(0, response.promptTokens());
    }
}
