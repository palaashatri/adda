// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include <cstdint>

#include "render/legacy/SoftwareRenderer.h"

namespace compositor {

class Surface {
public:
    Surface(int w, int h);
    ~Surface();

    int width() const;
    int height() const;

    void setPosition(int x, int y);
    int x() const;
    int y() const;

    void setVisible(bool value);
    bool isVisible() const;
    bool containsPoint(int px, int py) const;

    // Pixel buffer — used by the compositor to composite this surface
    // into the final output frame.
    render::SoftwareRenderer& renderer();
    const uint8_t* buffer() const;
    void clear(uint32_t color);

private:
    int w;
    int h;
    int posX = 0;
    int posY = 0;
    bool visible = true;
    render::SoftwareRenderer surfaceRenderer;
};

} // namespace compositor
