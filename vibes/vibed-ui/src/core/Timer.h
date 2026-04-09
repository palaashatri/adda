// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include <chrono>

namespace core {

class Timer {
public:
    Timer();
    ~Timer();

    void start();
    void stop();
    double elapsedSeconds() const;

private:
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point stopTime;
    bool started = false;
    bool stopped = false;
};

} // namespace core