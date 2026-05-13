// MIT License
// Copyright (c) 2026 Palaash

#include "Compositor.h"

#include <chrono>
#include <thread>

namespace compositor {

Compositor::Compositor() {}

Compositor::~Compositor() {}

void Compositor::run() {
    running = true;
    while (running) {
        wm.renderAll();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
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
