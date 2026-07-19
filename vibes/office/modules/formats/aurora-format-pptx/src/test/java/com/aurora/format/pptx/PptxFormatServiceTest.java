package com.aurora.format.pptx;

import com.aurora.platform.core.AuroraDocumentJson;
import com.aurora.platform.core.AuroraJson;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

import java.nio.file.Path;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

class PptxFormatServiceTest {
    @TempDir
    Path tempDir;

    @Test
    void exportsAndImportsDeckSlideText() throws Exception {
        PptxFormatService service = new PptxFormatService();
        Path target = tempDir.resolve("sample.pptx");

        service.exportDeck(target, AuroraDocumentJson.deck("Quarterly Plan", "Ship the MVP"));
        String imported = service.importDeck(target);

        assertTrue(target.toFile().length() > 0);
        assertEquals("Quarterly Plan", AuroraJson.readStringField(imported, "title").orElseThrow());
        assertEquals("Ship the MVP", AuroraJson.readStringField(imported, "body").orElseThrow());
    }
}
