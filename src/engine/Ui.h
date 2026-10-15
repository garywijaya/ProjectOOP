#pragma once
#include <string>

#include "raylib.h"

// Komponen UI kecil yang dipakai ulang oleh semua scene.
namespace ui {
inline constexpr Color kPanel{18, 20, 42, 235};
inline constexpr Color kPanelLight{40, 44, 82, 240};
inline constexpr Color kBorder{232, 205, 150, 255};
inline constexpr Color kText{245, 242, 235, 255};
inline constexpr Color kMuted{160, 165, 190, 255};
inline constexpr Color kAccent{255, 205, 90, 255};
inline constexpr Color kHp{90, 210, 120, 255};
inline constexpr Color kHpLow{235, 80, 80, 255};
inline constexpr Color kMp{90, 150, 255, 255};

void panel(Rectangle r);
void text(const std::string& s, float x, float y, int size, Color color);
void textShadow(const std::string& s, float x, float y, int size, Color color);
void textCentered(const std::string& s, float centerX, float y, int size, Color color);
int textWidth(const std::string& s, int size);
// Menggambar teks dengan pemenggalan kata otomatis. maxChars untuk efek ketik (typewriter).
void textWrapped(const std::string& s, Rectangle area, int size, Color color, std::size_t maxChars = std::string::npos);
void bar(Rectangle r, float ratio, Color fill);
void cursor(float x, float y, float time);
}  // namespace ui
