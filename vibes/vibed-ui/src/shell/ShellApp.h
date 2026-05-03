// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include <memory>

#include "../core/Application.h"
#include "../compositor/Compositor.h"
#include "../compositor/Surface.h"
#include "Launcher.h"
#include "Panel.h"

namespace shell {

class ShellApp : public core::Application {
public:
    ShellApp();
    ~ShellApp();

    bool initialize();
    void run();

private:
    Panel panel;
    Launcher launcher;
    compositor::Compositor compositorInstance;
    std::shared_ptr<compositor::Surface> shellSurface;
    std::shared_ptr<compositor::Surface> panelSurface;
    std::shared_ptr<compositor::Surface> launcherSurface;
};

} // namespace shell
