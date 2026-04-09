// MIT License
// Copyright (c) 2026 Palaash

#include "ShellApp.h"

#include "../core/Event.h"
#include "../platform/Platform.h"
#include "../render/legacy/SoftwareRenderer.h"

namespace shell {

ShellApp::ShellApp() {}

ShellApp::~ShellApp() {}

bool ShellApp::initialize() {
    if (!core::Application::initialize()) {
        return false;
    }

    panel.setFrame(0, 0, 800, 40);
    launcher.setFrame(0, 40, 220, 380);
    panel.setBackgroundColor(0xFF222222U);
    launcher.setBackgroundColor(0xFF444444U);

    shellSurface = std::make_shared<compositor::Surface>(800, 600);
    shellSurface->setPosition(0, 0);

    panelSurface = std::make_shared<compositor::Surface>(800, 40);
    panelSurface->setPosition(0, 0);

    launcherSurface = std::make_shared<compositor::Surface>(220, 380);
    launcherSurface->setPosition(0, 40);

    compositorInstance.windowManager().addSurface(shellSurface);
    compositorInstance.windowManager().addSurface(panelSurface);
    compositorInstance.windowManager().addSurface(launcherSurface);

    return true;
}

void ShellApp::run() {
    render::SoftwareRenderer renderer(800, 600);

    bool shouldExit = false;
    for (int frame = 0; frame < 240 && !shouldExit; ++frame) {
        if (platform::activePlatform() != nullptr) {
            platform::activePlatform()->pumpEvents();
        }

        core::Event event;
        while (core::EventQueue::poll(event)) {
            if (event.type == core::EventType::Quit) {
                shouldExit = true;
                break;
            }

            compositorInstance.processEvent(event);
            panel.onEvent(static_cast<int>(event.type), event.x, event.y);
            launcher.onEvent(static_cast<int>(event.type), event.x, event.y);
        }

        compositorInstance.windowManager().renderAll();

        const std::shared_ptr<compositor::Surface> focused = compositorInstance.windowManager().focusedSurface();
        if (focused == panelSurface) {
            panel.setBackgroundColor(0xFF2F3A46U);
            launcher.setBackgroundColor(0xFF444444U);
        } else if (focused == launcherSurface) {
            panel.setBackgroundColor(0xFF222222U);
            launcher.setBackgroundColor(0xFF5A5A5AU);
        } else {
            panel.setBackgroundColor(0xFF222222U);
            launcher.setBackgroundColor(0xFF444444U);
        }

        renderer.clear(0xFF111111U);
        panel.draw(renderer);
        launcher.draw(renderer);
        renderer.present();
    }
}

} // namespace shell
