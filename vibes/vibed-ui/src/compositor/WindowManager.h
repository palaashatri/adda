// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "Surface.h"

namespace compositor {

class WindowManager {
public:
    WindowManager();
    ~WindowManager();

    void addSurface(std::shared_ptr<Surface> s);

    // Set the viewport dimensions before calling renderAll. Must be called
    // once after compositor initialisation and again on window resize.
    void setViewport(int w, int h);

    // Composite all visible surfaces (back-to-front) into the output buffer
    // and blit the result to the active platform.
    void renderAll();

    void focusSurfaceAt(int x, int y);
    int focusedIndex() const;
    std::shared_ptr<Surface> focusedSurface() const;

    const std::vector<std::shared_ptr<Surface>>& allSurfaces() const;

private:
    void bringToFront(std::size_t index);

    std::vector<std::shared_ptr<Surface>> surfaces;
    int focusedSurfaceIndex = -1;

    int viewportW = 0;
    int viewportH = 0;
    std::vector<uint8_t> composite;
};

} // namespace compositor
