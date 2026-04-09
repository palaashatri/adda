// MIT License
// Copyright (c) 2026 Palaash

#include "SoftwareRenderer.h"

#include <algorithm>
#include <cctype>
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
    if (ch >= 'a' && ch <= 'z') {
        ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    }

    switch (ch) {
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
        case ' ': return kGlyphSpace;
        case '.': { static const uint8_t g[7] = {0,0,0,0,0,0x0C,0x0C}; return g; }
        case ',': { static const uint8_t g[7] = {0,0,0,0,0,0x0C,0x08}; return g; }
        case ':': { static const uint8_t g[7] = {0,0x0C,0x0C,0,0x0C,0x0C,0}; return g; }
        case ';': { static const uint8_t g[7] = {0,0x0C,0x0C,0,0x0C,0x08,0}; return g; }
        case '|': { static const uint8_t g[7] = {0x04,0x04,0x04,0x04,0x04,0x04,0x04}; return g; }
        case '-': { static const uint8_t g[7] = {0,0,0,0x1F,0,0,0}; return g; }
        case '_': { static const uint8_t g[7] = {0,0,0,0,0,0,0x1F}; return g; }
        case '/': { static const uint8_t g[7] = {0x01,0x02,0x04,0x08,0x10,0,0}; return g; }
        case '(': { static const uint8_t g[7] = {0x02,0x04,0x08,0x08,0x08,0x04,0x02}; return g; }
        case ')': { static const uint8_t g[7] = {0x08,0x04,0x02,0x02,0x02,0x04,0x08}; return g; }
        case '!': { static const uint8_t g[7] = {0x04,0x04,0x04,0x04,0x04,0,0x04}; return g; }
        case '?': { static const uint8_t g[7] = {0x0E,0x11,0x01,0x02,0x04,0,0x04}; return g; }
        case '\'': { static const uint8_t g[7] = {0x04,0x04,0x08,0,0,0,0}; return g; }
        case '"': { static const uint8_t g[7] = {0x0A,0x0A,0x0A,0,0,0,0}; return g; }
        case '+': { static const uint8_t g[7] = {0,0x04,0x04,0x1F,0x04,0x04,0}; return g; }
        case '=': { static const uint8_t g[7] = {0,0x1F,0,0x1F,0,0,0}; return g; }
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
    (void)radius;
    // TODO: Implement corner clipping for rounded rectangle rendering.
    drawRect(x, y, w, h, color);
}

void SoftwareRenderer::drawText(int x, int y, const std::string& text) {
    // TODO: Replace fallback 5x7 font with full glyph rasterization and kerning.
    const uint32_t color = 0xFFEAEAEAU;
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
