package com.aurora.platform.storage;

import com.aurora.platform.core.AuroraDocumentKind;

import java.util.Map;

/**
 * Result of loading an Aurora native document package.
 */
public record LoadedAuroraPackage(
        AuroraDocumentKind kind,
        String title,
        String documentJson,
        Map<String, String> metadata
) {
    public LoadedAuroraPackage {
        title = title == null || title.isBlank() ? "Untitled" : title;
        documentJson = documentJson == null ? "{}" : documentJson;
        metadata = metadata == null ? Map.of() : Map.copyOf(metadata);
    }
}
