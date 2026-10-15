#pragma once
#include <cstdint>

// Data tampilan murni (tanpa raylib) supaya core logic tetap bisa dites tanpa grafis.
struct Rgb { std::uint8_t r = 255, g = 255, b = 255; };

enum class BodyType { Heroine, Slime, Wisp, Golem };
enum class HairStyle { Long, Ponytail, Twintails, Short };
enum class Accessory { None, Ribbon, Tiara, WitchHat };

struct Appearance {
    BodyType body = BodyType::Heroine;
    Rgb hair{};
    Rgb outfit{};
    Rgb eyes{};
    HairStyle hairStyle = HairStyle::Long;
    Accessory accessory = Accessory::None;
};
