package com.aurora.platform.model;

import java.util.Objects;

/**
 * Mutable text model for MVP editors and command tests.
 */
public final class TextDocumentModel implements DocumentModel {
    private final StringBuilder text;

    public TextDocumentModel() {
        this("");
    }

    public TextDocumentModel(String initialText) {
        this.text = new StringBuilder(initialText == null ? "" : initialText);
    }

    @Override
    public String text() {
        return text.toString();
    }

    @Override
    public void replace(int start, int end, String replacement) {
        validateRange(start, end);
        text.replace(start, end, Objects.requireNonNullElse(replacement, ""));
    }

    private void validateRange(int start, int end) {
        if (start < 0 || end < start || end > text.length()) {
            throw new IndexOutOfBoundsException("Invalid document range [" + start + ", " + end + ") for length " + text.length());
        }
    }
}
