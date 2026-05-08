// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include "../ui/View.h"

namespace shell {

class Launcher : public ui::View {
public:
    Launcher();
    ~Launcher();

    void draw(render::Renderer& renderer) override;
};

} // namespace shell
