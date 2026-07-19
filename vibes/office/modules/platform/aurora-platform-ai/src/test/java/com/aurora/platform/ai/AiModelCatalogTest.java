package com.aurora.platform.ai;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

class AiModelCatalogTest {
    @Test
    void exposesGemmaAndUserSuppliedProviderProfiles() {
        assertTrue(AiModelCatalog.providers().contains(AiModelCatalog.PROVIDER_GEMMA4_LOCAL));
        assertTrue(AiModelCatalog.providers().contains(AiModelCatalog.PROVIDER_OLLAMA));
        assertTrue(AiModelCatalog.providers().contains(AiModelCatalog.PROVIDER_LM_STUDIO));
        assertTrue(AiModelCatalog.providers().contains(AiModelCatalog.PROVIDER_VLLM));
        assertTrue(AiModelCatalog.providers().contains(AiModelCatalog.PROVIDER_LLAMA_CPP));
        assertTrue(AiModelCatalog.providers().contains(AiModelCatalog.PROVIDER_OPENAI_COMPATIBLE));
        assertTrue(AiModelCatalog.requiresDownloadConsent(AiModelCatalog.PROVIDER_GEMMA4_LOCAL));
    }

    @Test
    void knowsDefaultLocalEndpoints() {
        assertEquals("gemma4:e4b", AiModelCatalog.DEFAULT_GEMMA4_MODEL);
        assertEquals("http://localhost:11434/api/generate", AiModelCatalog.defaultEndpoint(AiModelCatalog.PROVIDER_GEMMA4_LOCAL));
        assertEquals("http://localhost:11434/api/generate", AiModelCatalog.defaultEndpoint(AiModelCatalog.PROVIDER_OLLAMA));
        assertEquals("http://localhost:1234/v1/chat/completions", AiModelCatalog.defaultEndpoint(AiModelCatalog.PROVIDER_LM_STUDIO));
        assertTrue(AiModelCatalog.isOllamaNative(AiModelCatalog.PROVIDER_GEMMA4_LOCAL, "http://localhost:11434/api/generate"));
    }
}
