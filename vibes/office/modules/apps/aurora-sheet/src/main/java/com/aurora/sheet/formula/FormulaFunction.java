package com.aurora.sheet.formula;

import com.aurora.sheet.model.CellValue;

import java.util.List;

/**
 * Spreadsheet formula function.
 */
@FunctionalInterface
public interface FormulaFunction {
    /**
     * Evaluates this function with already-resolved arguments.
     */
    CellValue evaluate(List<CellValue> args);
}
