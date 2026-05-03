// MIT License
// Copyright (c) 2026 Palaash

#include "Surface.h"

namespace compositor {

Surface::Surface(int width, int height)
    : w(width), h(height) {}

Surface::~Surface() {}

int Surface::width() const {
    return w;
}

int Surface::height() const {
    return h;
}

void Surface::setPosition(int xPos, int yPos) {
    posX = xPos;
    posY = yPos;
}

int Surface::x() const {
    return posX;
}

int Surface::y() const {
    return posY;
}

void Surface::setVisible(bool value) {
    visible = value;
}

bool Surface::isVisible() const {
    return visible;
}

bool Surface::containsPoint(int px, int py) const {
    return px >= posX && py >= posY && px < (posX + w) && py < (posY + h);
}

} // namespace compositor
