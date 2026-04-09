// MIT License
// Copyright (c) 2026 Palaash

#include "SoftwareRenderer.h"

#include <algorithm>
#include <cstring>

#include "platform/Platform.h"

namespace render {

SoftwareRenderer::SoftwareRenderer(int width, int height)
    : widthPx(width), heightPx(height), pixels(static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4U, 0U) {}

SoftwareRenderer::~SoftwareRenderer() {}

void SoftwareRenderer::clear(uint32_t color) {
    const int totalPixels = widthPx * heightPx;
    for (int i = 0; i < totalPixels; ++i) {
        std::memcpy(&pixels[static_cast<std::size_t>(i) * 4U], &color, 4U);
    }
}

void SoftwareRenderer::drawRect(int x, int y, int w, int h, uint32_t color) {
    if (w <= 0 || h <= 0) {
        return;
    }

    const int startX = std::max(0, x);
    const int startY = std::max(0, y);
    const int endX = std::min(widthPx, x + w);
    const int endY = std::min(heightPx, y + h);

    for (int yy = startY; yy < endY; ++yy) {
        for (int xx = startX; xx < endX; ++xx) {
            setPixel(xx, yy, color);
        }
    }
}

void SoftwareRenderer::drawRoundedRect(int x, int y, int w, int h, int radius, uint32_t color) {
    (void)radius;
    // TODO: Implement corner clipping for rounded rectangle rendering.
    drawRect(x, y, w, h, color);
}

void SoftwareRenderer::drawText(int x, int y, const std::string& text) {
    (void)x;
    (void)y;
    (void)text;
    // TODO: Implement software text rasterization.
}

void SoftwareRenderer::present() {
    platform::Platform* platformBackend = platform::activePlatform();
    if (platformBackend == nullptr) {
        // TODO: Handle software backbuffer presentation without platform backend.
        return;
    }

    platformBackend->blit(buffer(), widthPx, heightPx);
}

const uint8_t* SoftwareRenderer::buffer() const {
    return pixels.data();
}

int SoftwareRenderer::width() const {
    return widthPx;
}

int SoftwareRenderer::height() const {
    return heightPx;
}

void SoftwareRenderer::setPixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= widthPx || y < 0 || y >= heightPx) {
        return;
    }
    const std::size_t offset = (static_cast<std::size_t>(y) * static_cast<std::size_t>(widthPx) + static_cast<std::size_t>(x)) * 4U;
    std::memcpy(&pixels[offset], &color, 4U);
}

} // namespace render
