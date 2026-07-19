package com.aurora.platform.ui;

import javafx.scene.Node;
import javafx.scene.layout.StackPane;
import javafx.stage.Popup;
import javafx.stage.Window;

/**
 * Lightweight popover helper for contextual editor UI.
 */
public final class AuroraPopover {
    private AuroraPopover() {
    }

    /**
     * Shows a styled popup near the center of the owner window.
     */
    public static Popup show(Window owner, Node content) {
        Popup popup = new Popup();
        StackPane frame = new StackPane(content);
        frame.getStyleClass().add("aurora-popover");
        popup.getContent().add(frame);
        popup.setAutoHide(true);
        if (owner != null) {
            popup.show(owner, owner.getX() + owner.getWidth() / 2 - 180, owner.getY() + 120);
        }
        return popup;
    }
}
