package com.aurora.platform.model;

/**
 * Minimal text document contract used by command-based editing.
 */
public interface DocumentModel {
    /**
     * Current document text.
     */
    String text();

    /**
     * Replaces text in the half-open range {@code [start, end)}.
     */
    void replace(int start, int end, String replacement);

    /**
     * Current document length in UTF-16 code units.
     */
    default int length() {
        return text().length();
    }

    /**
     * Returns text in the half-open range {@code [start, end)}.
     */
    default String slice(int start, int end) {
        return text().substring(start, end);
    }
}
