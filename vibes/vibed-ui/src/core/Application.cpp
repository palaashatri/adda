// MIT License
// Copyright (c) 2026 Palaash

#include "Application.h"

#include <chrono>
#include <thread>

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

        if (frameCallback) {
            frameCallback();
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }
}

void Application::setWindowSize(int w, int h) {
    windowWidth  = w;
    windowHeight = h;
}

void Application::setFrameCallback(std::function<void()> cb) {
    frameCallback = std::move(cb);
}

void Application::shutdown() {
    running = false;
    EventQueue::clear();
    platform::setActivePlatform(nullptr);
    platformBackend.reset();
    Logger::info("Application shutdown");
}

} // namespace core
