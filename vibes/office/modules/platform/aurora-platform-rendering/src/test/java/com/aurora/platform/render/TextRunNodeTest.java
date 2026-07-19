package com.aurora.platform.render;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertFalse;

class TextRunNodeTest {
    @Test
    void layoutPaintAndHitTestAreStable() {
        TextRunNode node = new TextRunNode("Aurora");
        RenderBounds bounds = node.layout(new LayoutContext(300, 1));
        PaintContext paint = new PaintContext();

        node.paint(paint);

        assertEquals(new RenderBounds(0, 0, 42, 18), bounds);
        assertEquals(1, paint.operations().size());
        assertFalse(node.hitTest(new RenderPoint(10, 10)).isEmpty());
    }
}
