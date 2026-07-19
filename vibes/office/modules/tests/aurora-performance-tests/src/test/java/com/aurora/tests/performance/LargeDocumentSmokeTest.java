package com.aurora.tests.performance;

import com.aurora.platform.model.CommandHistory;
import com.aurora.platform.model.InsertTextCommand;
import com.aurora.platform.model.TextDocumentModel;
import org.junit.jupiter.api.Test;

import java.time.Duration;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertTimeoutPreemptively;

class LargeDocumentSmokeTest {
    @Test
    void appliesCommandsToLargeTextDocumentQuickly() {
        assertTimeoutPreemptively(Duration.ofSeconds(2), () -> {
            StringBuilder builder = new StringBuilder();
            for (int page = 0; page < 500; page++) {
                builder.append("Page ").append(page + 1).append("\n");
                builder.append("Aurora Office performance smoke text.\n\n");
            }
            TextDocumentModel model = new TextDocumentModel(builder.toString());
            CommandHistory history = new CommandHistory(model);
            int originalLength = model.length();

            history.execute(new InsertTextCommand(model.length(), "Appendix"));

            assertEquals(originalLength + "Appendix".length(), model.length());
        });
    }
}
