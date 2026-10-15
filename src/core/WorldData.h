#pragma once
#include <string>
#include <vector>

// Data dunia (peta dan posisi penting). Dipisah dari kode supaya mudah diubah.
namespace world {

struct Spot { int x; int y; };

inline const std::vector<std::string>& mapRows() {
    static const std::vector<std::string> rows = {
        "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCC",
        "#CCCCCCCCCCCCCccCCCCCCCCCCCCC#",
        "##############=###############",
        "#..,....o.....=.....,...o....#",
        "#.#....,......=....#.....,...#",
        "#....#........=.....o....#...#",
        "#..,..........=..,.......#...#",
        "#.....o.......=.....#...,....#",
        "~~~~~~~~~~~~~~b~~~~~~~~~~~~~~~",
        "~~~~~~~~~~~~~~b~~~~~~~~~~~~~~~",
        "#.....,.......=.......,......#",
        "#.RRRR........=.......RRRR...#",
        "#.HHHH...,....=..,....HHHH...#",
        "#.............=..............#",
        "#..,..........=..........,...#",
        "#.............=..............#",
        "##############################",
    };
    return rows;
}

constexpr Spot kPlayerStart{14, 14};
constexpr Spot kGuildGirl{4, 13};
constexpr Spot kInnkeeper{23, 13};
constexpr Spot kSignpost{15, 10};
constexpr Spot kGolem{14, 9};
constexpr Spot kSlimesA{6, 6};
constexpr Spot kWispPack{22, 5};
constexpr Spot kSlimesB{4, 3};
constexpr Spot kWitch{14, 2};

}  // namespace world
