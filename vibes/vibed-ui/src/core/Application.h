// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include <functional>
#include <memory>

namespace platform {
class Platform;
}

namespace core {

class Application {
public:
    Application();
    ~Application();

    bool initialize();
    void run();
    void shutdown();
    void setWindowSize(int w, int h);

    // Optional callback invoked once per frame inside run().
    // Provide rendering and per-frame logic here when using Application::run()
    // directly instead of writing a custom loop.
    void setFrameCallback(std::function<void()> cb);

private:
    bool running = false;
    std::unique_ptr<platform::Platform> platformBackend;
    int windowWidth  = 800;
    int windowHeight = 600;
    std::function<void()> frameCallback;
};

} // namespace core
