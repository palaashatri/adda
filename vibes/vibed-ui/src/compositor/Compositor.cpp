// MIT License
// Copyright (c) 2026 Palaash

#include "Compositor.h"

namespace compositor {

Compositor::Compositor() {}

Compositor::~Compositor() {}

void Compositor::run() {
    running = true;
    while (running) {
        wm.renderAll();
        // TODO: Add frame pacing or vsync synchronization.
    }
}

void Compositor::stop() {
    running = false;
}

void Compositor::processEvent(const core::Event& event) {
    if (event.type == core::EventType::MouseDown) {
        wm.focusSurfaceAt(event.x, event.y);
    }
}

WindowManager& Compositor::windowManager() {
    return wm;
}

} // namespace compositor
