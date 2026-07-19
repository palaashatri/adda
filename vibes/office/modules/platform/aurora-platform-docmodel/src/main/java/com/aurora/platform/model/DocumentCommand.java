package com.aurora.platform.model;

/**
 * Undoable mutation applied to a document model.
 */
public interface DocumentCommand {
    /**
     * Applies the mutation to the supplied model.
     */
    void apply(DocumentModel model);

    /**
     * Reverts a previously applied mutation from the supplied model.
     */
    void undo(DocumentModel model);

    /**
     * Human-readable command name for menus, logs, and tests.
     */
    default String name() {
        return getClass().getSimpleName();
    }
}
