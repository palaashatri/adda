package com.aurora.platform.model;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;

class CommandHistoryTest {
    @Test
    void executeUndoRedoInsertCommand() {
        TextDocumentModel model = new TextDocumentModel("Aurora");
        CommandHistory history = new CommandHistory(model);

        history.execute(new InsertTextCommand(6, " Office"));

        assertEquals("Aurora Office", model.text());
        assertTrue(history.canUndo());
        assertTrue(history.undo());
        assertEquals("Aurora", model.text());
        assertTrue(history.canRedo());
        assertTrue(history.redo());
        assertEquals("Aurora Office", model.text());
    }

    @Test
    void replaceAndDeleteRestoreOriginalText() {
        TextDocumentModel model = new TextDocumentModel("draft report");
        CommandHistory history = new CommandHistory(model);

        history.execute(new ReplaceTextCommand(0, 5, "final"));
        history.execute(new DeleteTextCommand(5, 12));

        assertEquals("final", model.text());
        assertTrue(history.undo());
        assertEquals("final report", model.text());
        assertTrue(history.undo());
        assertEquals("draft report", model.text());
        assertFalse(history.undo());
    }
}
