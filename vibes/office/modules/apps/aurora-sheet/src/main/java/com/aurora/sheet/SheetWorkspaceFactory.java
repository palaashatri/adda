package com.aurora.sheet;

import com.aurora.platform.ai.AiProvider;
import com.aurora.platform.ai.AiRequest;
import com.aurora.platform.ai.DisabledAiProvider;
import com.aurora.platform.core.AuroraDocumentKind;
import com.aurora.platform.ui.AuroraSheetGrid;
import com.aurora.platform.ui.AuroraWorkspace;
import com.aurora.sheet.formula.FormulaEngine;
import com.aurora.sheet.model.Worksheet;
import javafx.application.Platform;
import javafx.geometry.Insets;
import javafx.scene.control.Alert;
import javafx.scene.control.ButtonType;
import javafx.scene.control.Label;
import javafx.scene.control.ScrollPane;
import javafx.scene.control.TextField;
import javafx.scene.control.TextInputDialog;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;

import java.util.List;
import java.util.Optional;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionException;

/**
 * Creates Aurora Sheet MVP workspaces.
 */
public final class SheetWorkspaceFactory {
    private SheetWorkspaceFactory() {
    }

    /**
     * Creates a new spreadsheet workspace.
     */
    public static AuroraWorkspace create() {
        return create(new DisabledAiProvider("AI is disabled. Open AI Settings to choose a provider."));
    }

    /**
     * Creates a new spreadsheet workspace.
     */
    public static AuroraWorkspace create(AiProvider aiProvider) {
        AuroraSheetGrid grid = new AuroraSheetGrid(80, 16);
        grid.putCell("A1", "Metric");
        grid.putCell("B1", "Q1");
        grid.putCell("C1", "Q2");
        grid.putCell("A2", "Revenue");
        grid.putCell("B2", "12000");
        grid.putCell("C2", "14800");
        grid.putCell("A3", "Expenses");
        grid.putCell("B3", "7400");
        grid.putCell("C3", "8100");
        grid.putCell("A4", "Profit");
        grid.putCell("B4", "=SUM(B2,-B3)");
        grid.putCell("C4", "=SUM(C2,-C3)");

        TextField formulaField = new TextField();
        formulaField.setPromptText("Formula");
        formulaField.setPrefColumnCount(48);
        FormulaEngine formulaEngine = new FormulaEngine();
        formulaField.setOnAction(event -> applyFormulaBar(grid, formulaField.getText()));
        HBox formulaBar = new HBox(8, new Label("fx"), formulaField);
        formulaBar.setPadding(new Insets(0, 0, 10, 0));
        formulaBar.getStyleClass().add("aurora-formula-bar");

        ScrollPane gridScroll = new ScrollPane(grid);
        gridScroll.setPannable(true);
        gridScroll.setFitToHeight(true);
        gridScroll.setFitToWidth(false);

        VBox canvas = new VBox(formulaBar, gridScroll);
        canvas.setPadding(new Insets(18));
        canvas.setMaxWidth(1280);

        return new AuroraWorkspace(
                AuroraDocumentKind.SHEET,
                "Untitled Spreadsheet",
                List.of("Table", "Chart", "Formula", "Sort", "Filter", "Pivot", "AI Analyze"),
                List.of("Sheet 1", "Named Ranges", "Pivot Tables", "Data Sources"),
                List.of("Cell Format", "Number Format", "Conditional Formatting", "Chart Options", "AI Formula Helper"),
                canvas,
                grid::toDocumentJson,
                grid::loadDocumentJson,
                action -> {
                    if ("Formula".equals(action)) {
                        recalculateVisibleFormulas(grid, formulaEngine);
                    } else if ("AI Analyze".equals(action)) {
                        generateFormulaWithAi(grid, aiProvider);
                    }
                }
        );
    }

    private static void applyFormulaBar(AuroraSheetGrid grid, String expression) {
        if (expression == null || expression.isBlank()) {
            return;
        }
        grid.putCell(grid.focusedAddress(), expression);
    }

    private static void recalculateVisibleFormulas(AuroraSheetGrid grid, FormulaEngine engine) {
        Worksheet worksheet = worksheetFromGrid(grid);
        grid.snapshot().forEach((address, rawValue) -> {
            if (rawValue.startsWith("=")) {
                grid.putCell(address, engine.evaluate(rawValue, worksheet).asText());
            }
        });
    }

    private static Worksheet worksheetFromGrid(AuroraSheetGrid grid) {
        Worksheet worksheet = new Worksheet();
        grid.snapshot().forEach(worksheet::setCell);
        return worksheet;
    }

    private static void generateFormulaWithAi(AuroraSheetGrid grid, AiProvider aiProvider) {
        TextInputDialog prompt = new TextInputDialog();
        prompt.setTitle("AI Formula Helper");
        prompt.setHeaderText("Describe the formula for " + grid.focusedAddress());
        prompt.setContentText("Formula goal");
        Optional<String> description = prompt.showAndWait();
        if (description.isEmpty() || description.get().isBlank()) {
            return;
        }
        String focusedAddress = grid.focusedAddress();
        String visibleContext = grid.snapshot().toString();
        Alert consent = new Alert(Alert.AlertType.CONFIRMATION);
        consent.setTitle("AI Formula Helper");
        consent.setHeaderText("Send visible sheet cells to " + aiProvider.id() + "?");
        consent.setContentText("Aurora will send this visible cell context with your formula request:\n\n" + visibleContext);
        Optional<ButtonType> consentResult = consent.showAndWait();
        if (consentResult.isEmpty() || consentResult.get() != ButtonType.OK) {
            return;
        }

        String request = "Return only one spreadsheet formula beginning with '=' for this request: "
                + description.get()
                + "\nVisible cells: "
                + visibleContext;
        CompletableFuture.supplyAsync(() -> aiProvider.complete(new AiRequest(request, null)).text().trim())
                .whenComplete((proposed, failure) -> Platform.runLater(() -> {
                    if (failure != null) {
                        showAiError("AI Formula Helper", failure);
                        return;
                    }
                    reviewFormula(grid, focusedAddress, proposed);
                }));
    }

    private static void reviewFormula(AuroraSheetGrid grid, String focusedAddress, String proposed) {
        Alert review = new Alert(Alert.AlertType.CONFIRMATION);
        review.setTitle("AI Formula Helper");
        review.setHeaderText("Review formula before applying");
        review.setContentText("Cell: " + focusedAddress + "\n\nSuggested formula:\n" + proposed);
        Optional<ButtonType> result = review.showAndWait();
        if (result.isPresent() && result.get() == ButtonType.OK) {
            grid.putCell(focusedAddress, proposed);
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
}
