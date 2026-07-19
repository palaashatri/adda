package com.aurora.platform.ui;

import javafx.geometry.Pos;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.layout.GridPane;

import com.aurora.platform.core.AuroraDocumentJson;

import java.util.LinkedHashMap;
import java.util.Map;

/**
 * Small virtual-grid placeholder for Aurora Sheet MVP editing.
 */
public class AuroraSheetGrid extends GridPane {
    private final Map<String, TextField> cells = new LinkedHashMap<>();
    private String focusedAddress = "A1";

    public AuroraSheetGrid(int rows, int columns) {
        getStyleClass().add("aurora-sheet-grid");
        setHgap(0);
        setVgap(0);

        addHeader("", 0, 0);
        for (int column = 1; column <= columns; column++) {
            addHeader(columnName(column), column, 0);
        }
        for (int row = 1; row <= rows; row++) {
            addHeader(Integer.toString(row), 0, row);
            for (int column = 1; column <= columns; column++) {
                String address = columnName(column) + row;
                TextField field = new TextField();
                field.getStyleClass().add("aurora-sheet-cell");
                field.setPrefColumnCount(10);
                field.setMinWidth(92);
                field.setPrefWidth(112);
                field.setAlignment(Pos.CENTER_LEFT);
                field.focusedProperty().addListener((observable, oldValue, hasFocus) -> {
                    if (hasFocus) {
                        focusedAddress = address;
                    }
                });
                cells.put(address, field);
                add(field, column, row);
            }
        }
    }

    /**
     * Sets a cell value by A1-style address.
     */
    public void putCell(String address, String value) {
        TextField field = cells.get(address);
        if (field != null) {
            field.setText(value == null ? "" : value);
        }
    }

    /**
     * Returns a cell value by A1-style address.
     */
    public String getCell(String address) {
        TextField field = cells.get(address);
        return field == null ? "" : field.getText();
    }

    /**
     * Current focused A1-style cell address.
     */
    public String focusedAddress() {
        return focusedAddress;
    }

    /**
     * Returns a sparse snapshot of non-empty cell values.
     */
    public Map<String, String> snapshot() {
        Map<String, String> snapshot = new LinkedHashMap<>();
        for (Map.Entry<String, TextField> entry : cells.entrySet()) {
            String value = entry.getValue().getText();
            if (value != null && !value.isBlank()) {
                snapshot.put(entry.getKey(), value);
            }
        }
        return snapshot;
    }

    /**
     * Serializes non-empty cells into a compact JSON payload.
     */
    public String toDocumentJson() {
        return AuroraDocumentJson.sheet(snapshot());
    }

    /**
     * Loads cell values from the compact JSON payload emitted by this grid.
     */
    public void loadDocumentJson(String json) {
        cells.values().forEach(field -> field.clear());
        if (json == null || json.isBlank()) {
            return;
        }
        AuroraDocumentJson.readSheetCells(json).forEach(this::putCell);
    }

    private void addHeader(String text, int column, int row) {
        Label label = new Label(text);
        label.getStyleClass().add("aurora-sheet-header");
        label.setMinWidth(column == 0 ? 44 : 92);
        label.setPrefWidth(column == 0 ? 44 : 112);
        label.setAlignment(Pos.CENTER);
        add(label, column, row);
    }

    private static String columnName(int column) {
        int value = column;
        StringBuilder builder = new StringBuilder();
        while (value > 0) {
            value--;
            builder.insert(0, (char) ('A' + value % 26));
            value /= 26;
        }
        return builder.toString();
    }

}
