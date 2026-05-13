// MIT License
// Copyright (c) 2026 Palaash

#include "Button.h"

#include "core/Event.h"

namespace ui {

Button::Button(const std::string& t)
    : label(t) {}

Button::~Button() {}

void Button::setOnClick(std::function<void()> handler) {
    onClick = handler;
}

void Button::draw(render::Renderer& renderer) {
    const int drawWidth  = width  > 0 ? width  : 120;
    const int drawHeight = height > 0 ? height : 32;

    // Three-state shading: pressed → darker, hovered → lighter, default.
    const uint32_t bgColor = pressed  ? 0xFF888888U
                           : hovered  ? 0xFFDDDDDDU
                           :            0xFFCCCCCCU;

    renderer.drawRoundedRect(x, y, drawWidth, drawHeight, 4, bgColor);
    renderer.drawText(x + 6, y + 8, label);
    View::draw(renderer);
}

void Button::onEvent(int eventType, int ex, int ey) {
    const int drawWidth  = width  > 0 ? width  : 120;
    const int drawHeight = height > 0 ? height : 32;
    const bool hit = (ex >= x && ex <= x + drawWidth &&
                      ey >= y && ey <= y + drawHeight);

    if (eventType == static_cast<int>(core::EventType::MouseMove)) {
        hovered = hit;
    } else if (eventType == static_cast<int>(core::EventType::MouseDown)) {
        if (hit) {
            pressed = true;
            if (onClick) {
                onClick();
            }
        }
    } else if (eventType == static_cast<int>(core::EventType::MouseUp)) {
        pressed = false;
    }

    View::onEvent(eventType, ex, ey);
}

} // namespace ui
