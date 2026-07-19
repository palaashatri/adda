package com.aurora.sheet.formula;

import com.aurora.sheet.model.CellValue;
import com.aurora.sheet.model.Worksheet;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;

class FormulaEngineTest {
    @Test
    void evaluatesRequiredMathFunctionsWithRanges() {
        Worksheet sheet = new Worksheet();
        sheet.setCell("A1", "10");
        sheet.setCell("A2", "20");
        sheet.setCell("B1", "5");
        FormulaEngine engine = new FormulaEngine();

        assertEquals("35", engine.evaluate("=SUM(A1:A2,B1)", sheet).asText());
        assertEquals("5", engine.evaluate("=SUM(A1,-B1)", sheet).asText());
        assertEquals("10", engine.evaluate("=AVERAGE(A1,B1,15)", sheet).asText());
        assertEquals("5", engine.evaluate("=MIN(A1:A2,B1)", sheet).asText());
        assertEquals("20", engine.evaluate("=MAX(A1:A2,B1)", sheet).asText());
        assertEquals("3", engine.evaluate("=COUNT(A1:A2,B1,\"x\")", sheet).asText());
    }

    @Test
    void evaluatesIfConcatRoundAndCellFormulaReferences() {
        Worksheet sheet = new Worksheet();
        sheet.setCell("A1", "=ROUND(3.14159,2)");
        sheet.setCell("B1", "Aurora");
        FormulaEngine engine = new FormulaEngine();

        assertEquals("3.14", engine.evaluate("A1", sheet).asText());
        assertEquals("yes", engine.evaluate("=IF(TRUE,\"yes\",\"no\")", sheet).asText());
        assertEquals("Aurora Office", engine.evaluate("=CONCAT(B1,\" Office\")", sheet).asText());
        assertEquals(CellValue.Type.ERROR, engine.evaluate("=NOPE(1)", sheet).type());
    }
}
