// MIT License
// Copyright (c) 2026 Palaash

#include <chrono>
#include <memory>
#include <string>
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
    app.setWindowSize(900, 620);
    if (!app.initialize()) {
        return 1;
    }

    render::SoftwareRenderer renderer(900, 620);

    auto root = std::make_shared<ui::View>();
    root->setFrame(0, 0, 900, 620);
    root->setBackgroundColor(0xFF1A1A1AU);

    auto title = std::make_shared<ui::Label>("Event Inspector");
    title->setFrame(20, 18, 320, 26);

    auto resetButton = std::make_shared<ui::Button>("Reset Counters");
    resetButton->setFrame(20, 56, 170, 30);

    auto mouseDownLabel = std::make_shared<ui::Label>("MouseDown: 0");
    mouseDownLabel->setFrame(20, 110, 360, 26);

    auto mouseMoveLabel = std::make_shared<ui::Label>("MouseMove: 0");
    mouseMoveLabel->setFrame(20, 144, 360, 26);

    auto keyDownLabel = std::make_shared<ui::Label>("KeyDown: 0");
    keyDownLabel->setFrame(20, 178, 360, 26);

    auto lastEventLabel = std::make_shared<ui::Label>("Last Event: none");
    lastEventLabel->setFrame(20, 212, 820, 26);

    int mouseDownCount = 0;
    int mouseMoveCount = 0;
    int keyDownCount = 0;

    auto refresh = [&]() {
        mouseDownLabel->setText("MouseDown: " + std::to_string(mouseDownCount));
        mouseMoveLabel->setText("MouseMove: " + std::to_string(mouseMoveCount));
        keyDownLabel->setText("KeyDown: " + std::to_string(keyDownCount));
    };

    resetButton->setOnClick([&]() {
        mouseDownCount = 0;
        mouseMoveCount = 0;
        keyDownCount = 0;
        lastEventLabel->setText("Last Event: counters reset");
        refresh();
    });

    root->addChild(title);
    root->addChild(resetButton);
    root->addChild(mouseDownLabel);
    root->addChild(mouseMoveLabel);
    root->addChild(keyDownLabel);
    root->addChild(lastEventLabel);

    ui::ViewTree tree;
    tree.setRoot(root);

    refresh();

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

            if (event.type == core::EventType::MouseDown) {
                ++mouseDownCount;
                lastEventLabel->setText("Last Event: MouseDown @ (" + std::to_string(event.x) + ", " + std::to_string(event.y) + ")");
                refresh();
            } else if (event.type == core::EventType::MouseMove) {
                ++mouseMoveCount;
                refresh();
            } else if (event.type == core::EventType::KeyDown) {
                ++keyDownCount;
                lastEventLabel->setText("Last Event: KeyDown code=" + std::to_string(event.keyCode));
                refresh();
            }

            tree.onEvent(static_cast<int>(event.type), event.x, event.y);
        }

        renderer.clear(0xFF0F0F0FU);
        tree.draw(renderer);
        renderer.present();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    app.shutdown();
    return 0;
}