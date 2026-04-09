// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include <cstdint>

namespace ui {

class Color {
public:
    Color();
    Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255);
    ~Color();

    uint32_t toUint32() const;
    static Color fromUint32(uint32_t value);

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;
};

} // namespace ui
