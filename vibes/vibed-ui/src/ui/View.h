// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "../render/Renderer.h"

namespace ui {

class View {
public:
    View();
    virtual ~View();

    void setFrame(int x, int y, int w, int h);
    void getFrame(int& x, int& y, int& w, int& h) const;

    void setBackgroundColor(uint32_t color);
    uint32_t backgroundColorValue() const;

    void addChild(std::shared_ptr<View> child);
    const std::vector<std::shared_ptr<View>>& children() const;

    virtual void draw(render::Renderer& renderer);
    virtual void layout();
    virtual void onEvent(int eventType, int x, int y);

protected:
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    uint32_t backgroundColor = 0x00000000U;

    std::vector<std::shared_ptr<View>> childViews;
};

} // namespace ui
