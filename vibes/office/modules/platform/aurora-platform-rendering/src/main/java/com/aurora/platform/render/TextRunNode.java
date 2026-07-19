package com.aurora.platform.render;

import java.util.List;

/**
 * Simple text run node used by early Write rendering tests.
 */
public final class TextRunNode implements RenderNode {
    private static final double BASE_CHAR_WIDTH = 7.0;
    private static final double BASE_LINE_HEIGHT = 18.0;

    private final String text;
    private RenderBounds bounds = new RenderBounds(0, 0, 0, 0);

    public TextRunNode(String text) {
        this.text = text == null ? "" : text;
    }

    @Override
    public RenderBounds layout(LayoutContext context) {
        double charWidth = BASE_CHAR_WIDTH * context.zoom();
        double lineHeight = BASE_LINE_HEIGHT * context.zoom();
        double width = Math.min(context.availableWidth(), Math.max(charWidth, text.length() * charWidth));
        int lines = Math.max(1, (int) Math.ceil((text.length() * charWidth) / context.availableWidth()));
        bounds = new RenderBounds(0, 0, width, lines * lineHeight);
        return bounds;
    }

    @Override
    public void paint(PaintContext context) {
        context.drawText(text, bounds);
    }

    @Override
    public List<HitResult> hitTest(RenderPoint point) {
        if (!bounds.contains(point)) {
            return List.of();
        }
        double approximateCharWidth = text.isEmpty() ? BASE_CHAR_WIDTH : Math.max(1, bounds.width() / Math.max(1, text.length()));
        int offset = Math.max(0, Math.min(text.length(), (int) Math.floor(point.x() / approximateCharWidth)));
        return List.of(new HitResult(this, offset, bounds));
    }
}
