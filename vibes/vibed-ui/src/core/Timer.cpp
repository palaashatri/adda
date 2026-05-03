// MIT License
// Copyright (c) 2026 Palaash

#include "Timer.h"

namespace core {

Timer::Timer() {}

Timer::~Timer() {}

void Timer::start() {
    started = true;
    stopped = false;
    startTime = std::chrono::steady_clock::now();
}

void Timer::stop() {
    if (!started) {
        return;
    }
    stopped = true;
    stopTime = std::chrono::steady_clock::now();
}

double Timer::elapsedSeconds() const {
    if (!started) {
        return 0.0;
    }

    const std::chrono::steady_clock::time_point endTime = stopped
        ? stopTime
        : std::chrono::steady_clock::now();

    const std::chrono::duration<double> delta = endTime - startTime;
    return delta.count();
}

} // namespace core