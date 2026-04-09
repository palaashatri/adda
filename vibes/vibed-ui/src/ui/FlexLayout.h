// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include "Layout.h"

namespace ui {

class FlexLayout : public Layout {
public:
    FlexLayout();
    ~FlexLayout();

    void setSpacing(int value);
    void setVertical(bool value);

    void apply(View& parent) override;

private:
    int spacing = 0;
    bool vertical = true;
};

} // namespace ui
