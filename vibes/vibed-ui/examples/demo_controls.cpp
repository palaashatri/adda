// MIT License
// Copyright (c) 2026 Palaash

#include <memory>

#include "core/Application.h"
#include "core/Event.h"
#include "platform/Platform.h"
#include "render/legacy/SoftwareRenderer.h"
#include "ui/Button.h"
#include "ui/FlexLayout.h"
#include "ui/Label.h"
#include "ui/Theme.h"
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
    root->setBackgroundColor(0xFF101010U);

    auto label = std::make_shared<ui::Label>("Controls Demo");
    auto b1 = std::make_shared<ui::Button>("One");
    auto b2 = std::make_shared<ui::Button>("Two");
    auto b3 = std::make_shared<ui::Button>("Toggle Theme");

    bool darkTheme = true;
    b3->setOnClick([&root, &label, &darkTheme]() {
        darkTheme = !darkTheme;
        if (darkTheme) {
            root->setBackgroundColor(0xFF101010U);
            label->setText("Controls Demo (Dark)");
        } else {
            root->setBackgroundColor(0xFFF0F0F0U);
            label->setText("Controls Demo (Light)");
        }
    });

    root->addChild(label);
    root->addChild(b1);
    root->addChild(b2);
    root->addChild(b3);

    ui::FlexLayout layout;
    layout.setSpacing(8);
    layout.setVertical(true);
    layout.apply(*root);

    ui::ViewTree tree;
    tree.setRoot(root);

    ui::Theme theme;
    (void)theme;

    bool shouldExit = false;
    for (int frame = 0; frame < 120 && !shouldExit; ++frame) {
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
    }

    app.shutdown();
    return 0;
}