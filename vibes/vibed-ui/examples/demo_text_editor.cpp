// MIT License
// Copyright (c) 2026 Palaash

#include <chrono>
#include <cstdio>
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
    app.setWindowSize(900, 640);
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
    bool cursorVisible = true;
    int frameTick = 0;

    auto refreshUi = [&]() {
        const std::string preview = clampEditorText(editorText, 170);
        editorTextLabel->setText(preview + (cursorVisible ? "|" : " "));

        const std::string status = dirty
            ? "Modified  |  Chars: " + std::to_string(editorText.size()) +
              "  |  Backspace/Del to erase  |  Save writes editor_output.txt"
            : "Ready     |  Chars: " + std::to_string(editorText.size()) +
              "  |  Type to edit";
        statusLabel->setText(status);
    };

    newButton->setOnClick([&]() {
        editorText.clear();
        dirty = false;
        refreshUi();
    });

    saveButton->setOnClick([&]() {
        FILE* fp = nullptr;
#if defined(_MSC_VER)
        fopen_s(&fp, "editor_output.txt", "w");
#else
        fp = std::fopen("editor_output.txt", "w");
#endif
        if (fp != nullptr) {
            std::fwrite(editorText.c_str(), 1, editorText.size(), fp);
            std::fclose(fp);
            dirty = false;
            statusLabel->setText("Status: Saved to editor_output.txt | Chars: " + std::to_string(editorText.size()));
        } else {
            statusLabel->setText("Status: Save failed - check write permissions");
        }
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

                if (inputCode == 8 || inputCode == 127 || inputCode == 46) {
                    // 8 = backspace (WM_CHAR), 127 = Unix DEL, 46 = VK_DELETE
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

        // Toggle cursor visibility every 30 frames (~0.5 s at 60 fps).
        ++frameTick;
        if (frameTick >= 30) {
            frameTick = 0;
            cursorVisible = !cursorVisible;
            refreshUi();
        }

        renderer.clear(0xFF101010U);
        tree.draw(renderer);
        renderer.present();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    app.shutdown();
    return 0;
}