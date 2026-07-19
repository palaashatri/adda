package com.aurora.platform.ui;

import javafx.geometry.Pos;
import javafx.scene.Node;
import javafx.scene.control.ScrollPane;
import javafx.scene.layout.StackPane;

/**
 * Scrollable document canvas that centers editor content.
 */
public class AuroraCanvas extends StackPane {
    public AuroraCanvas(Node content) {
        getStyleClass().add("aurora-canvas");
        setAlignment(Pos.CENTER);

        StackPane centeringPane = new StackPane(content);
        centeringPane.setAlignment(Pos.TOP_CENTER);
        centeringPane.getStyleClass().add("aurora-canvas-content");

        ScrollPane scrollPane = new ScrollPane(centeringPane);
        scrollPane.getStyleClass().add("aurora-canvas-scroll");
        scrollPane.setFitToWidth(true);
        scrollPane.setFitToHeight(true);
        scrollPane.setPannable(true);
        getChildren().add(scrollPane);
    }
}
