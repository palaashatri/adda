// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include "View.h"

namespace ui {

class Layout {
public:
    Layout();
    virtual ~Layout();

    virtual void apply(View& parent) = 0;
};

} // namespace ui
