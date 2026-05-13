// MIT License
// Copyright (c) 2026 Palaash

#include "Launcher.h"

namespace shell {

// Simple list of launcher entries shown in the app menu.
static const char* const kAppEntries[] = {
    "Terminal",
    "Text Editor",
    "File Manager",
    "System Monitor",
    "Settings",
    nullptr
};

Launcher::Launcher() {}

Launcher::~Launcher() {}

void Launcher::draw(render::Renderer& renderer) {
    const uint32_t bgColor = backgroundColorValue() != 0x00000000U
        ? backgroundColorValue()
        : 0xFF444444U;

    // Background
    renderer.drawRect(x, y, width, height, bgColor);

    // Header bar
    renderer.drawRect(x, y, width, 28, 0xFF333333U);
    renderer.drawText(x + 6, y + 8, "Applications");
    renderer.drawRect(x, y + 28, width, 1, 0xFF555555U);

    // App entries
    int entryY = y + 36;
    for (int i = 0; kAppEntries[i] != nullptr; ++i) {
        renderer.drawText(x + 10, entryY, kAppEntries[i]);
        entryY += 20;
        renderer.drawRect(x + 4, entryY, width - 8, 1, 0xFF4A4A4AU);
        entryY += 8;
    }

    // Footer separator
    renderer.drawRect(x, y + height - 28, width, 1, 0xFF555555U);
    renderer.drawText(x + 6, y + height - 20, "Click to focus panel/launcher");

    ui::View::draw(renderer);
}

} // namespace shell
