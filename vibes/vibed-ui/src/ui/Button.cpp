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
    const int drawWidth = width > 0 ? width : 120;
    const int drawHeight = height > 0 ? height : 32;

    renderer.drawRoundedRect(x, y, drawWidth, drawHeight, 4, 0xFFCCCCCCU);
    renderer.drawText(x + 6, y + 8, label);
    View::draw(renderer);
}

void Button::onEvent(int eventType, int ex, int ey) {
    if (eventType == static_cast<int>(core::EventType::MouseDown)) {
        const int drawWidth = width > 0 ? width : 120;
        const int drawHeight = height > 0 ? height : 32;
        if (ex >= x && ex <= x + drawWidth && ey >= y && ey <= y + drawHeight) {
            if (onClick) {
                onClick();
            }
        }
    }

    View::onEvent(eventType, ex, ey);
}

} // namespace ui