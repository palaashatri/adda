// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include <cstdint>
#include <string>

#include "../Renderer.h"
#include "../legacy/SoftwareRenderer.h"

namespace render {

// ModernRenderer currently delegates to SoftwareRenderer.
// TODO: Replace delegation with platform-specific GPU paths when available.
class ModernRenderer : public Renderer {
public:
    ModernRenderer(int width, int height);
    ~ModernRenderer();

    void clear(uint32_t color) override;
    void drawRect(int x, int y, int w, int h, uint32_t color) override;
    void drawRoundedRect(int x, int y, int w, int h, int radius, uint32_t color) override;
    void drawText(int x, int y, const std::string& text) override;
    void present() override;

private:
    SoftwareRenderer delegate;
};

} // namespace render
