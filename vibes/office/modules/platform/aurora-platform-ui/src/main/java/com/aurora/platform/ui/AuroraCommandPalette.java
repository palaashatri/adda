package com.aurora.platform.ui;

import javafx.geometry.Insets;
import javafx.scene.control.ListView;
import javafx.scene.control.TextField;
import javafx.scene.input.KeyCode;
import javafx.scene.layout.VBox;
import javafx.stage.Popup;
import javafx.stage.Window;

import java.util.List;
import java.util.function.Consumer;

/**
 * Keyboard-first command palette used by the launcher and editors.
 */
public final class AuroraCommandPalette {
    private AuroraCommandPalette() {
    }

    /**
     * Opens a command palette for the supplied commands.
     */
    public static Popup show(Window owner, List<String> commands, Consumer<String> handler) {
        TextField search = new TextField();
        search.setPromptText("Command");

        ListView<String> listView = new ListView<>();
        listView.getItems().setAll(commands == null ? List.of() : commands);
        listView.setPrefHeight(180);

        VBox content = new VBox(8, search, listView);
        content.setPadding(new Insets(12));
        content.setPrefWidth(360);

        Popup popup = AuroraPopover.show(owner, content);
        Runnable choose = () -> {
            String selected = listView.getSelectionModel().getSelectedItem();
            if (selected != null && handler != null) {
                handler.accept(selected);
                popup.hide();
            }
        };

        search.setOnKeyPressed(event -> {
            if (event.getCode() == KeyCode.ENTER) {
                choose.run();
            }
        });
        listView.setOnMouseClicked(event -> {
            if (event.getClickCount() == 2) {
                choose.run();
            }
        });
        listView.getSelectionModel().selectFirst();
        search.requestFocus();
        return popup;
    }
}
