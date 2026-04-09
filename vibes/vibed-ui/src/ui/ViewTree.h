// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include <memory>

#include "View.h"

namespace ui {

class ViewTree {
public:
    ViewTree();
    ~ViewTree();

    void setRoot(std::shared_ptr<View> view);
    const std::shared_ptr<View>& root() const;

    void draw(render::Renderer& renderer);
    void layout();
    void onEvent(int eventType, int x, int y);

private:
    std::shared_ptr<View> rootView;
};

} // namespace ui
