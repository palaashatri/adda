// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include "../ui/View.h"

namespace shell {

class Panel : public ui::View {
public:
    Panel();
    ~Panel();

    void draw(render::Renderer& renderer) override;
};

} // namespace shell
