package com.aurora.format.xlsx;

import com.aurora.platform.core.AuroraDocumentJson;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

import java.nio.file.Path;
import java.util.Map;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTrue;

class XlsxFormatServiceTest {
    @TempDir
    Path tempDir;

    @Test
    void exportsAndImportsSparseSheetCellsAndFormulas() throws Exception {
        XlsxFormatService service = new XlsxFormatService();
        Path target = tempDir.resolve("sample.xlsx");

        service.exportSheet(target, AuroraDocumentJson.sheet(Map.of(
                "A1", "Metric",
                "B2", "42",
                "C2", "=SUM(B2,8)"
        )));
        String imported = service.importSheet(target);
        Map<String, String> cells = AuroraDocumentJson.readSheetCells(imported);

        assertTrue(target.toFile().length() > 0);
        assertEquals("Metric", cells.get("A1"));
        assertEquals("42", cells.get("B2"));
        assertEquals("=SUM(B2,8)", cells.get("C2"));
    }
}
