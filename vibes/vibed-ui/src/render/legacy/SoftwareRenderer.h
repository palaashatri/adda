// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../Renderer.h"

namespace render {

class SoftwareRenderer : public Renderer {
public:
    SoftwareRenderer(int width, int height);
    ~SoftwareRenderer();

    void clear(uint32_t color) override;
    void drawRect(int x, int y, int w, int h, uint32_t color) override;
    void drawRoundedRect(int x, int y, int w, int h, int radius, uint32_t color) override;
    void drawText(int x, int y, const std::string& text) override;
    void present() override;

    // Configurable text colour (default: near-white 0xFFEAEAEA)
    void setTextColor(uint32_t color) { textColor = color; }
    uint32_t getTextColor() const { return textColor; }

    // HDR linear-light pipeline -----------------------------------------
    // Enable to activate an internal float32 RGBA buffer (linear light).
    // SDR callers still use the uint32 API; their colours are expanded to
    // linear before being written to the HDR buffer and tone-mapped back
    // to 8-bit sRGB on present().  HDR callers can also write directly via
    // the float API below.
    void setHdrMode(bool enabled);
    bool hdrMode() const { return hdrModeEnabled; }

    // Direct linear-light drawing (values > 1.0 allowed for HDR headroom)
    void clearF(float r, float g, float b);
    void drawRectF(int x, int y, int w, int h, float r, float g, float b, float a = 1.0f);

    // Colour-space helpers (public so callers can do their own transforms)
    static float srgbToLinear(float c);
    static float linearToSrgb(float c);
    static float reinhardTonemap(float L);  // maps [0,inf) → [0,1)

    const uint8_t* buffer() const;
    int width() const;
    int height() const;

private:
    void setPixel(int x, int y, uint32_t color);
    void setPixelF(int x, int y, float r, float g, float b, float a = 1.0f);
    void tonemapHdrBuffer();  // writes HDR float buffer → pixels (8-bit sRGB)

    int widthPx;
    int heightPx;
    std::vector<uint8_t> pixels;

    uint32_t textColor = 0xFFEAEAEAU;

    bool hdrModeEnabled = false;
    std::vector<float> hdrBuffer;  // RGBA float32, linear light, per pixel
};

} // namespace render
