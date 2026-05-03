// MIT License
// Copyright (c) 2026 Palaash

#include <chrono>
#include <memory>
#include <thread>

#include "core/Application.h"
#include "core/Event.h"
#include "platform/Platform.h"
#include "render/legacy/SoftwareRenderer.h"
#include "ui/Button.h"
#include "ui/Label.h"
#include "ui/View.h"
#include "ui/ViewTree.h"

int main() {
    core::Application app;
    if (!app.initialize()) {
        return 1;
    }

    render::SoftwareRenderer renderer(800, 600);

    auto root = std::make_shared<ui::View>();
    root->setFrame(0, 0, 800, 600);
    root->setBackgroundColor(0xFF000000U);

    auto label = std::make_shared<ui::Label>("Hello World");
    label->setFrame(10, 10, 240, 30);

    auto button = std::make_shared<ui::Button>("Click Me");
    button->setFrame(10, 50, 200, 30);
    button->setOnClick([&label]() {
        label->setText("Button Clicked!");
    });

    root->addChild(label);
    root->addChild(button);

    ui::ViewTree tree;
    tree.setRoot(root);

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

            tree.onEvent(static_cast<int>(event.type), event.x, event.y);
        }

        renderer.clear(0xFF000000U);
        tree.draw(renderer);
        renderer.present();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    app.shutdown();
    return 0;
}
