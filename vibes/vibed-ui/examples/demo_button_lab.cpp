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
    if (!app.initialize()) {
        return 1;
    }

    render::SoftwareRenderer renderer(900, 620);

    auto root = std::make_shared<ui::View>();
    root->setFrame(0, 0, 900, 620);
    root->setBackgroundColor(0xFF1C1C2AU);

    auto title = std::make_shared<ui::Label>("Button Lab Demo");
    title->setFrame(20, 18, 320, 26);

    auto status = std::make_shared<ui::Label>("Press any button");
    status->setFrame(20, 520, 840, 26);

    auto b1 = std::make_shared<ui::Button>("Primary");
    auto b2 = std::make_shared<ui::Button>("Secondary");
    auto b3 = std::make_shared<ui::Button>("Accent");
    auto b4 = std::make_shared<ui::Button>("Danger");
    auto b5 = std::make_shared<ui::Button>("Toggle BG");
    auto b6 = std::make_shared<ui::Button>("Reset");

    b1->setFrame(20, 80, 180, 42);
    b2->setFrame(220, 80, 180, 42);
    b3->setFrame(420, 80, 180, 42);
    b4->setFrame(620, 80, 180, 42);
    b5->setFrame(20, 140, 180, 42);
    b6->setFrame(220, 140, 180, 42);

    bool darkBg = true;
    int clickCount = 0;

    auto updateStatus = [&](const std::string& source) {
        ++clickCount;
        status->setText("Last: " + source + " | Total Clicks: " + std::to_string(clickCount));
    };

    b1->setOnClick([&]() { updateStatus("Primary"); });
    b2->setOnClick([&]() { updateStatus("Secondary"); });
    b3->setOnClick([&]() { updateStatus("Accent"); });
    b4->setOnClick([&]() { updateStatus("Danger"); });
    b5->setOnClick([&]() {
        darkBg = !darkBg;
        root->setBackgroundColor(darkBg ? 0xFF1C1C2AU : 0xFFE6E6F0U);
        updateStatus("Toggle BG");
    });
    b6->setOnClick([&]() {
        clickCount = 0;
        darkBg = true;
        root->setBackgroundColor(0xFF1C1C2AU);
        status->setText("Reset complete");
    });

    root->addChild(title);
    root->addChild(status);
    root->addChild(b1);
    root->addChild(b2);
    root->addChild(b3);
    root->addChild(b4);
    root->addChild(b5);
    root->addChild(b6);

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

        renderer.clear(0xFF121212U);
        tree.draw(renderer);
        renderer.present();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    app.shutdown();
    return 0;
}