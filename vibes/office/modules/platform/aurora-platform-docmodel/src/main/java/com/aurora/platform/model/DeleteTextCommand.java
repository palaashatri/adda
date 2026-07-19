package com.aurora.platform.model;

/**
 * Deletes text in a fixed half-open range.
 */
public final class DeleteTextCommand implements DocumentCommand {
    private final int start;
    private final int end;
    private String deletedText;

    public DeleteTextCommand(int start, int end) {
        this.start = start;
        this.end = end;
    }

    @Override
    public void apply(DocumentModel model) {
        deletedText = model.slice(start, end);
        model.replace(start, end, "");
    }

    @Override
    public void undo(DocumentModel model) {
        model.replace(start, start, deletedText == null ? "" : deletedText);
    }
}
