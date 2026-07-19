package com.aurora.platform.ui;

import javafx.scene.Scene;

import java.net.URL;
import java.util.Locale;

/**
 * Applies bundled Aurora JavaFX themes to scenes.
 */
public final class AuroraTheme {
    private AuroraTheme() {
    }

    /**
     * Applies a named theme such as {@code macos-light} or {@code macos-dark}.
     */
    public static void apply(Scene scene, String themeName) {
        if (scene == null) {
            return;
        }
        scene.getStylesheets().removeIf(stylesheet -> stylesheet.contains("/themes/"));
        URL resource = AuroraTheme.class.getResource("/com/aurora/platform/ui/themes/" + normalize(themeName) + ".css");
        if (resource != null) {
            scene.getStylesheets().add(resource.toExternalForm());
        }
    }

    private static String normalize(String themeName) {
        if (themeName == null || themeName.isBlank()) {
            return "macos-light";
        }
        return themeName.toLowerCase(Locale.ROOT);
    }
}
