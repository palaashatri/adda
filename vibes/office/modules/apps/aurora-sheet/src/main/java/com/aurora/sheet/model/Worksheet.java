package com.aurora.sheet.model;

import java.util.LinkedHashMap;
import java.util.Locale;
import java.util.Map;

/**
 * Sparse worksheet storage for MVP formula evaluation.
 */
public final class Worksheet {
    private final Map<String, String> cells = new LinkedHashMap<>();

    /**
     * Sets a raw cell value or formula.
     */
    public void setCell(String address, String value) {
        String normalized = normalize(address);
        if (value == null || value.isBlank()) {
            cells.remove(normalized);
        } else {
            cells.put(normalized, value);
        }
    }

    /**
     * Returns the raw cell value or formula.
     */
    public String rawCell(String address) {
        return cells.getOrDefault(normalize(address), "");
    }

    /**
     * Returns a stable snapshot of raw cells.
     */
    public Map<String, String> snapshot() {
        return Map.copyOf(cells);
    }

    private static String normalize(String address) {
        if (address == null || address.isBlank()) {
            throw new IllegalArgumentException("Cell address is required");
        }
        return address.toUpperCase(Locale.ROOT);
    }
}
