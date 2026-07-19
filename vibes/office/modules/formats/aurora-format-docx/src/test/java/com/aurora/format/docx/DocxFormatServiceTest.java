package com.aurora.format.docx;

import com.aurora.platform.core.AuroraDocumentJson;
import com.aurora.platform.core.AuroraJson;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

import java.nio.file.Path;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

class DocxFormatServiceTest {
    @TempDir
    Path tempDir;

    @Test
    void exportsAndImportsWriteDocument() throws Exception {
        DocxFormatService service = new DocxFormatService();
        Path target = tempDir.resolve("sample.docx");

        service.exportWrite(target, AuroraDocumentJson.write("Heading\nBody text"));
        String imported = service.importWrite(target);

        assertTrue(target.toFile().length() > 0);
        assertEquals("Heading\nBody text", AuroraJson.readStringField(imported, "body").orElseThrow());
    }
}
