// MIT License
// Copyright (c) 2026 Palaash

#include "WindowManager.h"

#include <utility>

namespace compositor {

WindowManager::WindowManager() {}

WindowManager::~WindowManager() {}

void WindowManager::addSurface(std::shared_ptr<Surface> s) {
    if (s) {
        surfaces.push_back(s);
        focusedSurfaceIndex = static_cast<int>(surfaces.size()) - 1;
    }
}

void WindowManager::renderAll() {
    // TODO: Composite all visible surfaces into output buffer.
    for (const auto& surface : surfaces) {
        if (surface == nullptr || !surface->isVisible()) {
            continue;
        }
        (void)surface;
    }
}

void WindowManager::focusSurfaceAt(int x, int y) {
    if (surfaces.empty()) {
        focusedSurfaceIndex = -1;
        return;
    }

    for (int i = static_cast<int>(surfaces.size()) - 1; i >= 0; --i) {
        const auto& surface = surfaces[static_cast<std::size_t>(i)];
        if (surface != nullptr && surface->isVisible() && surface->containsPoint(x, y)) {
            bringToFront(static_cast<std::size_t>(i));
            focusedSurfaceIndex = static_cast<int>(surfaces.size()) - 1;
            return;
        }
    }
}

int WindowManager::focusedIndex() const {
    return focusedSurfaceIndex;
}

std::shared_ptr<Surface> WindowManager::focusedSurface() const {
    if (focusedSurfaceIndex < 0 || focusedSurfaceIndex >= static_cast<int>(surfaces.size())) {
        return nullptr;
    }
    return surfaces[static_cast<std::size_t>(focusedSurfaceIndex)];
}

const std::vector<std::shared_ptr<Surface>>& WindowManager::allSurfaces() const {
    return surfaces;
}

void WindowManager::bringToFront(std::size_t index) {
    if (index >= surfaces.size() || index == surfaces.size() - 1) {
        return;
    }

    std::shared_ptr<Surface> selected = std::move(surfaces[index]);
    surfaces.erase(surfaces.begin() + static_cast<std::ptrdiff_t>(index));
    surfaces.push_back(std::move(selected));
}

} // namespace compositor
