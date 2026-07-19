package com.aurora.platform.ui;

import javafx.geometry.Insets;
import javafx.scene.control.Button;
import javafx.scene.control.Separator;
import javafx.scene.layout.HBox;

import java.util.List;
import java.util.function.Consumer;

/**
 * Compact contextual toolbar used by all Aurora editors.
 */
public class AuroraToolbar extends HBox {
    public AuroraToolbar(List<String> actions, Consumer<String> actionHandler) {
        getStyleClass().add("aurora-toolbar");
        setSpacing(8);
        setPadding(new Insets(8, 12, 8, 12));

        List<String> safeActions = actions == null ? List.of() : actions;
        for (int index = 0; index < safeActions.size(); index++) {
            String action = safeActions.get(index);
            if ("|".equals(action)) {
                getChildren().add(new Separator());
                continue;
            }
            Button button = new Button(action);
            button.getStyleClass().add("aurora-toolbar-button");
            button.setFocusTraversable(true);
            button.setOnAction(event -> {
                if (actionHandler != null) {
                    actionHandler.accept(action);
                }
            });
            getChildren().add(button);
        }
    }
}
