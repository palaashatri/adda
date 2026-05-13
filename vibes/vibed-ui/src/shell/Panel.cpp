// MIT License
// Copyright (c) 2026 Palaash

#include "Panel.h"

namespace shell {

Panel::Panel() {}

Panel::~Panel() {}

void Panel::draw(render::Renderer& renderer) {
    const uint32_t bgColor = backgroundColorValue() != 0x00000000U
        ? backgroundColorValue()
        : 0xFF222222U;

    renderer.drawRect(x, y, width, height, bgColor);

    // Left logo / app name
    renderer.drawText(x + 8, y + 12, "vibed-ui Shell");

    // Vertical separator after the logo
    renderer.drawRect(x + 128, y + 6, 1, height - 12, 0xFF555555U);

    // Menu bar items
    renderer.drawText(x + 138, y + 12, "File   Edit   View   Help");

    // Right-side status indicator
    renderer.drawText(x + width - 80, y + 12, "[Desktop]");

    ui::View::draw(renderer);
}

} // namespace shell
