package com.aurora.write;

import com.aurora.platform.core.AuroraDocumentKind;
import com.aurora.platform.core.AuroraJson;
import com.aurora.platform.ai.AiProvider;
import com.aurora.platform.ai.AiRequest;
import com.aurora.platform.ai.DisabledAiProvider;
import com.aurora.platform.ui.AuroraWorkspace;
import javafx.application.Platform;
import javafx.geometry.Insets;
import javafx.scene.control.Alert;
import javafx.scene.control.ButtonType;
import javafx.scene.control.IndexRange;
import javafx.scene.control.TextArea;
import javafx.scene.layout.VBox;

import java.util.List;
import java.util.Optional;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionException;

/**
 * Creates Aurora Write MVP workspaces.
 */
public final class WriteWorkspaceFactory {
    private static final String DEFAULT_BODY = """
            Aurora Write

            Start drafting your document here. This MVP keeps content local, supports native package save/load, and leaves AI changes reviewable before anything is applied.
            """;

    private WriteWorkspaceFactory() {
    }

    /**
     * Creates a new Write document workspace.
     */
    public static AuroraWorkspace create() {
        return create(new DisabledAiProvider("AI is disabled. Open AI Settings to choose a provider."));
    }

    /**
     * Creates a new Write document workspace using the supplied AI provider.
     */
    public static AuroraWorkspace create(AiProvider aiProvider) {
        TextArea editor = new TextArea(DEFAULT_BODY);
        editor.setWrapText(true);
        editor.setPrefRowCount(32);
        editor.setStyle("-fx-control-inner-background: white; -fx-text-fill: #1d1d1f; -fx-font-size: 14px;");

        VBox page = new VBox(editor);
        page.getStyleClass().add("aurora-page");
        page.setPadding(new Insets(54, 62, 54, 62));
        page.setPrefSize(612, 792);
        page.setMinSize(612, 792);
        page.setMaxWidth(612);

        return new AuroraWorkspace(
                AuroraDocumentKind.WRITE,
                "Untitled Document",
                List.of("Insert", "Text", "Table", "Chart", "Shape", "Media", "Comment", "AI Rewrite"),
                List.of("Page 1", "Outline", "Comments", "Search Results"),
                List.of("Text Style", "Paragraph", "Page Layout", "AI Suggestions"),
                page,
                () -> "{\"type\":\"write\",\"body\":\"" + AuroraJson.escape(editor.getText()) + "\"}",
                json -> editor.setText(AuroraJson.readStringField(json, "body").orElse("")),
                action -> {
                    if ("AI Rewrite".equals(action)) {
                        rewriteSelectedText(editor, aiProvider);
                    }
                }
        );
    }

    private static void rewriteSelectedText(TextArea editor, AiProvider aiProvider) {
        String selected = editor.getSelectedText();
        if (selected == null || selected.isBlank()) {
            return;
        }
        AiProvider provider = aiProvider == null
                ? new DisabledAiProvider("AI is disabled. Open AI Settings to choose a provider.")
                : aiProvider;
        IndexRange selection = editor.getSelection();
        Alert consent = new Alert(Alert.AlertType.CONFIRMATION);
        consent.setTitle("AI Rewrite");
        consent.setHeaderText("Send selected text to " + provider.id() + "?");
        consent.setContentText("Only the selected text below will be sent:\n\n" + selected);
        Optional<ButtonType> consentResult = consent.showAndWait();
        if (consentResult.isEmpty() || consentResult.get() != ButtonType.OK) {
            return;
        }

        CompletableFuture.supplyAsync(() -> provider.complete(new AiRequest("Rewrite this clearly: " + selected, null)).text())
                .whenComplete((proposed, failure) -> Platform.runLater(() -> {
                    if (failure != null) {
                        showAiError("AI Rewrite", failure);
                        return;
                    }
                    reviewRewrite(editor, selection, selected, proposed);
                }));
    }

    private static void reviewRewrite(TextArea editor, IndexRange selection, String selected, String proposed) {
        Alert alert = new Alert(Alert.AlertType.CONFIRMATION);
        alert.setTitle("AI Rewrite");
        alert.setHeaderText("Review suggested replacement");
        alert.setContentText("Original:\n" + selected + "\n\nSuggested:\n" + proposed);
        Optional<ButtonType> result = alert.showAndWait();
        if (result.isEmpty() || result.get() != ButtonType.OK) {
            return;
        }
        if (editor.getLength() >= selection.getEnd()
                && selected.equals(editor.getText(selection.getStart(), selection.getEnd()))) {
            editor.replaceText(selection.getStart(), selection.getEnd(), proposed);
            return;
        }

        Alert changed = new Alert(Alert.AlertType.WARNING);
        changed.setTitle("AI Rewrite");
        changed.setHeaderText("Selection changed");
        changed.setContentText("The original selected text changed before the AI suggestion was applied.");
        changed.showAndWait();
    }

    private static void showAiError(String title, Throwable failure) {
        Throwable cause = failure instanceof CompletionException && failure.getCause() != null
                ? failure.getCause()
                : failure;
        Alert alert = new Alert(Alert.AlertType.ERROR);
        alert.setTitle(title);
        alert.setHeaderText("AI request failed");
        alert.setContentText(cause.getMessage());
        alert.showAndWait();
    }
}
