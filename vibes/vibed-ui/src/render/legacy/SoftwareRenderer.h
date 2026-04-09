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

    const uint8_t* buffer() const;
    int width() const;
    int height() const;

private:
    void setPixel(int x, int y, uint32_t color);

    int widthPx;
    int heightPx;
    std::vector<uint8_t> pixels;
};

} // namespace render
