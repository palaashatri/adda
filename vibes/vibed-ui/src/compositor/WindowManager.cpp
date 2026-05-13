// MIT License
// Copyright (c) 2026 Palaash

#include "WindowManager.h"

#include <cstring>
#include <utility>

#include "platform/Platform.h"

namespace compositor {

WindowManager::WindowManager() {}

WindowManager::~WindowManager() {}

void WindowManager::addSurface(std::shared_ptr<Surface> s) {
    if (s) {
        surfaces.push_back(s);
        focusedSurfaceIndex = static_cast<int>(surfaces.size()) - 1;
    }
}

void WindowManager::setViewport(int w, int h) {
    viewportW = w;
    viewportH = h;
    composite.assign(
        static_cast<std::size_t>(w) * static_cast<std::size_t>(h) * 4U,
        0U);
}

void WindowManager::renderAll() {
    if (viewportW <= 0 || viewportH <= 0 || composite.empty()) {
        return;
    }

    // Fill composite buffer with opaque black background.
    const uint32_t bg = 0xFF111111U;
    const int totalPx = viewportW * viewportH;
    for (int i = 0; i < totalPx; ++i) {
        std::memcpy(&composite[static_cast<std::size_t>(i) * 4U], &bg, 4U);
    }

    // Composite visible surfaces back-to-front (painter's algorithm).
    for (const auto& surface : surfaces) {
        if (surface == nullptr || !surface->isVisible()) {
            continue;
        }

        const int sw  = surface->width();
        const int sh  = surface->height();
        const int sx  = surface->x();
        const int sy  = surface->y();
        const uint8_t* src = surface->buffer();

        for (int row = 0; row < sh; ++row) {
            const int dstY = sy + row;
            if (dstY < 0 || dstY >= viewportH) {
                continue;
            }
            for (int col = 0; col < sw; ++col) {
                const int dstX = sx + col;
                if (dstX < 0 || dstX >= viewportW) {
                    continue;
                }
                const std::size_t srcOff =
                    (static_cast<std::size_t>(row) * static_cast<std::size_t>(sw) +
                     static_cast<std::size_t>(col)) * 4U;
                const std::size_t dstOff =
                    (static_cast<std::size_t>(dstY) * static_cast<std::size_t>(viewportW) +
                     static_cast<std::size_t>(dstX)) * 4U;
                std::memcpy(&composite[dstOff], &src[srcOff], 4U);
            }
        }
    }

    // Blit the composite frame to the platform display.
    platform::Platform* plat = platform::activePlatform();
    if (plat != nullptr) {
        plat->blit(composite.data(), viewportW, viewportH);
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
    if (focusedSurfaceIndex < 0 ||
        focusedSurfaceIndex >= static_cast<int>(surfaces.size())) {
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
