// MIT License
// Copyright (c) 2026 Palaash

#pragma once

namespace compositor {

class Surface {
public:
    Surface(int w, int h);
    ~Surface();

    int width() const;
    int height() const;

    void setPosition(int x, int y);
    int x() const;
    int y() const;

    void setVisible(bool value);
    bool isVisible() const;
    bool containsPoint(int px, int py) const;

private:
    int w;
    int h;
    int posX = 0;
    int posY = 0;
    bool visible = true;
};

} // namespace compositor
