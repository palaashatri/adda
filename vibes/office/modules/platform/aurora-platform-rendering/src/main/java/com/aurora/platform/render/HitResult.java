package com.aurora.platform.render;

/**
 * Result of hit-testing a render node.
 */
public record HitResult(RenderNode node, int textOffset, RenderBounds bounds) {
}
