// MIT License
// Copyright (c) 2026 Palaash

#pragma once

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

private:
    bool running = false;
    std::unique_ptr<platform::Platform> platformBackend;
    int windowWidth = 800;
    int windowHeight = 600;
};

} // namespace core