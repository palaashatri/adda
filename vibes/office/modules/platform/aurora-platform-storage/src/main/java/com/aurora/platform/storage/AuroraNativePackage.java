package com.aurora.platform.storage;

import com.aurora.platform.core.AuroraDocumentKind;
import com.aurora.platform.core.AuroraVersion;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;
import java.time.Instant;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import java.util.zip.ZipOutputStream;

/**
 * Reads and writes Aurora ZIP-based native packages.
 */
public final class AuroraNativePackage {
    private AuroraNativePackage() {
    }

    /**
     * Saves a native package using the MVP package layout.
     */
    public static void save(Path target, AuroraDocumentKind kind, String title, String documentJson) throws IOException {
        Objects.requireNonNull(target, "target");
        Objects.requireNonNull(kind, "kind");

        Path absoluteTarget = target.toAbsolutePath();
        Path parent = absoluteTarget.getParent();
        if (parent != null) {
            Files.createDirectories(parent);
        }

        Path temp = Files.createTempFile(parent == null ? Path.of(".") : parent, ".aurora-", ".tmp");
        try {
            try (ZipOutputStream zip = new ZipOutputStream(Files.newOutputStream(temp), StandardCharsets.UTF_8)) {
                writeText(zip, "document.json", normalizeDocumentJson(documentJson));
                writeText(zip, "styles.json", defaultStylesJson());
                writeText(zip, "metadata.json", metadataJson(kind, title));
                writeDirectory(zip, "assets/");
                writeDirectory(zip, "history/");
                writeDirectory(zip, "embeddings/");
            }
            try {
                Files.move(temp, absoluteTarget, StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);
            } catch (IOException atomicMoveFailure) {
                Files.move(temp, absoluteTarget, StandardCopyOption.REPLACE_EXISTING);
            }
        } finally {
            Files.deleteIfExists(temp);
        }
    }

    /**
     * Loads a native package and returns its metadata plus document payload.
     */
    public static LoadedAuroraPackage load(Path source) throws IOException {
        Objects.requireNonNull(source, "source");
        try (ZipFile zip = new ZipFile(source.toFile(), StandardCharsets.UTF_8)) {
            String documentJson = readRequiredText(zip, "document.json");
            String metadataJson = readRequiredText(zip, "metadata.json");
            Map<String, String> metadata = parseFlatJson(metadataJson);
            AuroraDocumentKind kind = AuroraDocumentKind.fromId(metadata.get("kind"))
                    .or(() -> AuroraDocumentKind.fromExtension(source.toString()))
                    .orElse(AuroraDocumentKind.WRITE);
            return new LoadedAuroraPackage(kind, metadata.getOrDefault("title", "Untitled"), documentJson, metadata);
        }
    }

    private static void writeText(ZipOutputStream zip, String name, String text) throws IOException {
        ZipEntry entry = new ZipEntry(name);
        zip.putNextEntry(entry);
        zip.write(text.getBytes(StandardCharsets.UTF_8));
        zip.closeEntry();
    }

    private static void writeDirectory(ZipOutputStream zip, String name) throws IOException {
        ZipEntry entry = new ZipEntry(name);
        zip.putNextEntry(entry);
        zip.closeEntry();
    }

    private static String readRequiredText(ZipFile zip, String name) throws IOException {
        ZipEntry entry = Optional.ofNullable(zip.getEntry(name))
                .orElseThrow(() -> new IOException("Missing required entry: " + name));
        try (InputStream input = zip.getInputStream(entry)) {
            ByteArrayOutputStream output = new ByteArrayOutputStream();
            input.transferTo(output);
            return output.toString(StandardCharsets.UTF_8);
        }
    }

    private static String normalizeDocumentJson(String documentJson) {
        if (documentJson == null || documentJson.isBlank()) {
            return "{}";
        }
        return documentJson;
    }

    private static String defaultStylesJson() {
        return """
                {
                  "formatVersion": 1,
                  "theme": "macos-light",
                  "styles": []
                }
                """;
    }

    private static String metadataJson(AuroraDocumentKind kind, String title) {
        String safeTitle = title == null || title.isBlank() ? "Untitled " + kind.documentLabel() : title;
        Instant now = Instant.now();
        return """
                {
                  "formatVersion": %d,
                  "application": "%s",
                  "kind": "%s",
                  "title": "%s",
                  "createdAt": "%s",
                  "modifiedAt": "%s"
                }
                """.formatted(
                AuroraVersion.NATIVE_FORMAT_VERSION,
                escapeJson(AuroraVersion.PRODUCT_NAME),
                escapeJson(kind.id()),
                escapeJson(safeTitle),
                now,
                now
        );
    }

    private static Map<String, String> parseFlatJson(String json) {
        Map<String, String> values = new LinkedHashMap<>();
        if (json == null || json.isBlank()) {
            return values;
        }

        int index = 0;
        while (index < json.length()) {
            int keyStart = json.indexOf('"', index);
            if (keyStart < 0) {
                break;
            }
            int keyEnd = findStringEnd(json, keyStart + 1);
            if (keyEnd < 0) {
                break;
            }
            int colon = json.indexOf(':', keyEnd);
            if (colon < 0) {
                break;
            }
            int valueStart = skipWhitespace(json, colon + 1);
            if (valueStart < json.length() && json.charAt(valueStart) == '"') {
                int valueEnd = findStringEnd(json, valueStart + 1);
                if (valueEnd < 0) {
                    break;
                }
                values.put(unescapeJson(json.substring(keyStart + 1, keyEnd)),
                        unescapeJson(json.substring(valueStart + 1, valueEnd)));
                index = valueEnd + 1;
            } else {
                int valueEnd = valueStart;
                while (valueEnd < json.length() && ",}\n\r\t ".indexOf(json.charAt(valueEnd)) < 0) {
                    valueEnd++;
                }
                values.put(unescapeJson(json.substring(keyStart + 1, keyEnd)),
                        json.substring(valueStart, valueEnd));
                index = valueEnd + 1;
            }
        }
        return values;
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

    private static String escapeJson(String value) {
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

    private static String unescapeJson(String value) {
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
}
