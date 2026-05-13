// MIT License
// Copyright (c) 2026 Palaash

#include "ShellApp.h"

#include <chrono>
#include <thread>

#include "../core/Event.h"
#include "../platform/Platform.h"

namespace shell {

ShellApp::ShellApp() {}

ShellApp::~ShellApp() {}

bool ShellApp::initialize() {
    if (!core::Application::initialize()) {
        return false;
    }

    panel.setFrame(0, 0, 800, 40);
    launcher.setFrame(0, 0, 220, 380);
    panel.setBackgroundColor(0xFF222222U);
    launcher.setBackgroundColor(0xFF444444U);

    // Background canvas for the desktop area behind panel and launcher.
    shellSurface = std::make_shared<compositor::Surface>(800, 600);
    shellSurface->setPosition(0, 0);

    // Panel occupies the top 40 px.
    panelSurface = std::make_shared<compositor::Surface>(800, 40);
    panelSurface->setPosition(0, 0);

    // Launcher sits below the panel.
    launcherSurface = std::make_shared<compositor::Surface>(220, 380);
    launcherSurface->setPosition(0, 40);

    compositor::WindowManager& wm = compositorInstance.windowManager();
    wm.setViewport(800, 600);
    wm.addSurface(shellSurface);
    wm.addSurface(panelSurface);
    wm.addSurface(launcherSurface);

    return true;
}

void ShellApp::run() {
    bool shouldExit = false;
    while (!shouldExit) {
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

        // Update panel/launcher highlight based on compositor focus.
        const std::shared_ptr<compositor::Surface> focused =
            compositorInstance.windowManager().focusedSurface();

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

        // Render desktop background into its surface.
        shellSurface->clear(0xFF111111U);

        // Render panel into its dedicated surface.
        panelSurface->clear(0xFF111111U);
        panel.draw(panelSurface->renderer());

        // Render launcher into its dedicated surface.
        launcherSurface->clear(0xFF111111U);
        launcher.draw(launcherSurface->renderer());

        // Compositor composites all surfaces and blits the result to screen.
        compositorInstance.windowManager().renderAll();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

} // namespace shell
