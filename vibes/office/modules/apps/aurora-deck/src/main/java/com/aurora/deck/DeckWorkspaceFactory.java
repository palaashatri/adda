package com.aurora.deck;

import com.aurora.platform.ai.AiProvider;
import com.aurora.platform.ai.AiRequest;
import com.aurora.platform.ai.DisabledAiProvider;
import com.aurora.platform.core.AuroraDocumentKind;
import com.aurora.platform.core.AuroraJson;
import com.aurora.platform.ui.AuroraWorkspace;
import javafx.application.Platform;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.control.Alert;
import javafx.scene.control.ButtonType;
import javafx.scene.control.TextArea;
import javafx.scene.control.TextField;
import javafx.scene.control.TextInputDialog;
import javafx.scene.layout.VBox;

import java.util.List;
import java.util.Optional;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionException;

/**
 * Creates Aurora Deck MVP workspaces.
 */
public final class DeckWorkspaceFactory {
    private DeckWorkspaceFactory() {
    }

    /**
     * Creates a new presentation workspace.
     */
    public static AuroraWorkspace create() {
        return create(new DisabledAiProvider("AI is disabled. Open AI Settings to choose a provider."));
    }

    /**
     * Creates a new presentation workspace.
     */
    public static AuroraWorkspace create(AiProvider aiProvider) {
        TextField title = new TextField("Aurora Deck");
        title.getStyleClass().add("aurora-slide-title-field");
        title.setAlignment(Pos.CENTER);
        title.setStyle("-fx-font-size: 30px; -fx-font-weight: 700; -fx-background-color: transparent;");

        TextArea body = new TextArea("A clean, native-feeling presentation editor for JavaFX.");
        body.setWrapText(true);
        body.setPrefRowCount(6);
        body.setStyle("-fx-control-inner-background: transparent; -fx-background-color: transparent; -fx-font-size: 17px;");

        VBox slide = new VBox(24, title, body);
        slide.getStyleClass().add("aurora-page");
        slide.setAlignment(Pos.CENTER);
        slide.setPadding(new Insets(72));
        slide.setPrefSize(960, 540);
        slide.setMinSize(720, 405);
        slide.setMaxWidth(960);

        return new AuroraWorkspace(
                AuroraDocumentKind.DECK,
                "Untitled Presentation",
                List.of("Theme", "Insert", "Shape", "Media", "Animate", "Present", "AI Generate"),
                List.of("Slide 1", "Sections", "Presenter Notes"),
                List.of("Slide Layout", "Object Style", "Transitions", "Animations", "Speaker Notes", "AI Slide Assistant"),
                slide,
                () -> "{\"type\":\"deck\",\"title\":\"" + AuroraJson.escape(title.getText()) + "\",\"body\":\""
                        + AuroraJson.escape(body.getText()) + "\"}",
                json -> {
                    title.setText(AuroraJson.readStringField(json, "title").orElse("Untitled Slide"));
                    body.setText(AuroraJson.readStringField(json, "body").orElse(""));
                },
                action -> {
                    if ("AI Generate".equals(action)) {
                        generateSlideWithAi(title, body, aiProvider);
                    }
                }
        );
    }

    private static void generateSlideWithAi(TextField title, TextArea body, AiProvider aiProvider) {
        TextInputDialog prompt = new TextInputDialog();
        prompt.setTitle("AI Slide Assistant");
        prompt.setHeaderText("Generate a slide from an outline");
        prompt.setContentText("Outline");
        Optional<String> outline = prompt.showAndWait();
        if (outline.isEmpty() || outline.get().isBlank()) {
            return;
        }
        CompletableFuture.supplyAsync(() -> aiProvider.complete(new AiRequest(
                        "Create one concise presentation slide. Put the title on the first line and body bullets after it. Outline: "
                                + outline.get(),
                        null
                )).text().trim())
                .whenComplete((response, failure) -> Platform.runLater(() -> {
                    if (failure != null) {
                        showAiError("AI Slide Assistant", failure);
                        return;
                    }
                    reviewGeneratedSlide(title, body, response);
                }));
    }

    private static void reviewGeneratedSlide(TextField title, TextArea body, String response) {
        String proposedTitle = firstLine(response, "Generated Slide");
        String proposedBody = remainingLines(response);
        Alert review = new Alert(Alert.AlertType.CONFIRMATION);
        review.setTitle("AI Slide Assistant");
        review.setHeaderText("Review generated slide before applying");
        review.setContentText("Title:\n" + proposedTitle + "\n\nBody:\n" + proposedBody);
        Optional<ButtonType> result = review.showAndWait();
        if (result.isPresent() && result.get() == ButtonType.OK) {
            title.setText(proposedTitle);
            body.setText(proposedBody);
        }
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

    private static String firstLine(String text, String fallback) {
        if (text == null || text.isBlank()) {
            return fallback;
        }
        return text.lines().findFirst().orElse(fallback).replaceFirst("^[#\\-\\s]+", "").trim();
    }

    private static String remainingLines(String text) {
        if (text == null || text.isBlank()) {
            return "";
        }
        return String.join("\n", text.lines().skip(1).toList()).trim();
    }
}
