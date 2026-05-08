// MIT License
// Copyright (c) 2026 Palaash

#include "Label.h"

namespace ui {

Label::Label(const std::string& t)
    : labelText(t) {}

Label::~Label() {}

void Label::setText(const std::string& t) {
    labelText = t;
}

const std::string& Label::text() const {
    return labelText;
}

void Label::draw(render::Renderer& renderer) {
    View::draw(renderer);
    renderer.drawText(x, y, labelText);
}

} // namespace ui
