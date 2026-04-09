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

namespace {

char keyCodeToAscii(int keyCode) {
    if (keyCode >= 32 && keyCode <= 126) {
        return static_cast<char>(keyCode);
    }
    return '\0';
}

std::string clampEditorText(const std::string& text, std::size_t maxChars) {
    if (text.size() <= maxChars) {
        return text;
    }
    return text.substr(text.size() - maxChars, maxChars);
}

} // namespace

int main() {
    core::Application app;
    if (!app.initialize()) {
        return 1;
    }

    render::SoftwareRenderer renderer(900, 640);

    auto root = std::make_shared<ui::View>();
    root->setFrame(0, 0, 900, 640);
    root->setBackgroundColor(0xFF151515U);

    auto title = std::make_shared<ui::Label>("vibed-ui Text Editor Demo");
    title->setFrame(20, 16, 500, 24);

    auto newButton = std::make_shared<ui::Button>("New");
    newButton->setFrame(20, 52, 90, 30);

    auto saveButton = std::make_shared<ui::Button>("Save");
    saveButton->setFrame(118, 52, 90, 30);

    auto editorFrame = std::make_shared<ui::View>();
    editorFrame->setFrame(20, 96, 860, 470);
    editorFrame->setBackgroundColor(0xFF252525U);

    auto editorTextLabel = std::make_shared<ui::Label>("");
    editorTextLabel->setFrame(32, 112, 836, 430);

    auto statusLabel = std::make_shared<ui::Label>("Ready");
    statusLabel->setFrame(20, 580, 860, 30);

    std::string editorText = "Welcome to the vibed-ui editor. Type to append text.";
    bool dirty = false;

    auto refreshUi = [&]() {
        const std::string preview = clampEditorText(editorText, 170);
        editorTextLabel->setText(preview);

        const std::string status = dirty
            ? "Status: Modified | Chars: " + std::to_string(editorText.size())
            : "Status: Saved | Chars: " + std::to_string(editorText.size());
        statusLabel->setText(status);
    };

    newButton->setOnClick([&]() {
        editorText.clear();
        dirty = false;
        refreshUi();
    });

    saveButton->setOnClick([&]() {
        // TODO: Persist editor buffer to a file in the project workspace.
        dirty = false;
        statusLabel->setText("Status: Save requested (stub)");
    });

    editorFrame->addChild(editorTextLabel);

    root->addChild(title);
    root->addChild(newButton);
    root->addChild(saveButton);
    root->addChild(editorFrame);
    root->addChild(statusLabel);

    ui::ViewTree tree;
    tree.setRoot(root);

    refreshUi();

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

            if (event.type == core::EventType::KeyDown) {
                const int inputCode = event.textCode != 0 ? event.textCode : event.keyCode;

                if (inputCode == 8 || inputCode == 127) {
                    if (!editorText.empty()) {
                        editorText.pop_back();
                        dirty = true;
                        refreshUi();
                    }
                    continue;
                }

                if (inputCode == 13 || inputCode == 10) {
                    editorText.push_back(' ');
                    dirty = true;
                    refreshUi();
                    continue;
                }

                const char ch = keyCodeToAscii(inputCode);
                if (ch != '\0') {
                    editorText.push_back(ch);
                    dirty = true;
                    refreshUi();
                }
            }
        }

        renderer.clear(0xFF101010U);
        tree.draw(renderer);
        renderer.present();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    app.shutdown();
    return 0;
}