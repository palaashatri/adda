// MIT License
// Copyright (c) 2026 Palaash

#include "Launcher.h"

namespace shell {

Launcher::Launcher() {}

Launcher::~Launcher() {}

void Launcher::draw(render::Renderer& renderer) {
    const uint32_t color = backgroundColorValue() != 0x00000000U
        ? backgroundColorValue()
        : 0xFF444444U;
    renderer.drawRect(x, y, width, height, color);
    ui::View::draw(renderer);
}

} // namespace shell
