// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include <string>
#include <unordered_map>

#include "Color.h"
#include "Font.h"

namespace ui {

class Theme {
public:
    Theme();
    ~Theme();

    bool loadFromJson(const std::string& path);

    const Color& color(const std::string& name) const;
    int radius(const std::string& name) const;
    const Font& font(const std::string& name) const;

private:
    std::unordered_map<std::string, Color> colors;
    std::unordered_map<std::string, int> radii;
    std::unordered_map<std::string, Font> fonts;

    Color fallbackColor;
    Font fallbackFont;
    int fallbackRadius = 0;
};

} // namespace ui
