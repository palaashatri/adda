package com.aurora.platform.ai;

import java.util.List;

/**
 * Built-in AI provider and model names shown in settings.
 */
public final class AiModelCatalog {
    public static final String PROVIDER_NONE = "None";
    public static final String PROVIDER_GEMMA4_LOCAL = "Gemma 4 local";
    public static final String PROVIDER_OLLAMA = "Ollama";
    public static final String PROVIDER_LM_STUDIO = "LM Studio";
    public static final String PROVIDER_VLLM = "vLLM";
    public static final String PROVIDER_LLAMA_CPP = "llama.cpp";
    public static final String PROVIDER_OPENAI_COMPATIBLE = "OpenAI-compatible";
    public static final String PROVIDER_CUSTOM_HTTP = "Custom HTTP endpoint";
    public static final String DEFAULT_GEMMA4_MODEL = "gemma4:e4b";

    private AiModelCatalog() {
    }

    /**
     * Providers supported by the settings UI.
     */
    public static List<String> providers() {
        return List.of(
                PROVIDER_NONE,
                PROVIDER_GEMMA4_LOCAL,
                PROVIDER_OLLAMA,
                PROVIDER_LM_STUDIO,
                PROVIDER_VLLM,
                PROVIDER_LLAMA_CPP,
                PROVIDER_OPENAI_COMPATIBLE,
                PROVIDER_CUSTOM_HTTP
        );
    }

    /**
     * Default endpoint for local provider profiles.
     */
    public static String defaultEndpoint(String provider) {
        return switch (provider) {
            case PROVIDER_GEMMA4_LOCAL -> "http://localhost:11434/api/generate";
            case PROVIDER_OLLAMA -> "http://localhost:11434/api/generate";
            case PROVIDER_LM_STUDIO -> "http://localhost:1234/v1/chat/completions";
            case PROVIDER_VLLM -> "http://localhost:8000/v1/chat/completions";
            case PROVIDER_LLAMA_CPP -> "http://localhost:8080/v1/chat/completions";
            default -> "";
        };
    }

    /**
     * Whether the profile uses Ollama's native generation endpoint by default.
     */
    public static boolean isOllamaNative(String provider, String endpoint) {
        return (PROVIDER_OLLAMA.equals(provider) || PROVIDER_GEMMA4_LOCAL.equals(provider))
                && (endpoint == null || endpoint.endsWith("/api/generate"));
    }

    /**
     * Whether selecting this provider can require model download/install consent.
     */
    public static boolean requiresDownloadConsent(String provider) {
        return PROVIDER_GEMMA4_LOCAL.equals(provider);
    }
}
