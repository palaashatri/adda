package com.aurora.format.xlsx;

import com.aurora.platform.core.AuroraDocumentJson;
import org.apache.poi.ss.usermodel.Cell;
import org.apache.poi.ss.usermodel.CellType;
import org.apache.poi.ss.usermodel.Row;
import org.apache.poi.ss.usermodel.Sheet;
import org.apache.poi.ss.usermodel.Workbook;
import org.apache.poi.ss.util.CellAddress;
import org.apache.poi.xssf.usermodel.XSSFWorkbook;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.LinkedHashMap;
import java.util.Map;

/**
 * Basic XLSX import/export for Aurora Sheet MVP payloads.
 */
public final class XlsxFormatService {
    /**
     * Exports Sheet JSON to a workbook with one worksheet.
     */
    public void exportSheet(Path target, String sheetDocumentJson) throws IOException {
        Files.createDirectories(target.toAbsolutePath().getParent());
        Map<String, String> cells = AuroraDocumentJson.readSheetCells(sheetDocumentJson);
        try (Workbook workbook = new XSSFWorkbook();
             OutputStream output = Files.newOutputStream(target)) {
            Sheet sheet = workbook.createSheet("Sheet 1");
            for (Map.Entry<String, String> entry : cells.entrySet()) {
                CellAddress address = new CellAddress(entry.getKey());
                Row row = sheet.getRow(address.getRow());
                if (row == null) {
                    row = sheet.createRow(address.getRow());
                }
                Cell cell = row.createCell(address.getColumn());
                writeCell(cell, entry.getValue());
            }
            workbook.write(output);
        }
    }

    /**
     * Imports the first worksheet in an XLSX file into Sheet JSON.
     */
    public String importSheet(Path source) throws IOException {
        try (InputStream input = Files.newInputStream(source);
             Workbook workbook = new XSSFWorkbook(input)) {
            Map<String, String> cells = new LinkedHashMap<>();
            Sheet sheet = workbook.getNumberOfSheets() == 0 ? null : workbook.getSheetAt(0);
            if (sheet == null) {
                return AuroraDocumentJson.sheet(cells);
            }
            for (Row row : sheet) {
                for (Cell cell : row) {
                    String value = readCell(cell);
                    if (!value.isBlank()) {
                        cells.put(cell.getAddress().formatAsString(), value);
                    }
                }
            }
            return AuroraDocumentJson.sheet(cells);
        }
    }

    private void writeCell(Cell cell, String value) {
        if (value == null || value.isBlank()) {
            cell.setBlank();
            return;
        }
        if (value.startsWith("=") && value.length() > 1) {
            cell.setCellFormula(value.substring(1));
            return;
        }
        try {
            cell.setCellValue(Double.parseDouble(value));
        } catch (NumberFormatException exception) {
            cell.setCellValue(value);
        }
    }

    private String readCell(Cell cell) {
        CellType type = cell.getCellType();
        return switch (type) {
            case STRING -> cell.getStringCellValue();
            case NUMERIC -> {
                double number = cell.getNumericCellValue();
                if (Math.rint(number) == number) {
                    yield Long.toString((long) number);
                }
                yield Double.toString(number);
            }
            case BOOLEAN -> Boolean.toString(cell.getBooleanCellValue());
            case FORMULA -> "=" + cell.getCellFormula();
            case BLANK, _NONE, ERROR -> "";
        };
    }
}
