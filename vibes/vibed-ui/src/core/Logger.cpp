// MIT License
// Copyright (c) 2026 Palaash

#include "Logger.h"

#include <iostream>

namespace core {

Logger::Logger() {}

Logger::~Logger() {}

void Logger::info(const std::string& message) {
    std::cout << "[INFO] " << message << std::endl;
}

void Logger::warn(const std::string& message) {
    std::cout << "[WARN] " << message << std::endl;
}

void Logger::error(const std::string& message) {
    std::cerr << "[ERROR] " << message << std::endl;
}

} // namespace core