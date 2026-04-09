// MIT License
// Copyright (c) 2026 Palaash

#include "Color.h"

namespace ui {

Color::Color() {}

Color::Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
    : r(red), g(green), b(blue), a(alpha) {}

Color::~Color() {}

uint32_t Color::toUint32() const {
    return (static_cast<uint32_t>(a) << 24U)
        | (static_cast<uint32_t>(r) << 16U)
        | (static_cast<uint32_t>(g) << 8U)
        | static_cast<uint32_t>(b);
}

Color Color::fromUint32(uint32_t value) {
    return Color(
        static_cast<uint8_t>((value >> 16U) & 0xFFU),
        static_cast<uint8_t>((value >> 8U) & 0xFFU),
        static_cast<uint8_t>(value & 0xFFU),
        static_cast<uint8_t>((value >> 24U) & 0xFFU));
}

} // namespace ui
