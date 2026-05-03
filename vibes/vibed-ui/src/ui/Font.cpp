// MIT License
// Copyright (c) 2026 Palaash

#include "Font.h"

namespace ui {

Font::Font() : fontName("Sans"), fontSize(12) {}

Font::Font(const std::string& name, int size)
    : fontName(name), fontSize(size > 0 ? size : 12) {}

Font::~Font() {}

void Font::setName(const std::string& name) {
    fontName = name;
}

const std::string& Font::name() const {
    return fontName;
}

void Font::setSize(int size) {
    if (size > 0) {
        fontSize = size;
    }
}

int Font::size() const {
    return fontSize;
}

} // namespace ui
