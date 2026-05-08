// MIT License
// Copyright (c) 2026 Palaash

#include "Application.h"

#include "Event.h"
#include "Logger.h"
#include "platform/Platform.h"

namespace core {

Application::Application() {}

Application::~Application() {}

bool Application::initialize() {
    platformBackend = platform::createPlatform();
    if (!platformBackend) {
        Logger::error("No platform backend available");
        return false;
    }

    if (!platformBackend->createWindow(windowWidth, windowHeight, "vibed-ui")) {
        Logger::error("Failed to create platform window");
        platformBackend.reset();
        return false;
    }

    EventQueue::clear();
    platform::setActivePlatform(platformBackend.get());
    Logger::info("Application initialized");
    running = true;
    return true;
}

void Application::run() {
    while (running) {
        if (platformBackend) {
            platformBackend->pumpEvents();
        }

        Event event;
        while (EventQueue::poll(event)) {
            if (event.type == EventType::Quit) {
                running = false;
                break;
            }
        }

        if (!running) {
            break;
        }

        // TODO: Rendering.
    }
}

void Application::setWindowSize(int w, int h) {
    windowWidth = w;
    windowHeight = h;
}

void Application::shutdown() {
    running = false;
    EventQueue::clear();
    platform::setActivePlatform(nullptr);
    platformBackend.reset();
    Logger::info("Application shutdown");
}

} // namespace core