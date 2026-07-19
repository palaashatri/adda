package com.aurora.platform.storage;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Properties;

/**
 * Persists local Aurora settings as a Java properties file.
 */
public final class AuroraSettingsStore {
    private final Path settingsFile;

    public AuroraSettingsStore(Path settingsFile) {
        this.settingsFile = settingsFile;
    }

    /**
     * Loads settings or returns privacy-first defaults when no file exists.
     */
    public AuroraSettings load() throws IOException {
        if (!Files.exists(settingsFile)) {
            return AuroraSettings.defaults();
        }
        Properties properties = new Properties();
        try (InputStream input = Files.newInputStream(settingsFile)) {
            properties.load(input);
        }
        return new AuroraSettings(
                properties.getProperty("ai.provider", "None"),
                properties.getProperty("ai.endpoint", ""),
                properties.getProperty("ai.model", "gemma-4-e4b-it"),
                properties.getProperty("ai.apiKey", ""),
                Boolean.parseBoolean(properties.getProperty("ai.allowModelDownloads", "false")),
                Boolean.parseBoolean(properties.getProperty("ai.redactSecrets", "true")),
                properties.getProperty("ui.theme", "macos-light")
        );
    }

    /**
     * Saves settings locally.
     */
    public void save(AuroraSettings settings) throws IOException {
        Files.createDirectories(settingsFile.toAbsolutePath().getParent());
        Properties properties = new Properties();
        properties.setProperty("ai.provider", settings.aiProvider());
        properties.setProperty("ai.endpoint", settings.aiEndpoint());
        properties.setProperty("ai.model", settings.aiModel());
        properties.setProperty("ai.apiKey", settings.aiApiKey());
        properties.setProperty("ai.allowModelDownloads", Boolean.toString(settings.allowModelDownloads()));
        properties.setProperty("ai.redactSecrets", Boolean.toString(settings.redactSecrets()));
        properties.setProperty("ui.theme", settings.theme());
        try (OutputStream output = Files.newOutputStream(settingsFile)) {
            properties.store(output, "Aurora Office settings");
        }
    }
}
