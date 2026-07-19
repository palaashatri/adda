package com.aurora.platform.render;

import java.util.List;

/**
 * Scene-graph-like document render node.
 */
public interface RenderNode {
    /**
     * Computes layout bounds for this node.
     */
    RenderBounds layout(LayoutContext context);

    /**
     * Emits drawing operations for this node.
     */
    void paint(PaintContext context);

    /**
     * Finds render hits for a point in document coordinates.
     */
    List<HitResult> hitTest(RenderPoint point);
}
