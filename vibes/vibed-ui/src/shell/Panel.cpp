// MIT License
// Copyright (c) 2026 Palaash

#include "Panel.h"

namespace shell {

Panel::Panel() {}

Panel::~Panel() {}

void Panel::draw(render::Renderer& renderer) {
    const uint32_t color = backgroundColorValue() != 0x00000000U
        ? backgroundColorValue()
        : 0xFF222222U;
    renderer.drawRect(x, y, width, height, color);
    ui::View::draw(renderer);
}

} // namespace shell
