package com.aurora.platform.model;

/**
 * Replaces text in a fixed half-open range.
 */
public final class ReplaceTextCommand implements DocumentCommand {
    private final int start;
    private final int end;
    private final String replacement;
    private String previousText;

    public ReplaceTextCommand(int start, int end, String replacement) {
        this.start = start;
        this.end = end;
        this.replacement = replacement == null ? "" : replacement;
    }

    @Override
    public void apply(DocumentModel model) {
        previousText = model.slice(start, end);
        model.replace(start, end, replacement);
    }

    @Override
    public void undo(DocumentModel model) {
        model.replace(start, start + replacement.length(), previousText == null ? "" : previousText);
    }
}
