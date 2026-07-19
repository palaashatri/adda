package com.aurora.platform.render;

/**
 * Immutable rectangular bounds in document-rendering coordinates.
 */
public record RenderBounds(double x, double y, double width, double height) {
    public RenderBounds {
        if (width < 0 || height < 0) {
            throw new IllegalArgumentException("Bounds dimensions must be non-negative");
        }
    }

    /**
     * Whether the point lies inside this rectangle.
     */
    public boolean contains(RenderPoint point) {
        return point.x() >= x
                && point.y() >= y
                && point.x() <= x + width
                && point.y() <= y + height;
    }
}
