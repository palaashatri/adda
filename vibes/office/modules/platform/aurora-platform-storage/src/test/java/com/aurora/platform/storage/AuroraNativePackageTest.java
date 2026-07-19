package com.aurora.platform.storage;

import com.aurora.platform.core.AuroraDocumentKind;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

import java.nio.file.Path;
import java.util.zip.ZipFile;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;

class AuroraNativePackageTest {
    @TempDir
    Path tempDir;

    @Test
    void saveAndLoadRoundTripsDocumentJson() throws Exception {
        Path target = tempDir.resolve("example.awrite");
        String documentJson = "{\"type\":\"write\",\"body\":\"Hello Aurora\"}";

        AuroraNativePackage.save(target, AuroraDocumentKind.WRITE, "Example", documentJson);
        LoadedAuroraPackage loaded = AuroraNativePackage.load(target);

        assertEquals(AuroraDocumentKind.WRITE, loaded.kind());
        assertEquals("Example", loaded.title());
        assertEquals(documentJson, loaded.documentJson());
    }

    @Test
    void packageContainsRequiredMvpEntries() throws Exception {
        Path target = tempDir.resolve("example.asheet");

        AuroraNativePackage.save(target, AuroraDocumentKind.SHEET, "Sheet", "{\"type\":\"sheet\"}");

        try (ZipFile zip = new ZipFile(target.toFile())) {
            assertNotNull(zip.getEntry("document.json"));
            assertNotNull(zip.getEntry("styles.json"));
            assertNotNull(zip.getEntry("metadata.json"));
            assertNotNull(zip.getEntry("assets/"));
            assertNotNull(zip.getEntry("history/"));
            assertNotNull(zip.getEntry("embeddings/"));
        }
    }
}
