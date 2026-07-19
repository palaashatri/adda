package com.aurora.platform.ui;

import javafx.geometry.Insets;
import javafx.scene.control.Label;
import javafx.scene.layout.HBox;

/**
 * Minimal document tab strip for the active editor.
 */
public class AuroraDocumentTab extends HBox {
    public AuroraDocumentTab(String title, String subtitle) {
        getStyleClass().add("aurora-document-tab");
        setPadding(new Insets(8, 12, 6, 12));
        setSpacing(10);

        Label titleLabel = new Label(title);
        titleLabel.getStyleClass().add("aurora-tab-title");
        Label subtitleLabel = new Label(subtitle);
        subtitleLabel.getStyleClass().add("aurora-tab-subtitle");
        getChildren().addAll(titleLabel, subtitleLabel);
    }
}
