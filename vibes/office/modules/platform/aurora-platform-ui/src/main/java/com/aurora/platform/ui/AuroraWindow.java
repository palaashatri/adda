package com.aurora.platform.ui;

import javafx.geometry.Insets;
import javafx.scene.Node;
import javafx.scene.layout.BorderPane;

/**
 * Base root pane for Aurora windows.
 */
public class AuroraWindow extends BorderPane {
    public AuroraWindow() {
        getStyleClass().add("aurora-window");
        setPadding(Insets.EMPTY);
    }

    /**
     * Replaces the central content area.
     */
    public void setContent(Node content) {
        setCenter(content);
    }
}
