#include "Ui.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <vector>

namespace ui {

void panel(Rectangle r) {
    DrawRectangleRounded(r, 0.08f, 8, kPanel);
    DrawRectangleRoundedLinesEx(r, 0.08f, 8, 2.0f, kBorder);
}

void text(const std::string& s, float x, float y, int size, Color color) {
    DrawText(s.c_str(), static_cast<int>(x), static_cast<int>(y), size, color);
}

void textShadow(const std::string& s, float x, float y, int size, Color color) {
    DrawText(s.c_str(), static_cast<int>(x) + 2, static_cast<int>(y) + 2, size, {0, 0, 0, 160});
    DrawText(s.c_str(), static_cast<int>(x), static_cast<int>(y), size, color);
}

void textCentered(const std::string& s, float centerX, float y, int size, Color color) {
    textShadow(s, centerX - static_cast<float>(textWidth(s, size)) / 2.0f, y, size, color);
}

int textWidth(const std::string& s, int size) { return MeasureText(s.c_str(), size); }

void textWrapped(const std::string& s, Rectangle area, int size, Color color, std::size_t maxChars) {
    std::istringstream words(s);
    std::string word, line;
    std::vector<std::string> lines;
    while (words >> word) {
        const std::string candidate = line.empty() ? word : line + " " + word;
        if (textWidth(candidate, size) > static_cast<int>(area.width) && !line.empty()) {
            lines.push_back(line);
            line = word;
        } else {
            line = candidate;
        }
    }
    if (!line.empty()) lines.push_back(line);

    std::size_t remaining = maxChars;
    float y = area.y;
    for (const std::string& l : lines) {
        if (remaining == 0) break;
        const std::string shown = l.substr(0, std::min(remaining, l.size()));
        text(shown, area.x, y, size, color);
        remaining = remaining == std::string::npos ? remaining
                                                   : remaining - std::min(remaining, l.size() + 1);
        y += static_cast<float>(size) * 1.35f;
        if (y > area.y + area.height) break;
    }
}

void bar(Rectangle r, float ratio, Color fill) {
    ratio = std::clamp(ratio, 0.0f, 1.0f);
    DrawRectangleRounded(r, 0.5f, 6, {10, 10, 20, 220});
    if (ratio > 0.0f) DrawRectangleRounded({r.x, r.y, r.width * ratio, r.height}, 0.5f, 6, fill);
    DrawRectangleRoundedLinesEx(r, 0.5f, 6, 1.0f, {255, 255, 255, 70});
}

void cursor(float x, float y, float time) {
    const float off = std::sin(time * 8.0f) * 3.0f;
    DrawTriangle({x + off, y - 7}, {x + off, y + 7}, {x + 10 + off, y}, kAccent);
}

}  // namespace ui
