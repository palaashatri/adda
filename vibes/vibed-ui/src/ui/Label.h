// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include <string>

#include "View.h"

namespace ui {

class Label : public View {
public:
    Label(const std::string& text);
    ~Label();

    void setText(const std::string& t);
    const std::string& text() const;

    void draw(render::Renderer& renderer) override;

private:
    std::string labelText;
};

} // namespace ui
