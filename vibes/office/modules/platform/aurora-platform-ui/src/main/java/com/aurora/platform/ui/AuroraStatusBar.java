package com.aurora.platform.ui;

import javafx.geometry.Insets;
import javafx.scene.control.Label;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Priority;
import javafx.scene.layout.Region;

/**
 * Bottom status bar with document status, zoom, and AI provider state.
 */
public class AuroraStatusBar extends HBox {
    private final Label statusLabel = new Label();
    private final Label zoomLabel = new Label("100%");
    private final Label aiLabel = new Label("AI: Local mock");

    public AuroraStatusBar(String initialStatus) {
        getStyleClass().add("aurora-status-bar");
        setPadding(new Insets(6, 12, 6, 12));
        setSpacing(12);

        statusLabel.setText(initialStatus == null ? "Ready" : initialStatus);
        Region spacer = new Region();
        HBox.setHgrow(spacer, Priority.ALWAYS);
        getChildren().addAll(statusLabel, spacer, zoomLabel, aiLabel);
    }

    /**
     * Updates the left status text.
     */
    public void setStatus(String status) {
        statusLabel.setText(status == null || status.isBlank() ? "Ready" : status);
    }
}
