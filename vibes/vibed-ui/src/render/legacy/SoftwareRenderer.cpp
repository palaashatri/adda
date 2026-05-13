// MIT License
// Copyright (c) 2026 Palaash

#include "SoftwareRenderer.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>

#include "platform/Platform.h"

namespace {

const uint8_t kGlyphUnknown[7] = {
    0x1F,
    0x11,
    0x01,
    0x02,
    0x04,
    0x00,
    0x04,
};

const uint8_t kGlyphSpace[7] = {0, 0, 0, 0, 0, 0, 0};

const uint8_t* glyphForChar(char ch) {
    switch (ch) {
        // --- Uppercase ---
        case 'A': { static const uint8_t g[7] = {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11}; return g; }
        case 'B': { static const uint8_t g[7] = {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E}; return g; }
        case 'C': { static const uint8_t g[7] = {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E}; return g; }
        case 'D': { static const uint8_t g[7] = {0x1C,0x12,0x11,0x11,0x11,0x12,0x1C}; return g; }
        case 'E': { static const uint8_t g[7] = {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F}; return g; }
        case 'F': { static const uint8_t g[7] = {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10}; return g; }
        case 'G': { static const uint8_t g[7] = {0x0E,0x11,0x10,0x13,0x11,0x11,0x0E}; return g; }
        case 'H': { static const uint8_t g[7] = {0x11,0x11,0x11,0x1F,0x11,0x11,0x11}; return g; }
        case 'I': { static const uint8_t g[7] = {0x1F,0x04,0x04,0x04,0x04,0x04,0x1F}; return g; }
        case 'J': { static const uint8_t g[7] = {0x01,0x01,0x01,0x01,0x11,0x11,0x0E}; return g; }
        case 'K': { static const uint8_t g[7] = {0x11,0x12,0x14,0x18,0x14,0x12,0x11}; return g; }
        case 'L': { static const uint8_t g[7] = {0x10,0x10,0x10,0x10,0x10,0x10,0x1F}; return g; }
        case 'M': { static const uint8_t g[7] = {0x11,0x1B,0x15,0x15,0x11,0x11,0x11}; return g; }
        case 'N': { static const uint8_t g[7] = {0x11,0x19,0x15,0x13,0x11,0x11,0x11}; return g; }
        case 'O': { static const uint8_t g[7] = {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E}; return g; }
        case 'P': { static const uint8_t g[7] = {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10}; return g; }
        case 'Q': { static const uint8_t g[7] = {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D}; return g; }
        case 'R': { static const uint8_t g[7] = {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11}; return g; }
        case 'S': { static const uint8_t g[7] = {0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E}; return g; }
        case 'T': { static const uint8_t g[7] = {0x1F,0x04,0x04,0x04,0x04,0x04,0x04}; return g; }
        case 'U': { static const uint8_t g[7] = {0x11,0x11,0x11,0x11,0x11,0x11,0x0E}; return g; }
        case 'V': { static const uint8_t g[7] = {0x11,0x11,0x11,0x11,0x11,0x0A,0x04}; return g; }
        case 'W': { static const uint8_t g[7] = {0x11,0x11,0x11,0x15,0x15,0x15,0x0A}; return g; }
        case 'X': { static const uint8_t g[7] = {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11}; return g; }
        case 'Y': { static const uint8_t g[7] = {0x11,0x11,0x0A,0x04,0x04,0x04,0x04}; return g; }
        case 'Z': { static const uint8_t g[7] = {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F}; return g; }
        // --- Lowercase (distinct from uppercase) ---
        case 'a': { static const uint8_t g[7] = {0x00,0x00,0x0E,0x01,0x0F,0x11,0x0F}; return g; }
        case 'b': { static const uint8_t g[7] = {0x10,0x10,0x1E,0x11,0x11,0x11,0x1E}; return g; }
        case 'c': { static const uint8_t g[7] = {0x00,0x00,0x0E,0x10,0x10,0x10,0x0E}; return g; }
        case 'd': { static const uint8_t g[7] = {0x01,0x01,0x0F,0x11,0x11,0x11,0x0F}; return g; }
        case 'e': { static const uint8_t g[7] = {0x00,0x00,0x0E,0x11,0x1F,0x10,0x0E}; return g; }
        case 'f': { static const uint8_t g[7] = {0x03,0x04,0x0E,0x04,0x04,0x04,0x00}; return g; }
        case 'g': { static const uint8_t g[7] = {0x00,0x0F,0x11,0x11,0x0F,0x01,0x0E}; return g; }
        case 'h': { static const uint8_t g[7] = {0x10,0x10,0x16,0x19,0x11,0x11,0x11}; return g; }
        case 'i': { static const uint8_t g[7] = {0x04,0x00,0x04,0x04,0x04,0x04,0x0E}; return g; }
        case 'j': { static const uint8_t g[7] = {0x02,0x00,0x06,0x02,0x02,0x12,0x0C}; return g; }
        case 'k': { static const uint8_t g[7] = {0x10,0x10,0x12,0x14,0x18,0x14,0x12}; return g; }
        case 'l': { static const uint8_t g[7] = {0x0C,0x04,0x04,0x04,0x04,0x04,0x0E}; return g; }
        case 'm': { static const uint8_t g[7] = {0x00,0x00,0x1A,0x15,0x15,0x11,0x11}; return g; }
        case 'n': { static const uint8_t g[7] = {0x00,0x00,0x16,0x19,0x11,0x11,0x11}; return g; }
        case 'o': { static const uint8_t g[7] = {0x00,0x00,0x0E,0x11,0x11,0x11,0x0E}; return g; }
        case 'p': { static const uint8_t g[7] = {0x00,0x1E,0x11,0x11,0x11,0x1E,0x10}; return g; }
        case 'q': { static const uint8_t g[7] = {0x00,0x0F,0x11,0x11,0x11,0x0F,0x01}; return g; }
        case 'r': { static const uint8_t g[7] = {0x00,0x00,0x16,0x18,0x10,0x10,0x10}; return g; }
        case 's': { static const uint8_t g[7] = {0x00,0x00,0x0E,0x10,0x0E,0x01,0x0E}; return g; }
        case 't': { static const uint8_t g[7] = {0x04,0x04,0x1F,0x04,0x04,0x04,0x00}; return g; }
        case 'u': { static const uint8_t g[7] = {0x00,0x00,0x11,0x11,0x11,0x11,0x0E}; return g; }
        case 'v': { static const uint8_t g[7] = {0x00,0x00,0x11,0x11,0x11,0x0A,0x04}; return g; }
        case 'w': { static const uint8_t g[7] = {0x00,0x00,0x11,0x11,0x15,0x15,0x0A}; return g; }
        case 'x': { static const uint8_t g[7] = {0x00,0x00,0x11,0x0A,0x04,0x0A,0x11}; return g; }
        case 'y': { static const uint8_t g[7] = {0x00,0x00,0x11,0x11,0x0F,0x01,0x0E}; return g; }
        case 'z': { static const uint8_t g[7] = {0x00,0x00,0x1F,0x02,0x04,0x08,0x1F}; return g; }
        // --- Digits ---
        case '0': { static const uint8_t g[7] = {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E}; return g; }
        case '1': { static const uint8_t g[7] = {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E}; return g; }
        case '2': { static const uint8_t g[7] = {0x0E,0x11,0x01,0x02,0x04,0x08,0x1F}; return g; }
        case '3': { static const uint8_t g[7] = {0x1E,0x01,0x01,0x0E,0x01,0x01,0x1E}; return g; }
        case '4': { static const uint8_t g[7] = {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02}; return g; }
        case '5': { static const uint8_t g[7] = {0x1F,0x10,0x10,0x1E,0x01,0x01,0x1E}; return g; }
        case '6': { static const uint8_t g[7] = {0x0E,0x10,0x10,0x1E,0x11,0x11,0x0E}; return g; }
        case '7': { static const uint8_t g[7] = {0x1F,0x01,0x02,0x04,0x08,0x08,0x08}; return g; }
        case '8': { static const uint8_t g[7] = {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E}; return g; }
        case '9': { static const uint8_t g[7] = {0x0E,0x11,0x11,0x0F,0x01,0x01,0x0E}; return g; }
        // --- Punctuation and symbols ---
        case ' ': return kGlyphSpace;
        case '.': { static const uint8_t g[7] = {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C}; return g; }
        case ',': { static const uint8_t g[7] = {0x00,0x00,0x00,0x00,0x00,0x0C,0x08}; return g; }
        case ':': { static const uint8_t g[7] = {0x00,0x0C,0x0C,0x00,0x0C,0x0C,0x00}; return g; }
        case ';': { static const uint8_t g[7] = {0x00,0x0C,0x0C,0x00,0x0C,0x08,0x00}; return g; }
        case '|': { static const uint8_t g[7] = {0x04,0x04,0x04,0x04,0x04,0x04,0x04}; return g; }
        case '-': { static const uint8_t g[7] = {0x00,0x00,0x00,0x1F,0x00,0x00,0x00}; return g; }
        case '_': { static const uint8_t g[7] = {0x00,0x00,0x00,0x00,0x00,0x00,0x1F}; return g; }
        case '/': { static const uint8_t g[7] = {0x01,0x02,0x04,0x08,0x10,0x00,0x00}; return g; }
        case '\\':{ static const uint8_t g[7] = {0x10,0x08,0x04,0x02,0x01,0x00,0x00}; return g; }
        case '(': { static const uint8_t g[7] = {0x02,0x04,0x08,0x08,0x08,0x04,0x02}; return g; }
        case ')': { static const uint8_t g[7] = {0x08,0x04,0x02,0x02,0x02,0x04,0x08}; return g; }
        case '[': { static const uint8_t g[7] = {0x0E,0x08,0x08,0x08,0x08,0x08,0x0E}; return g; }
        case ']': { static const uint8_t g[7] = {0x0E,0x02,0x02,0x02,0x02,0x02,0x0E}; return g; }
        case '!': { static const uint8_t g[7] = {0x04,0x04,0x04,0x04,0x04,0x00,0x04}; return g; }
        case '?': { static const uint8_t g[7] = {0x0E,0x11,0x01,0x02,0x04,0x00,0x04}; return g; }
        case '\'':{ static const uint8_t g[7] = {0x04,0x04,0x08,0x00,0x00,0x00,0x00}; return g; }
        case '"': { static const uint8_t g[7] = {0x0A,0x0A,0x0A,0x00,0x00,0x00,0x00}; return g; }
        case '+': { static const uint8_t g[7] = {0x00,0x04,0x04,0x1F,0x04,0x04,0x00}; return g; }
        case '=': { static const uint8_t g[7] = {0x00,0x00,0x1F,0x00,0x1F,0x00,0x00}; return g; }
        case '<': { static const uint8_t g[7] = {0x00,0x02,0x04,0x08,0x04,0x02,0x00}; return g; }
        case '>': { static const uint8_t g[7] = {0x00,0x08,0x04,0x02,0x04,0x08,0x00}; return g; }
        case '@': { static const uint8_t g[7] = {0x0E,0x11,0x17,0x15,0x17,0x10,0x0E}; return g; }
        case '#': { static const uint8_t g[7] = {0x0A,0x0A,0x1F,0x0A,0x1F,0x0A,0x0A}; return g; }
        case '*': { static const uint8_t g[7] = {0x00,0x04,0x15,0x0E,0x15,0x04,0x00}; return g; }
        case '%': { static const uint8_t g[7] = {0x18,0x19,0x02,0x04,0x08,0x13,0x03}; return g; }
        case '&': { static const uint8_t g[7] = {0x0C,0x12,0x12,0x0C,0x15,0x12,0x0D}; return g; }
        case '^': { static const uint8_t g[7] = {0x04,0x0A,0x11,0x00,0x00,0x00,0x00}; return g; }
        case '~': { static const uint8_t g[7] = {0x00,0x00,0x08,0x15,0x02,0x00,0x00}; return g; }
        case '`': { static const uint8_t g[7] = {0x08,0x04,0x02,0x00,0x00,0x00,0x00}; return g; }
        case '$': { static const uint8_t g[7] = {0x04,0x0F,0x14,0x0E,0x05,0x1E,0x04}; return g; }
        default:
            return kGlyphUnknown;
    }
}

} // namespace

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
    if (w <= 0 || h <= 0) {
        return;
    }

    const int r = radius <= 0 ? 0 : std::min(radius, std::min(w / 2, h / 2));
    if (r <= 0) {
        drawRect(x, y, w, h, color);
        return;
    }

    // Central horizontal band (full width, full height)
    drawRect(x + r, y, w - 2 * r, h, color);
    // Left and right vertical side bands (excluding corner quadrants)
    drawRect(x, y + r, r, h - 2 * r, color);
    drawRect(x + w - r, y + r, r, h - 2 * r, color);

    // Fill each corner quadrant using circle arc approximation.
    // For TL corner the circle center is at (x+r, y+r); pixel (x+dx, y+dy)
    // is inside when (dx-r)^2 + (dy-r)^2 <= r^2.
    for (int dy = 0; dy < r; ++dy) {
        for (int dx = 0; dx < r; ++dx) {
            const int cdx = dx - r;
            const int cdy = dy - r;
            if (cdx * cdx + cdy * cdy <= r * r) {
                setPixel(x + dx, y + dy, color);
                setPixel(x + w - 1 - dx, y + dy, color);
                setPixel(x + dx, y + h - 1 - dy, color);
                setPixel(x + w - 1 - dx, y + h - 1 - dy, color);
            }
        }
    }
}

void SoftwareRenderer::drawText(int x, int y, const std::string& text) {
    const uint32_t color = textColor;
    const int scale = 2;
    const int glyphWidth = 5 * scale;
    const int advance = 6 * scale;
    const int lineAdvance = 9 * scale;

    int cursorX = x;
    int cursorY = y;

    for (const char ch : text) {
        if (ch == '\n') {
            cursorX = x;
            cursorY += lineAdvance;
            continue;
        }

        const uint8_t* glyph = glyphForChar(ch);
        for (int row = 0; row < 7; ++row) {
            const uint8_t bits = glyph[row];
            for (int col = 0; col < 5; ++col) {
                if ((bits & (1U << (4 - col))) == 0) {
                    continue;
                }

                for (int sy = 0; sy < scale; ++sy) {
                    for (int sx = 0; sx < scale; ++sx) {
                        setPixel(cursorX + col * scale + sx, cursorY + row * scale + sy, color);
                    }
                }
            }
        }

        cursorX += advance;
        if (cursorX + glyphWidth >= widthPx) {
            cursorX = x;
            cursorY += lineAdvance;
        }
    }
}

void SoftwareRenderer::present() {
    platform::Platform* platformBackend = platform::activePlatform();
    if (platformBackend == nullptr) {
        return;
    }

    // If HDR mode is active, tone-map the float buffer → 8-bit sRGB pixels
    // before handing the buffer to the platform layer.
    if (hdrModeEnabled && !hdrBuffer.empty()) {
        tonemapHdrBuffer();
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

    // Mirror into HDR float buffer as sRGB→linear expanded value.
    if (hdrModeEnabled && !hdrBuffer.empty()) {
        const float r = srgbToLinear(static_cast<float>((color >> 16) & 0xFF) / 255.0f);
        const float g = srgbToLinear(static_cast<float>((color >>  8) & 0xFF) / 255.0f);
        const float b = srgbToLinear(static_cast<float>((color      ) & 0xFF) / 255.0f);
        const float a = static_cast<float>((color >> 24) & 0xFF) / 255.0f;
        const std::size_t fo = offset;  // same index, 4 floats per pixel
        hdrBuffer[fo    ] = r;
        hdrBuffer[fo + 1] = g;
        hdrBuffer[fo + 2] = b;
        hdrBuffer[fo + 3] = a;
    }
}

// ---- HDR pipeline -------------------------------------------------------

void SoftwareRenderer::setHdrMode(bool enabled) {
    hdrModeEnabled = enabled;
    if (enabled && hdrBuffer.empty()) {
        hdrBuffer.assign(static_cast<std::size_t>(widthPx) * static_cast<std::size_t>(heightPx) * 4U, 0.0f);
    }
}

void SoftwareRenderer::clearF(float r, float g, float b) {
    if (!hdrModeEnabled) {
        return;
    }
    if (hdrBuffer.empty()) {
        setHdrMode(true);
    }
    const std::size_t total = static_cast<std::size_t>(widthPx) * static_cast<std::size_t>(heightPx);
    for (std::size_t i = 0; i < total; ++i) {
        hdrBuffer[i * 4    ] = r;
        hdrBuffer[i * 4 + 1] = g;
        hdrBuffer[i * 4 + 2] = b;
        hdrBuffer[i * 4 + 3] = 1.0f;
    }
}

void SoftwareRenderer::drawRectF(int x, int y, int w, int h, float r, float g, float b, float a) {
    if (!hdrModeEnabled || w <= 0 || h <= 0) {
        return;
    }
    if (hdrBuffer.empty()) {
        setHdrMode(true);
    }
    const int x0 = std::max(0, x);
    const int y0 = std::max(0, y);
    const int x1 = std::min(widthPx,  x + w);
    const int y1 = std::min(heightPx, y + h);
    for (int py = y0; py < y1; ++py) {
        for (int px = x0; px < x1; ++px) {
            setPixelF(px, py, r, g, b, a);
        }
    }
}

void SoftwareRenderer::setPixelF(int x, int y, float r, float g, float b, float a) {
    if (x < 0 || x >= widthPx || y < 0 || y >= heightPx) {
        return;
    }
    const std::size_t offset = (static_cast<std::size_t>(y) * static_cast<std::size_t>(widthPx) + static_cast<std::size_t>(x)) * 4U;
    hdrBuffer[offset    ] = r;
    hdrBuffer[offset + 1] = g;
    hdrBuffer[offset + 2] = b;
    hdrBuffer[offset + 3] = a;
}

void SoftwareRenderer::tonemapHdrBuffer() {
    if (hdrBuffer.empty()) {
        return;
    }
    const std::size_t total = static_cast<std::size_t>(widthPx) * static_cast<std::size_t>(heightPx);
    for (std::size_t i = 0; i < total; ++i) {
        // Reinhard per-channel tone mapping then back to 8-bit sRGB.
        const float r8 = std::min(1.0f, linearToSrgb(reinhardTonemap(hdrBuffer[i * 4    ])));
        const float g8 = std::min(1.0f, linearToSrgb(reinhardTonemap(hdrBuffer[i * 4 + 1])));
        const float b8 = std::min(1.0f, linearToSrgb(reinhardTonemap(hdrBuffer[i * 4 + 2])));
        const uint8_t ri = static_cast<uint8_t>(r8 * 255.0f + 0.5f);
        const uint8_t gi = static_cast<uint8_t>(g8 * 255.0f + 0.5f);
        const uint8_t bi = static_cast<uint8_t>(b8 * 255.0f + 0.5f);
        // Pixel layout in memory: B G R A (little-endian uint32 = 0xAARRGGBB)
        pixels[i * 4    ] = bi;
        pixels[i * 4 + 1] = gi;
        pixels[i * 4 + 2] = ri;
        pixels[i * 4 + 3] = 0xFF;
    }
}

// ---- Colour-space helpers ------------------------------------------------

float SoftwareRenderer::srgbToLinear(float c) {
    if (c <= 0.04045f) {
        return c / 12.92f;
    }
    return std::pow((c + 0.055f) / 1.055f, 2.4f);
}

float SoftwareRenderer::linearToSrgb(float c) {
    if (c <= 0.0031308f) {
        return 12.92f * c;
    }
    return 1.055f * std::pow(c, 1.0f / 2.4f) - 0.055f;
}

float SoftwareRenderer::reinhardTonemap(float L) {
    // Simple Reinhard: maps [0,∞) → [0,1).  Preserves SDR colours (<1) well.
    return L / (1.0f + L);
}

} // namespace render
