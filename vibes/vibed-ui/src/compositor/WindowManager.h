// MIT License
// Copyright (c) 2026 Palaash

#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "Surface.h"

namespace compositor {

class WindowManager {
public:
    WindowManager();
    ~WindowManager();

    void addSurface(std::shared_ptr<Surface> s);
    void renderAll();

    void focusSurfaceAt(int x, int y);
    int focusedIndex() const;
    std::shared_ptr<Surface> focusedSurface() const;

    const std::vector<std::shared_ptr<Surface>>& allSurfaces() const;

private:
    void bringToFront(std::size_t index);

    std::vector<std::shared_ptr<Surface>> surfaces;
    int focusedSurfaceIndex = -1;
};

} // namespace compositor
