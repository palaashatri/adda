package com.aurora.platform.ui;

import javafx.geometry.Insets;
import javafx.scene.control.Label;
import javafx.scene.layout.Priority;
import javafx.scene.layout.VBox;

import java.util.List;

/**
 * Left-side navigator for pages, sheets, slides, outlines, and comments.
 */
public class AuroraSidebar extends VBox {
    public AuroraSidebar(String title, List<String> items) {
        getStyleClass().add("aurora-sidebar");
        setSpacing(8);
        setPadding(new Insets(14, 12, 14, 12));
        setPrefWidth(188);
        setMinWidth(160);
        setMaxWidth(240);

        Label heading = new Label(title);
        heading.getStyleClass().add("aurora-panel-title");
        getChildren().add(heading);

        for (String item : items == null ? List.<String>of() : items) {
            Label label = new Label(item);
            label.getStyleClass().add("aurora-sidebar-item");
            label.setMaxWidth(Double.MAX_VALUE);
            getChildren().add(label);
        }

        VBox spacer = new VBox();
        VBox.setVgrow(spacer, Priority.ALWAYS);
        getChildren().add(spacer);
    }
}
