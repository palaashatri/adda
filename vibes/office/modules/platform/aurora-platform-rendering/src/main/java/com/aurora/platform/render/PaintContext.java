package com.aurora.platform.render;

import java.util.ArrayList;
import java.util.List;

/**
 * MVP paint context that records draw operations for tests and future adapters.
 */
public final class PaintContext {
    private final List<String> operations = new ArrayList<>();

    /**
     * Records a text draw operation.
     */
    public void drawText(String text, RenderBounds bounds) {
        operations.add("text:" + text + "@" + bounds);
    }

    /**
     * Records a rectangle draw operation.
     */
    public void drawRect(RenderBounds bounds) {
        operations.add("rect@" + bounds);
    }

    /**
     * Draw operations emitted during painting.
     */
    public List<String> operations() {
        return List.copyOf(operations);
    }
}
