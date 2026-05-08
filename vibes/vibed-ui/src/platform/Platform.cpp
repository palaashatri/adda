// MIT License
// Copyright (c) 2026 Palaash

#include "Platform.h"

#if defined(_WIN32)
#include "windows/WinPlatform.h"
#elif defined(__APPLE__)
#include "mac/MacPlatform.h"
#elif defined(__linux__)
#include "linux/LinuxPlatform.h"
#endif

namespace platform {

namespace {

Platform* g_activePlatform = nullptr;

} // namespace

Platform::Platform() {}

Platform::~Platform() {}

std::unique_ptr<Platform> createPlatform() {
#if defined(_WIN32)
    return std::make_unique<WinPlatform>();
#elif defined(__APPLE__)
    return std::make_unique<MacPlatform>();
#elif defined(__linux__)
    return std::make_unique<LinuxPlatform>();
#else
    return nullptr;
#endif
}

Platform* activePlatform() {
    return g_activePlatform;
}

void setActivePlatform(Platform* platformInstance) {
    g_activePlatform = platformInstance;
}

} // namespace platform
