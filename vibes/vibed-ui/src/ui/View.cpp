// MIT License
// Copyright (c) 2026 Palaash

#include "View.h"

namespace ui {

View::View() {}

View::~View() {}

void View::setFrame(int px, int py, int w, int h) {
    x = px;
    y = py;
    width = w;
    height = h;
}

void View::getFrame(int& px, int& py, int& w, int& h) const {
    px = x;
    py = y;
    w = width;
    h = height;
}

void View::setBackgroundColor(uint32_t color) {
    backgroundColor = color;
}

uint32_t View::backgroundColorValue() const {
    return backgroundColor;
}

void View::addChild(std::shared_ptr<View> child) {
    if (child) {
        childViews.push_back(child);
    }
}

const std::vector<std::shared_ptr<View>>& View::children() const {
    return childViews;
}

void View::draw(render::Renderer& renderer) {
    if (width > 0 && height > 0 && backgroundColor != 0x00000000U) {
        renderer.drawRect(x, y, width, height, backgroundColor);
    }

    for (const auto& child : childViews) {
        child->draw(renderer);
    }
}

void View::layout() {
    for (const auto& child : childViews) {
        child->layout();
    }
}

void View::onEvent(int eventType, int ex, int ey) {
    for (const auto& child : childViews) {
        child->onEvent(eventType, ex, ey);
    }
}

} // namespace ui
