package com.aurora.platform.core;

import java.util.Optional;

/**
 * Minimal JSON string helpers for MVP document payloads.
 */
public final class AuroraJson {
    private AuroraJson() {
    }

    /**
     * Escapes a string for JSON output.
     */
    public static String escape(String value) {
        if (value == null) {
            return "";
        }
        StringBuilder builder = new StringBuilder();
        for (int index = 0; index < value.length(); index++) {
            char current = value.charAt(index);
            switch (current) {
                case '"' -> builder.append("\\\"");
                case '\\' -> builder.append("\\\\");
                case '\b' -> builder.append("\\b");
                case '\f' -> builder.append("\\f");
                case '\n' -> builder.append("\\n");
                case '\r' -> builder.append("\\r");
                case '\t' -> builder.append("\\t");
                default -> builder.append(current);
            }
        }
        return builder.toString();
    }

    /**
     * Reads a top-level string field from a simple JSON object.
     */
    public static Optional<String> readStringField(String json, String fieldName) {
        if (json == null || fieldName == null || fieldName.isBlank()) {
            return Optional.empty();
        }
        String needle = "\"" + escape(fieldName) + "\"";
        int keyStart = json.indexOf(needle);
        if (keyStart < 0) {
            return Optional.empty();
        }
        int colon = json.indexOf(':', keyStart + needle.length());
        if (colon < 0) {
            return Optional.empty();
        }
        int valueStart = skipWhitespace(json, colon + 1);
        if (valueStart >= json.length() || json.charAt(valueStart) != '"') {
            return Optional.empty();
        }
        int valueEnd = findStringEnd(json, valueStart + 1);
        if (valueEnd < 0) {
            return Optional.empty();
        }
        return Optional.of(unescape(json.substring(valueStart + 1, valueEnd)));
    }

    /**
     * Unescapes a JSON string literal body.
     */
    public static String unescape(String value) {
        if (value == null || value.isEmpty()) {
            return "";
        }
        StringBuilder builder = new StringBuilder();
        boolean escaped = false;
        for (int index = 0; index < value.length(); index++) {
            char current = value.charAt(index);
            if (!escaped) {
                if (current == '\\') {
                    escaped = true;
                } else {
                    builder.append(current);
                }
                continue;
            }
            switch (current) {
                case '"' -> builder.append('"');
                case '\\' -> builder.append('\\');
                case 'b' -> builder.append('\b');
                case 'f' -> builder.append('\f');
                case 'n' -> builder.append('\n');
                case 'r' -> builder.append('\r');
                case 't' -> builder.append('\t');
                default -> builder.append(current);
            }
            escaped = false;
        }
        if (escaped) {
            builder.append('\\');
        }
        return builder.toString();
    }

    private static int skipWhitespace(String text, int index) {
        int cursor = index;
        while (cursor < text.length() && Character.isWhitespace(text.charAt(cursor))) {
            cursor++;
        }
        return cursor;
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
