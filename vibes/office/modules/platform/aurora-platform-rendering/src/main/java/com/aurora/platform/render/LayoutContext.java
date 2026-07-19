package com.aurora.platform.render;

/**
 * Layout constraints for render nodes.
 */
public record LayoutContext(double availableWidth, double zoom) {
    public LayoutContext {
        if (availableWidth <= 0) {
            throw new IllegalArgumentException("availableWidth must be positive");
        }
        if (zoom <= 0) {
            throw new IllegalArgumentException("zoom must be positive");
        }
    }
}
