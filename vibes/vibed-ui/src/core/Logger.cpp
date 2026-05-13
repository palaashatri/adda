// MIT License
// Copyright (c) 2026 Palaash

#include "Logger.h"

#include <iostream>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace core {

Logger::Logger() {}

Logger::~Logger() {}

void Logger::info(const std::string& message) {
    std::cout << "[INFO] " << message << "\n";
#if defined(_WIN32)
    OutputDebugStringA(("[INFO] " + message + "\n").c_str());
#endif
}

void Logger::warn(const std::string& message) {
    std::cout << "[WARN] " << message << "\n";
#if defined(_WIN32)
    OutputDebugStringA(("[WARN] " + message + "\n").c_str());
#endif
}

void Logger::error(const std::string& message) {
    std::cerr << "[ERROR] " << message << "\n";
#if defined(_WIN32)
    OutputDebugStringA(("[ERROR] " + message + "\n").c_str());
#endif
}

} // namespace core
