// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include <X11/Xlib.h>

#include "../Platform.h"

namespace platform {

class LinuxPlatform : public Platform {
public:
    LinuxPlatform();
    ~LinuxPlatform();

    bool createWindow(int w, int h, const char* title) override;
    void pumpEvents() override;
    void blit(const uint8_t* buffer, int w, int h) override;

private:
    Display* display = nullptr;
    Window window = 0;
    GC gc = 0;
    int screen = 0;
    Atom wmDeleteMessage = 0;
};

} // namespace platform
