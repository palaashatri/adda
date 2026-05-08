// MIT License
// Copyright (c) 2026 Palaash

#include "FlexLayout.h"

namespace ui {

FlexLayout::FlexLayout() {}

FlexLayout::~FlexLayout() {}

void FlexLayout::setSpacing(int value) {
    spacing = value < 0 ? 0 : value;
}

void FlexLayout::setVertical(bool value) {
    vertical = value;
}

void FlexLayout::apply(View& parent) {
    int px = 0;
    int py = 0;
    int pw = 0;
    int ph = 0;
    parent.getFrame(px, py, pw, ph);

    const auto& childNodes = parent.children();
    if (childNodes.empty()) {
        return;
    }

    if (vertical) {
        const int count = static_cast<int>(childNodes.size());
        int sharedSize = ph - (spacing * (count - 1));
        int defaultHeight = count > 0 ? sharedSize / count : ph;
        if (defaultHeight <= 0) {
            defaultHeight = 30;
        }

        int yOffset = py;
        for (const auto& child : childNodes) {
            int cx = 0;
            int cy = 0;
            int cw = 0;
            int ch = 0;
            child->getFrame(cx, cy, cw, ch);

            const int widthToUse = cw > 0 ? cw : pw;
            const int heightToUse = ch > 0 ? ch : defaultHeight;
            child->setFrame(px, yOffset, widthToUse, heightToUse);
            yOffset += heightToUse + spacing;
        }
        return;
    }

    const int count = static_cast<int>(childNodes.size());
    int sharedSize = pw - (spacing * (count - 1));
    int defaultWidth = count > 0 ? sharedSize / count : pw;
    if (defaultWidth <= 0) {
        defaultWidth = 80;
    }

    int xOffset = px;
    for (const auto& child : childNodes) {
        int cx = 0;
        int cy = 0;
        int cw = 0;
        int ch = 0;
        child->getFrame(cx, cy, cw, ch);

        const int widthToUse = cw > 0 ? cw : defaultWidth;
        const int heightToUse = ch > 0 ? ch : ph;
        child->setFrame(xOffset, py, widthToUse, heightToUse);
        xOffset += widthToUse + spacing;
    }
}

} // namespace ui
