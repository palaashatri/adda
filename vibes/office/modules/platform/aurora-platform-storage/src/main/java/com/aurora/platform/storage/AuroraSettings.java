package com.aurora.platform.storage;

/**
 * Local Aurora application settings.
 */
public record AuroraSettings(
        String aiProvider,
        String aiEndpoint,
        String aiModel,
        String aiApiKey,
        boolean allowModelDownloads,
        boolean redactSecrets,
        String theme
) {
    public AuroraSettings(String aiProvider, String aiEndpoint, boolean redactSecrets, String theme) {
        this(aiProvider, aiEndpoint, "", "", false, redactSecrets, theme);
    }

    public AuroraSettings {
        aiProvider = aiProvider == null || aiProvider.isBlank() ? "None" : aiProvider;
        aiEndpoint = aiEndpoint == null ? "" : aiEndpoint;
        aiModel = aiModel == null || aiModel.isBlank() ? "gemma4:e4b" : aiModel;
        aiApiKey = aiApiKey == null ? "" : aiApiKey;
        theme = theme == null || theme.isBlank() ? "macos-light" : theme;
    }

    /**
     * Default privacy-first settings.
     */
    public static AuroraSettings defaults() {
        return new AuroraSettings("None", "", "gemma4:e4b", "", false, true, "macos-light");
    }
}
