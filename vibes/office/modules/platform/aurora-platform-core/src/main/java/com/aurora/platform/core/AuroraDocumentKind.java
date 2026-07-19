package com.aurora.platform.core;

import java.util.Arrays;
import java.util.Locale;
import java.util.Optional;

/**
 * Supported Aurora document families and their native package extensions.
 */
public enum AuroraDocumentKind {
    WRITE("write", "Aurora Write", "Document", "awrite"),
    SHEET("sheet", "Aurora Sheet", "Spreadsheet", "asheet"),
    DECK("deck", "Aurora Deck", "Presentation", "adeck");

    private final String id;
    private final String productName;
    private final String documentLabel;
    private final String extension;

    AuroraDocumentKind(String id, String productName, String documentLabel, String extension) {
        this.id = id;
        this.productName = productName;
        this.documentLabel = documentLabel;
        this.extension = extension;
    }

    /**
     * Stable package identifier stored inside native document metadata.
     */
    public String id() {
        return id;
    }

    /**
     * User-facing app name.
     */
    public String productName() {
        return productName;
    }

    /**
     * User-facing generic document label.
     */
    public String documentLabel() {
        return documentLabel;
    }

    /**
     * Native Aurora package extension without a leading dot.
     */
    public String extension() {
        return extension;
    }

    /**
     * Resolves a document kind from its metadata identifier.
     */
    public static Optional<AuroraDocumentKind> fromId(String id) {
        if (id == null || id.isBlank()) {
            return Optional.empty();
        }
        String normalized = id.toLowerCase(Locale.ROOT);
        return Arrays.stream(values())
                .filter(kind -> kind.id.equals(normalized))
                .findFirst();
    }

    /**
     * Resolves a document kind from a path or extension string.
     */
    public static Optional<AuroraDocumentKind> fromExtension(String pathOrExtension) {
        if (pathOrExtension == null || pathOrExtension.isBlank()) {
            return Optional.empty();
        }
        String normalized = pathOrExtension.toLowerCase(Locale.ROOT);
        int dot = normalized.lastIndexOf('.');
        String extensionOnly = dot >= 0 ? normalized.substring(dot + 1) : normalized;
        return Arrays.stream(values())
                .filter(kind -> kind.extension.equals(extensionOnly))
                .findFirst();
    }
}
