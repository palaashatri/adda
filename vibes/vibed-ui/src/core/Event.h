// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include <cstddef>

namespace core {

enum class EventType {
    None = 0,
    MouseDown,
    MouseUp,
    MouseMove,
    KeyDown,
    KeyUp,
    Quit,
};

class Event {
public:
    Event();
    ~Event();

    EventType type = EventType::None;
    int x = 0;
    int y = 0;
    int keyCode = 0;
};

class EventQueue {
public:
    EventQueue();
    ~EventQueue();

    static void push(const Event& event);
    static bool poll(Event& event);
    static void clear();
    static std::size_t size();
};

} // namespace core