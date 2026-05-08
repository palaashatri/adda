// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include "../core/Event.h"
#include "WindowManager.h"

namespace compositor {

class Compositor {
public:
    Compositor();
    ~Compositor();

    void run();
    void stop();
    void processEvent(const core::Event& event);

    WindowManager& windowManager();

private:
    WindowManager wm;
    bool running = false;
};

} // namespace compositor
