package com.aurora.platform.core;

import org.junit.jupiter.api.Test;

import java.util.Map;

import static org.junit.jupiter.api.Assertions.assertEquals;

class AuroraDocumentJsonTest {
    @Test
    void roundTripsSheetCells() {
        String json = AuroraDocumentJson.sheet(Map.of("A1", "Name", "B2", "=SUM(1,2)"));

        Map<String, String> cells = AuroraDocumentJson.readSheetCells(json);

        assertEquals("Name", cells.get("A1"));
        assertEquals("=SUM(1,2)", cells.get("B2"));
    }
}
