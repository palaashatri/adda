// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include "../Platform.h"

namespace platform {

class MacPlatform : public Platform {
public:
    MacPlatform();
    ~MacPlatform();

    bool createWindow(int w, int h, const char* title) override;
    void pumpEvents() override;
    void blit(const uint8_t* buffer, int w, int h) override;

private:
    void* window = nullptr; // NSWindow*
};

} // namespace platform
