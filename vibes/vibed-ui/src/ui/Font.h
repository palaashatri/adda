// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include <string>

namespace ui {

class Font {
public:
    Font();
    Font(const std::string& name, int size);
    ~Font();

    void setName(const std::string& name);
    const std::string& name() const;

    void setSize(int size);
    int size() const;

private:
    std::string fontName;
    int fontSize = 12;
};

} // namespace ui
