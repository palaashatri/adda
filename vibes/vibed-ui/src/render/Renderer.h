// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include <cstdint>
#include <string>

namespace render {

class Renderer {
public:
    Renderer();
    virtual ~Renderer();

    virtual void clear(uint32_t color) = 0;
    virtual void drawRect(int x, int y, int w, int h, uint32_t color) = 0;
    virtual void drawRoundedRect(int x, int y, int w, int h, int radius, uint32_t color) = 0;
    virtual void drawText(int x, int y, const std::string& text) = 0;
    virtual void present() = 0;
};

} // namespace render
