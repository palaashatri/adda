package com.aurora.platform.model;

/**
 * Inserts text at a fixed offset.
 */
public final class InsertTextCommand implements DocumentCommand {
    private final int offset;
    private final String text;

    public InsertTextCommand(int offset, String text) {
        this.offset = offset;
        this.text = text == null ? "" : text;
    }

    @Override
    public void apply(DocumentModel model) {
        model.replace(offset, offset, text);
    }

    @Override
    public void undo(DocumentModel model) {
        model.replace(offset, offset + text.length(), "");
    }
}
