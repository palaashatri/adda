// MIT License
// Copyright (c) 2026 Palaash

#include "ModernRenderer.h"

namespace render {

ModernRenderer::ModernRenderer(int width, int height)
    : delegate(width, height) {}

ModernRenderer::~ModernRenderer() {}

void ModernRenderer::clear(uint32_t color) {
    delegate.clear(color);
}

void ModernRenderer::drawRect(int x, int y, int w, int h, uint32_t color) {
    delegate.drawRect(x, y, w, h, color);
}

void ModernRenderer::drawRoundedRect(int x, int y, int w, int h, int radius, uint32_t color) {
    delegate.drawRoundedRect(x, y, w, h, radius, color);
}

void ModernRenderer::drawText(int x, int y, const std::string& text) {
    delegate.drawText(x, y, text);
}

void ModernRenderer::present() {
    delegate.present();
}

} // namespace render
