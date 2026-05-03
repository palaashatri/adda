// MIT License
// Copyright (c) 2026 Palaash

#include "Theme.h"

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace {

bool extractQuotedValue(const std::string& json, const std::string& key, std::string& value) {
    const std::string token = "\"" + key + "\"";
    const std::size_t keyPos = json.find(token);
    if (keyPos == std::string::npos) {
        return false;
    }

    const std::size_t colonPos = json.find(':', keyPos + token.size());
    if (colonPos == std::string::npos) {
        return false;
    }

    const std::size_t beginQuote = json.find('"', colonPos + 1U);
    if (beginQuote == std::string::npos) {
        return false;
    }

    const std::size_t endQuote = json.find('"', beginQuote + 1U);
    if (endQuote == std::string::npos || endQuote <= beginQuote) {
        return false;
    }

    value = json.substr(beginQuote + 1U, endQuote - beginQuote - 1U);
    return true;
}

bool extractIntegerValue(const std::string& json, const std::string& key, int& value) {
    const std::string token = "\"" + key + "\"";
    const std::size_t keyPos = json.find(token);
    if (keyPos == std::string::npos) {
        return false;
    }

    const std::size_t colonPos = json.find(':', keyPos + token.size());
    if (colonPos == std::string::npos) {
        return false;
    }

    std::size_t numberBegin = colonPos + 1U;
    while (numberBegin < json.size() && std::isspace(static_cast<unsigned char>(json[numberBegin])) != 0) {
        ++numberBegin;
    }
    if (numberBegin >= json.size()) {
        return false;
    }

    std::size_t numberEnd = numberBegin;
    if (json[numberEnd] == '-') {
        ++numberEnd;
    }
    while (numberEnd < json.size() && std::isdigit(static_cast<unsigned char>(json[numberEnd])) != 0) {
        ++numberEnd;
    }
    if (numberEnd <= numberBegin) {
        return false;
    }

    const std::string numberText = json.substr(numberBegin, numberEnd - numberBegin);
    value = std::atoi(numberText.c_str());
    return true;
}

int hexDigitToInt(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return 10 + (c - 'a');
    }
    if (c >= 'A' && c <= 'F') {
        return 10 + (c - 'A');
    }
    return 0;
}

uint8_t parseHexByte(const std::string& value, std::size_t offset) {
    if (offset + 1U >= value.size()) {
        return 0;
    }
    const int high = hexDigitToInt(value[offset]);
    const int low = hexDigitToInt(value[offset + 1U]);
    return static_cast<uint8_t>((high << 4) | low);
}

ui::Color parseHexColor(const std::string& text) {
    std::string value = text;
    if (!value.empty() && value[0] == '#') {
        value = value.substr(1U);
    }

    if (value.size() == 6U) {
        return ui::Color(parseHexByte(value, 0U), parseHexByte(value, 2U), parseHexByte(value, 4U), 255);
    }
    if (value.size() == 8U) {
        return ui::Color(parseHexByte(value, 0U), parseHexByte(value, 2U), parseHexByte(value, 4U), parseHexByte(value, 6U));
    }

    return ui::Color();
}

} // namespace

namespace ui {

Theme::Theme()
    : fallbackColor(255, 255, 255, 255), fallbackFont("Sans", 12), fallbackRadius(4) {
    colors["background"] = Color(34, 34, 34, 255);
    colors["text"] = Color(255, 255, 255, 255);
    radii["default"] = 4;
    fonts["default"] = Font("Sans", 12);
}

Theme::~Theme() {}

bool Theme::loadFromJson(const std::string& path) {
    std::ifstream input(path);
    if (!input.is_open()) {
        return false;
    }

    std::ostringstream stream;
    stream << input.rdbuf();
    const std::string json = stream.str();

    std::string colorValue;
    bool anyLoaded = false;
    if (extractQuotedValue(json, "background", colorValue)) {
        colors["background"] = parseHexColor(colorValue);
        anyLoaded = true;
    }
    if (extractQuotedValue(json, "text", colorValue)) {
        colors["text"] = parseHexColor(colorValue);
        anyLoaded = true;
    }
    if (extractQuotedValue(json, "primary", colorValue)) {
        colors["primary"] = parseHexColor(colorValue);
        anyLoaded = true;
    }

    int numberValue = 0;
    if (extractIntegerValue(json, "cornerRadius", numberValue)) {
        radii["default"] = numberValue;
        anyLoaded = true;
    }

    std::string fontName;
    if (extractQuotedValue(json, "fontName", fontName)) {
        int fontSize = fonts["default"].size();
        if (extractIntegerValue(json, "fontSize", fontSize)) {
            fonts["default"] = Font(fontName, fontSize);
        } else {
            fonts["default"] = Font(fontName, 12);
        }
        anyLoaded = true;
    }

    return anyLoaded;
}

const Color& Theme::color(const std::string& name) const {
    const auto it = colors.find(name);
    if (it != colors.end()) {
        return it->second;
    }
    return fallbackColor;
}

int Theme::radius(const std::string& name) const {
    const auto it = radii.find(name);
    if (it != radii.end()) {
        return it->second;
    }
    return fallbackRadius;
}

const Font& Theme::font(const std::string& name) const {
    const auto it = fonts.find(name);
    if (it != fonts.end()) {
        return it->second;
    }
    return fallbackFont;
}

} // namespace ui