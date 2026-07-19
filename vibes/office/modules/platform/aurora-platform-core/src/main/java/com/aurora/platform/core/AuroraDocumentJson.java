package com.aurora.platform.core;

import java.util.LinkedHashMap;
import java.util.Map;

/**
 * Helpers for MVP document JSON payloads.
 */
public final class AuroraDocumentJson {
    private AuroraDocumentJson() {
    }

    /**
     * Creates Write document JSON.
     */
    public static String write(String body) {
        return "{\"type\":\"write\",\"body\":\"" + AuroraJson.escape(body) + "\"}";
    }

    /**
     * Creates Deck document JSON.
     */
    public static String deck(String title, String body) {
        return "{\"type\":\"deck\",\"title\":\"" + AuroraJson.escape(title) + "\",\"body\":\""
                + AuroraJson.escape(body) + "\"}";
    }

    /**
     * Creates Sheet document JSON from sparse cells.
     */
    public static String sheet(Map<String, String> cells) {
        StringBuilder builder = new StringBuilder();
        builder.append("{\"type\":\"sheet\",\"cells\":{");
        boolean first = true;
        for (Map.Entry<String, String> entry : cells.entrySet()) {
            String value = entry.getValue();
            if (value == null || value.isBlank()) {
                continue;
            }
            if (!first) {
                builder.append(',');
            }
            first = false;
            builder.append('"').append(AuroraJson.escape(entry.getKey())).append("\":\"")
                    .append(AuroraJson.escape(value)).append('"');
        }
        builder.append("}}");
        return builder.toString();
    }

    /**
     * Reads sparse sheet cells from an MVP sheet JSON payload.
     */
    public static Map<String, String> readSheetCells(String json) {
        Map<String, String> cells = new LinkedHashMap<>();
        if (json == null || json.isBlank()) {
            return cells;
        }
        int cellsStart = json.indexOf("\"cells\"");
        if (cellsStart < 0) {
            return cells;
        }
        int objectStart = json.indexOf('{', cellsStart);
        int objectEnd = findMatchingBrace(json, objectStart);
        if (objectStart < 0 || objectEnd < 0) {
            return cells;
        }
        String body = json.substring(objectStart + 1, objectEnd);
        int index = 0;
        while (index < body.length()) {
            int keyStart = body.indexOf('"', index);
            if (keyStart < 0) {
                break;
            }
            int keyEnd = findStringEnd(body, keyStart + 1);
            int valueStart = body.indexOf('"', keyEnd + 1);
            int valueEnd = findStringEnd(body, valueStart + 1);
            if (keyEnd < 0 || valueStart < 0 || valueEnd < 0) {
                break;
            }
            cells.put(
                    AuroraJson.unescape(body.substring(keyStart + 1, keyEnd)),
                    AuroraJson.unescape(body.substring(valueStart + 1, valueEnd))
            );
            index = valueEnd + 1;
        }
        return cells;
    }

    private static int findMatchingBrace(String text, int start) {
        if (start < 0) {
            return -1;
        }
        boolean inString = false;
        boolean escaped = false;
        int depth = 0;
        for (int index = start; index < text.length(); index++) {
            char current = text.charAt(index);
            if (escaped) {
                escaped = false;
                continue;
            }
            if (current == '\\') {
                escaped = true;
                continue;
            }
            if (current == '"') {
                inString = !inString;
                continue;
            }
            if (inString) {
                continue;
            }
            if (current == '{') {
                depth++;
            } else if (current == '}') {
                depth--;
                if (depth == 0) {
                    return index;
                }
            }
        }
        return -1;
    }

    private static int findStringEnd(String text, int start) {
        boolean escaped = false;
        for (int index = start; index < text.length(); index++) {
            char current = text.charAt(index);
            if (escaped) {
                escaped = false;
            } else if (current == '\\') {
                escaped = true;
            } else if (current == '"') {
                return index;
            }
        }
        return -1;
    }
}
