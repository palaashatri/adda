// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include <cstdint>
#include <memory>

namespace platform {

class Platform {
public:
    Platform();
    virtual ~Platform();

    virtual bool createWindow(int w, int h, const char* title) = 0;
    virtual void pumpEvents() = 0;
    virtual void blit(const uint8_t* buffer, int w, int h) = 0;
};

std::unique_ptr<Platform> createPlatform();
Platform* activePlatform();
void setActivePlatform(Platform* platformInstance);

} // namespace platform
