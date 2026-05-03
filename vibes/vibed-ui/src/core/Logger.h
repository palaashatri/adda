// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include <string>

namespace core {

class Logger {
public:
    Logger();
    ~Logger();

    static void info(const std::string& message);
    static void warn(const std::string& message);
    static void error(const std::string& message);
};

} // namespace core