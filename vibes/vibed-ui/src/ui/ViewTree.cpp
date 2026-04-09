// MIT License
// Copyright (c) 2026 Palaash

#include "ViewTree.h"

namespace ui {

ViewTree::ViewTree() {}

ViewTree::~ViewTree() {}

void ViewTree::setRoot(std::shared_ptr<View> view) {
    rootView = view;
}

const std::shared_ptr<View>& ViewTree::root() const {
    return rootView;
}

void ViewTree::draw(render::Renderer& renderer) {
    if (rootView) {
        rootView->draw(renderer);
    }
}

void ViewTree::layout() {
    if (rootView) {
        rootView->layout();
    }
}

void ViewTree::onEvent(int eventType, int x, int y) {
    if (rootView) {
        rootView->onEvent(eventType, x, y);
    }
}

} // namespace ui
