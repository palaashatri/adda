package com.aurora.tests.performance;

import com.aurora.sheet.formula.FormulaEngine;
import com.aurora.sheet.model.Worksheet;
import org.junit.jupiter.api.Test;

import java.time.Duration;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTimeoutPreemptively;

class LargeSheetSmokeTest {
    @Test
    void sparseWorksheetHandlesMillionRowAddresses() {
        assertTimeoutPreemptively(Duration.ofSeconds(2), () -> {
            Worksheet worksheet = new Worksheet();
            worksheet.setCell("A1", "10");
            worksheet.setCell("A1000000", "32");
            FormulaEngine engine = new FormulaEngine();

            assertEquals("42", engine.evaluate("=SUM(A1,A1000000)", worksheet).asText());
            assertEquals(2, worksheet.snapshot().size());
        });
    }
}
