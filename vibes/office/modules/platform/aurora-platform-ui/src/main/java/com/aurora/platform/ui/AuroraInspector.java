package com.aurora.platform.ui;

import javafx.geometry.Insets;
import javafx.scene.control.Label;
import javafx.scene.control.Separator;
import javafx.scene.layout.VBox;

import java.util.List;

/**
 * Right-side inspector for style, layout, and AI panels.
 */
public class AuroraInspector extends VBox {
    public AuroraInspector(List<String> sections) {
        getStyleClass().add("aurora-inspector");
        setSpacing(10);
        setPadding(new Insets(14, 14, 14, 14));
        setPrefWidth(248);
        setMinWidth(220);
        setMaxWidth(300);

        Label heading = new Label("Inspector");
        heading.getStyleClass().add("aurora-panel-title");
        getChildren().add(heading);

        for (String section : sections == null ? List.<String>of() : sections) {
            Label label = new Label(section);
            label.getStyleClass().add("aurora-inspector-section");
            label.setMaxWidth(Double.MAX_VALUE);
            getChildren().add(label);
            getChildren().add(new Separator());
        }
    }
}
