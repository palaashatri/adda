// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include <functional>
#include <string>

#include "View.h"

namespace ui {

class Button : public View {
public:
    Button(const std::string& text);
    ~Button();

    void setOnClick(std::function<void()> handler);
    void draw(render::Renderer& renderer) override;
    void onEvent(int eventType, int ex, int ey) override;

private:
    std::string label;
    std::function<void()> onClick;
};

} // namespace ui
