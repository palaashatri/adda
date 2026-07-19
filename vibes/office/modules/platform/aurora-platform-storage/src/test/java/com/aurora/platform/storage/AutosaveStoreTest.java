package com.aurora.platform.storage;

import com.aurora.platform.core.AuroraDocumentKind;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

import java.nio.file.Path;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;

class AutosaveStoreTest {
    @TempDir
    Path tempDir;

    @Test
    void savesNativeAutosaveSnapshot() throws Exception {
        AutosaveStore store = new AutosaveStore(tempDir.resolve("autosave"));

        Path snapshot = store.saveSnapshot(AuroraDocumentKind.WRITE, "Draft", "{\"type\":\"write\"}");

        assertTrue(snapshot.toString().endsWith(".awrite"));
        assertFalse(store.listSnapshots().isEmpty());
        assertEquals(AuroraDocumentKind.WRITE, AuroraNativePackage.load(snapshot).kind());
    }
}
