// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include <windows.h>

#include "../Platform.h"

namespace platform {

class WinPlatform : public Platform {
public:
    WinPlatform();
    ~WinPlatform();

    bool createWindow(int w, int h, const char* title) override;
    void pumpEvents() override;
    void blit(const uint8_t* buffer, int w, int h) override;

    bool isHdrCapable() const override { return hdrCapable; }
    int  displayRefreshRate() const override { return refreshRateHz; }
    void waitForVSync() override;
    void setHdrEnabled(bool enabled) override { hdrEnabled = enabled; }

private:
    void detectDisplayCapabilities();

    HWND hwnd        = nullptr;
    HDC  hdc         = nullptr;
    bool hdrCapable  = false;
    bool hdrEnabled  = false;
    int  refreshRateHz = 60;
};

} // namespace platform
