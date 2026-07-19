package com.aurora.format.pdf;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;

import static org.junit.jupiter.api.Assertions.assertTrue;

class SimplePdfExporterTest {
    @TempDir
    Path tempDir;

    @Test
    void exportsPdfSmokeFile() throws Exception {
        Path target = tempDir.resolve("export.pdf");

        new SimplePdfExporter().exportText(target, "Aurora", "Hello PDF");

        String pdf = Files.readString(target, StandardCharsets.US_ASCII);
        assertTrue(pdf.startsWith("%PDF-1.4"));
        assertTrue(pdf.contains("Hello PDF"));
        assertTrue(pdf.endsWith("%%EOF\n"));
    }
}
