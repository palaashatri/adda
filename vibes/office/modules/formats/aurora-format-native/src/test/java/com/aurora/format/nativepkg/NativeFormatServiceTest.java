package com.aurora.format.nativepkg;

import com.aurora.platform.core.AuroraDocumentKind;
import com.aurora.platform.storage.LoadedAuroraPackage;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

import java.nio.file.Path;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

class NativeFormatServiceTest {
    @TempDir
    Path tempDir;

    @Test
    void roundTripsThroughFormatFacade() throws Exception {
        NativeFormatService service = new NativeFormatService();
        Path target = tempDir.resolve("deck.adeck");

        service.save(target, AuroraDocumentKind.DECK, "Deck", "{\"type\":\"deck\"}");
        LoadedAuroraPackage loaded = service.load(target);

        assertTrue(service.supports(target));
        assertEquals(AuroraDocumentKind.DECK, loaded.kind());
        assertEquals("{\"type\":\"deck\"}", loaded.documentJson());
    }
}
