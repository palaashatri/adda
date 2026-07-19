package com.aurora.platform.model;

import java.util.ArrayDeque;
import java.util.Deque;
import java.util.Objects;

/**
 * Per-document undo/redo history.
 */
public final class CommandHistory {
    private final DocumentModel model;
    private final Deque<DocumentCommand> undoStack = new ArrayDeque<>();
    private final Deque<DocumentCommand> redoStack = new ArrayDeque<>();

    public CommandHistory(DocumentModel model) {
        this.model = Objects.requireNonNull(model, "model");
    }

    /**
     * Applies a command and adds it to the undo stack.
     */
    public void execute(DocumentCommand command) {
        Objects.requireNonNull(command, "command").apply(model);
        undoStack.push(command);
        redoStack.clear();
    }

    /**
     * Undoes the most recently applied command.
     */
    public boolean undo() {
        if (undoStack.isEmpty()) {
            return false;
        }
        DocumentCommand command = undoStack.pop();
        command.undo(model);
        redoStack.push(command);
        return true;
    }

    /**
     * Redoes the most recently undone command.
     */
    public boolean redo() {
        if (redoStack.isEmpty()) {
            return false;
        }
        DocumentCommand command = redoStack.pop();
        command.apply(model);
        undoStack.push(command);
        return true;
    }

    /**
     * Whether an undo operation is currently available.
     */
    public boolean canUndo() {
        return !undoStack.isEmpty();
    }

    /**
     * Whether a redo operation is currently available.
     */
    public boolean canRedo() {
        return !redoStack.isEmpty();
    }
}
