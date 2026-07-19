package com.aurora.platform.storage;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

import java.nio.file.Path;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

class AuroraSettingsStoreTest {
    @TempDir
    Path tempDir;

    @Test
    void loadDefaultsWhenSettingsFileIsMissing() throws Exception {
        AuroraSettingsStore store = new AuroraSettingsStore(tempDir.resolve("settings.properties"));

        AuroraSettings settings = store.load();

        assertEquals("None", settings.aiProvider());
        assertEquals("gemma4:e4b", settings.aiModel());
        assertTrue(settings.redactSecrets());
        assertEquals("macos-light", settings.theme());
    }

    @Test
    void saveAndLoadSettings() throws Exception {
        AuroraSettingsStore store = new AuroraSettingsStore(tempDir.resolve("settings.properties"));

        store.save(new AuroraSettings("Ollama", "http://localhost:11434", "gemma4:e4b", "secret", true, false, "macos-dark"));
        AuroraSettings loaded = store.load();

        assertEquals("Ollama", loaded.aiProvider());
        assertEquals("http://localhost:11434", loaded.aiEndpoint());
        assertEquals("gemma4:e4b", loaded.aiModel());
        assertEquals("secret", loaded.aiApiKey());
        assertTrue(loaded.allowModelDownloads());
        assertEquals("macos-dark", loaded.theme());
    }
}
