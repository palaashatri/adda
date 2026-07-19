package com.aurora.platform.ui;

import javafx.geometry.Insets;
import javafx.scene.control.Label;
import javafx.scene.layout.VBox;

import java.util.List;

/**
 * Slide thumbnail stack for Aurora Deck.
 */
public class AuroraSlideNavigator extends VBox {
    public AuroraSlideNavigator(List<String> slides) {
        getStyleClass().add("aurora-slide-navigator");
        setPadding(new Insets(8));
        setSpacing(8);
        for (String slide : slides == null ? List.<String>of() : slides) {
            Label thumbnail = new Label(slide);
            thumbnail.getStyleClass().add("aurora-slide-thumb");
            thumbnail.setMaxWidth(Double.MAX_VALUE);
            getChildren().add(thumbnail);
        }
    }
}
