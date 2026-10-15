#include "Art.h"

#include <cmath>

namespace art {
namespace {

constexpr Rgb kSkin{255, 228, 210};
constexpr Rgb kSkinShade{240, 196, 178};
constexpr Rgb kLine{50, 36, 52};

unsigned hash2(int x, int y) {
    unsigned h = static_cast<unsigned>(x) * 374761393u + static_cast<unsigned>(y) * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

Color shade(Rgb c, float factor, float alpha) {
    auto ch = [&](std::uint8_t v) {
        const float f = static_cast<float>(v) * factor;
        return static_cast<unsigned char>(f > 255.f ? 255.f : f);
    };
    return {ch(c.r), ch(c.g), ch(c.b), static_cast<unsigned char>(255.f * alpha)};
}

Color withAlpha(Color c, float alpha) {
    c.a = static_cast<unsigned char>(static_cast<float>(c.a) * alpha);
    return c;
}

// Menggambar segitiga tanpa peduli urutan titik (satu dari dua akan terbuang oleh culling).
void tri(Vector2 a, Vector2 b, Vector2 c, Color color) {
    DrawTriangle(a, b, c, color);
    DrawTriangle(a, c, b, color);
}

void quad(Vector2 a, Vector2 b, Vector2 c, Vector2 d, Color color) {
    tri(a, b, c, color);
    tri(a, c, d, color);
}

float faceShift(Facing f) {
    if (f == Facing::Left) return -1.0f;
    if (f == Facing::Right) return 1.0f;
    return 0.0f;
}

void drawBackHair(const Appearance& look, Vector2 c, float R, float bottomY, Facing facing, float alpha) {
    const Color hair = toColor(look.hair, alpha);
    const Color hairDark = shade(look.hair, 0.8f, alpha);
    switch (look.hairStyle) {
        case HairStyle::Long:
            DrawRectangleRounded({c.x - R * 1.05f, c.y, R * 2.1f, bottomY - c.y}, 0.45f, 8, hairDark);
            break;
        case HairStyle::Ponytail: {
            const float side = facing == Facing::Left ? 1.0f : -1.0f;
            const float x = facing == Facing::Down || facing == Facing::Up ? c.x : c.x + side * R * 0.9f;
            DrawEllipse(static_cast<int>(x), static_cast<int>(c.y + R * 0.9f), R * 0.55f, R * 1.25f, hairDark);
            DrawCircleV({x, c.y - R * 0.55f}, R * 0.28f, toColor({240, 240, 255}, alpha));   // ikat rambut
            break;
        }
        case HairStyle::Twintails:
            DrawEllipse(static_cast<int>(c.x - R * 1.15f), static_cast<int>(c.y + R * 0.75f), R * 0.42f, R * 1.0f, hair);
            DrawEllipse(static_cast<int>(c.x + R * 1.15f), static_cast<int>(c.y + R * 0.75f), R * 0.42f, R * 1.0f, hair);
            break;
        case HairStyle::Short:
            break;
    }
}

void drawEye(Vector2 p, float R, const Appearance& look, float alpha, bool blink) {
    if (blink) {
        DrawLineEx({p.x - R * 0.24f, p.y}, {p.x + R * 0.24f, p.y}, R * 0.08f, toColor(kLine, alpha));
        return;
    }
    DrawEllipse(static_cast<int>(p.x), static_cast<int>(p.y), R * 0.25f, R * 0.33f, toColor(kLine, alpha));
    DrawEllipse(static_cast<int>(p.x), static_cast<int>(p.y + R * 0.03f), R * 0.2f, R * 0.27f, toColor(look.eyes, alpha));
    DrawEllipse(static_cast<int>(p.x), static_cast<int>(p.y + R * 0.12f), R * 0.15f, R * 0.12f, shade(look.eyes, 1.35f, alpha));
    DrawCircleV({p.x, p.y + R * 0.02f}, R * 0.08f, toColor(kLine, alpha));
    DrawCircleV({p.x - R * 0.08f, p.y - R * 0.12f}, R * 0.08f, withAlpha(WHITE, alpha));
}

void drawHead(const Appearance& look, Vector2 c, float R, Facing facing, float time, float alpha) {
    const Color hair = toColor(look.hair, alpha);
    const Color hairLight = shade(look.hair, 1.18f, alpha);
    DrawCircleV(c, R, toColor(kSkin, alpha));

    if (facing == Facing::Up) {
        DrawCircleV({c.x, c.y - R * 0.05f}, R * 1.06f, hair);
    } else {
        const float sx = faceShift(facing) * R * 0.28f;
        // Mata, pipi, mulut
        const bool blink = std::fmod(time + c.x * 0.013f, 4.2f) < 0.12f;
        if (facing == Facing::Down) {
            drawEye({c.x - R * 0.4f, c.y + R * 0.22f}, R, look, alpha, blink);
            drawEye({c.x + R * 0.4f, c.y + R * 0.22f}, R, look, alpha, blink);
        } else {
            drawEye({c.x + sx + R * 0.18f * faceShift(facing), c.y + R * 0.22f}, R, look, alpha, blink);
            drawEye({c.x + sx - R * 0.42f * faceShift(facing), c.y + R * 0.22f}, R * 0.8f, look, alpha, blink);
        }
        const Color blush{255, 140, 150, static_cast<unsigned char>(110 * alpha)};
        DrawEllipse(static_cast<int>(c.x - R * 0.62f + sx), static_cast<int>(c.y + R * 0.58f), R * 0.17f, R * 0.08f, blush);
        DrawEllipse(static_cast<int>(c.x + R * 0.62f + sx), static_cast<int>(c.y + R * 0.58f), R * 0.17f, R * 0.08f, blush);
        DrawEllipse(static_cast<int>(c.x + sx), static_cast<int>(c.y + R * 0.66f), R * 0.09f, R * 0.05f, toColor({200, 90, 100}, alpha));

        // Rambut atas + poni
        DrawCircleSector(c, R * 1.08f, 180.0f, 360.0f, 24, hair);
        const float top = c.y - R * 0.2f;
        const float tip = c.y + R * 0.18f;
        for (int i = -2; i <= 2; ++i) {
            const float x = c.x + sx * 0.5f + static_cast<float>(i) * R * 0.38f;
            tri({x - R * 0.26f, top}, {x + R * 0.26f, top}, {x + R * 0.04f, tip + (i == 0 ? R * 0.08f : 0.0f)}, hair);
        }
        DrawRectangleRounded({c.x - R * 1.08f, c.y - R * 0.25f, R * 0.32f, R * 1.0f}, 0.6f, 6, hair);
        DrawRectangleRounded({c.x + R * 0.76f, c.y - R * 0.25f, R * 0.32f, R * 1.0f}, 0.6f, 6, hair);
        DrawCircleSector({c.x - R * 0.35f, c.y - R * 0.62f}, R * 0.3f, 200.0f, 300.0f, 8, hairLight);
    }

    // Aksesori
    switch (look.accessory) {
        case Accessory::Ribbon: {
            const Vector2 b{c.x + R * 0.7f, c.y - R * 0.85f};
            const Color rib = toColor({255, 90, 120}, alpha);
            tri(b, {b.x - R * 0.5f, b.y - R * 0.3f}, {b.x - R * 0.45f, b.y + R * 0.3f}, rib);
            tri(b, {b.x + R * 0.5f, b.y - R * 0.3f}, {b.x + R * 0.45f, b.y + R * 0.3f}, rib);
            DrawCircleV(b, R * 0.13f, shade({255, 90, 120}, 0.8f, alpha));
            break;
        }
        case Accessory::Tiara: {
            const Color gold = toColor({255, 215, 90}, alpha);
            const float y = c.y - R * 0.92f;
            tri({c.x - R * 0.4f, y + R * 0.1f}, {c.x + R * 0.4f, y + R * 0.1f}, {c.x, y - R * 0.38f}, gold);
            DrawCircleV({c.x, y - R * 0.08f}, R * 0.1f, toColor({120, 220, 255}, alpha));
            break;
        }
        case Accessory::WitchHat: {
            const Color hat = toColor({40, 28, 62}, alpha);
            const float brimY = c.y - R * 0.72f;
            DrawEllipse(static_cast<int>(c.x), static_cast<int>(brimY), R * 1.65f, R * 0.38f, hat);
            tri({c.x - R * 0.85f, brimY}, {c.x + R * 0.85f, brimY}, {c.x + R * 0.75f, brimY - R * 2.0f}, hat);
            DrawRectangleV({c.x - R * 0.8f, brimY - R * 0.32f}, {R * 1.6f, R * 0.22f}, toColor({200, 60, 110}, alpha));
            break;
        }
        case Accessory::None:
            break;
    }
}

void drawHeroine(const Appearance& look, Vector2 feet, float s, float time, Facing facing,
                 bool walking, float alpha) {
    const float bob = walking ? std::fabs(std::sin(time * 12.0f)) * -1.6f * s : std::sin(time * 2.4f) * 0.6f * s;
    const float x = feet.x;
    const float y = feet.y;
    const float R = 10.5f * s;
    const Vector2 head{x, y - 28.0f * s + bob};

    DrawEllipse(static_cast<int>(x), static_cast<int>(y), 10.0f * s, 3.5f * s, {0, 0, 0, static_cast<unsigned char>(60 * alpha)});
    drawBackHair(look, head, R, y - 8.0f * s, facing, alpha);

    // Kaki
    const float step = walking ? std::sin(time * 12.0f) * 1.8f * s : 0.0f;
    const Color leg = toColor({60, 50, 72}, alpha);
    DrawRectangleRounded({x - 4.5f * s, y - 7.0f * s + step, 3.2f * s, 7.0f * s - step}, 0.4f, 4, leg);
    DrawRectangleRounded({x + 1.3f * s, y - 7.0f * s - step, 3.2f * s, 7.0f * s + step}, 0.4f, 4, leg);

    // Baju (trapesium) + garis tepi
    const Color outfit = toColor(look.outfit, alpha);
    const float top = y - 19.0f * s + bob * 0.5f;
    const float bottom = y - 5.0f * s;
    quad({x - 5.5f * s, top}, {x + 5.5f * s, top}, {x + 10.0f * s, bottom}, {x - 10.0f * s, bottom}, outfit);
    DrawRectangleV({x - 10.0f * s, bottom - 1.6f * s}, {20.0f * s, 1.6f * s}, shade(look.outfit, 1.5f, alpha));
    DrawCircleV({x, top + 2.0f * s}, 2.0f * s, toColor(look.hair, alpha));   // pita dada
    // Lengan
    DrawCircleV({x - 7.2f * s, y - 13.0f * s + bob * 0.5f}, 2.4f * s, toColor(kSkin, alpha));
    DrawCircleV({x + 7.2f * s, y - 13.0f * s + bob * 0.5f}, 2.4f * s, toColor(kSkin, alpha));

    drawHead(look, head, R, facing, time, alpha);
}

void drawSlime(const Appearance& look, Vector2 feet, float s, float time, float alpha) {
    const float squish = std::sin(time * 4.0f) * 0.08f;
    const float rx = 13.0f * s * (1.0f + squish);
    const float ry = 10.0f * s * (1.0f - squish);
    DrawEllipse(static_cast<int>(feet.x), static_cast<int>(feet.y), 12.0f * s, 3.5f * s, {0, 0, 0, static_cast<unsigned char>(60 * alpha)});
    DrawEllipse(static_cast<int>(feet.x), static_cast<int>(feet.y - ry), rx, ry, toColor(look.hair, alpha * 0.92f));
    DrawEllipse(static_cast<int>(feet.x), static_cast<int>(feet.y - ry * 0.6f), rx * 0.8f, ry * 0.45f, shade(look.outfit, 1.0f, alpha * 0.5f));
    DrawEllipse(static_cast<int>(feet.x - rx * 0.4f), static_cast<int>(feet.y - ry * 1.45f), rx * 0.22f, ry * 0.16f, withAlpha(WHITE, alpha * 0.8f));
    DrawEllipse(static_cast<int>(feet.x - 4.0f * s), static_cast<int>(feet.y - ry * 1.05f), 1.6f * s, 2.4f * s, toColor(look.eyes, alpha));
    DrawEllipse(static_cast<int>(feet.x + 4.0f * s), static_cast<int>(feet.y - ry * 1.05f), 1.6f * s, 2.4f * s, toColor(look.eyes, alpha));
}

void drawWisp(const Appearance& look, Vector2 feet, float s, float time, float alpha) {
    const float hover = std::sin(time * 3.0f) * 2.5f * s;
    const Vector2 c{feet.x, feet.y - 16.0f * s + hover};
    const float flick = 1.0f + std::sin(time * 17.0f) * 0.06f;
    DrawEllipse(static_cast<int>(feet.x), static_cast<int>(feet.y), 8.0f * s, 2.5f * s, {0, 0, 0, static_cast<unsigned char>(50 * alpha)});
    DrawCircleV(c, 15.0f * s * flick, withAlpha({255, 120, 40, 60}, alpha));
    tri({c.x - 9.0f * s, c.y - 2.0f * s}, {c.x + 9.0f * s, c.y - 2.0f * s}, {c.x + std::sin(time * 6.0f) * 3.0f * s, c.y - 20.0f * s * flick}, toColor(look.hair, alpha));
    DrawCircleV(c, 10.0f * s * flick, toColor(look.hair, alpha));
    DrawCircleV({c.x, c.y + 1.0f * s}, 6.5f * s, toColor(look.outfit, alpha));
    DrawEllipse(static_cast<int>(c.x - 3.0f * s), static_cast<int>(c.y), 1.4f * s, 2.2f * s, toColor(look.eyes, alpha));
    DrawEllipse(static_cast<int>(c.x + 3.0f * s), static_cast<int>(c.y), 1.4f * s, 2.2f * s, toColor(look.eyes, alpha));
}

void drawGolem(const Appearance& look, Vector2 feet, float s, float time, float alpha) {
    const float sway = std::sin(time * 1.6f) * 1.0f * s;
    const Color body = toColor(look.hair, alpha);
    const Color dark = toColor(look.outfit, alpha);
    const float x = feet.x;
    const float y = feet.y;
    DrawEllipse(static_cast<int>(x), static_cast<int>(y), 20.0f * s, 5.0f * s, {0, 0, 0, static_cast<unsigned char>(70 * alpha)});
    DrawRectangleRounded({x - 13.0f * s, y - 12.0f * s, 9.0f * s, 12.0f * s}, 0.3f, 4, dark);
    DrawRectangleRounded({x + 4.0f * s, y - 12.0f * s, 9.0f * s, 12.0f * s}, 0.3f, 4, dark);
    DrawRectangleRounded({x - 17.0f * s, y - 38.0f * s + sway, 34.0f * s, 28.0f * s}, 0.25f, 6, body);
    DrawRectangleRounded({x - 25.0f * s, y - 36.0f * s - sway, 9.0f * s, 24.0f * s}, 0.4f, 6, dark);
    DrawRectangleRounded({x + 16.0f * s, y - 36.0f * s + sway, 9.0f * s, 24.0f * s}, 0.4f, 6, dark);
    DrawRectangleRounded({x - 9.0f * s, y - 50.0f * s + sway, 18.0f * s, 14.0f * s}, 0.3f, 6, body);
    const Color glow = toColor(look.eyes, alpha);
    DrawRectangleV({x - 6.0f * s, y - 45.0f * s + sway}, {4.0f * s, 2.5f * s}, glow);
    DrawRectangleV({x + 2.0f * s, y - 45.0f * s + sway}, {4.0f * s, 2.5f * s}, glow);
    const Color crystal = withAlpha({220, 245, 255, 230}, alpha);
    tri({x - 18.0f * s, y - 37.0f * s}, {x - 10.0f * s, y - 37.0f * s}, {x - 15.0f * s, y - 50.0f * s}, crystal);
    tri({x + 10.0f * s, y - 37.0f * s}, {x + 18.0f * s, y - 37.0f * s}, {x + 15.0f * s, y - 53.0f * s}, crystal);
    DrawCircleV({x, y - 25.0f * s + sway}, 3.5f * s, glow);
}

}  // namespace

Color toColor(Rgb c, float alpha) {
    return {c.r, c.g, c.b, static_cast<unsigned char>(255.0f * alpha)};
}

void drawCharacter(const Appearance& look, Vector2 feet, float scale, float time, Facing facing,
                   bool walking, float alpha) {
    switch (look.body) {
        case BodyType::Heroine: drawHeroine(look, feet, scale, time, facing, walking, alpha); break;
        case BodyType::Slime:   drawSlime(look, feet, scale, time, alpha); break;
        case BodyType::Wisp:    drawWisp(look, feet, scale, time, alpha); break;
        case BodyType::Golem:   drawGolem(look, feet, scale, time, alpha); break;
    }
}

void drawPortrait(const Appearance& look, Rectangle area, float time, bool flip) {
    BeginScissorMode(static_cast<int>(area.x), static_cast<int>(area.y), static_cast<int>(area.width),
                     static_cast<int>(area.height));
    if (look.body != BodyType::Heroine) {
        drawCharacter(look, {area.x + area.width / 2, area.y + area.height * 0.95f}, area.width / 55.0f, time,
                      Facing::Down, false);
        EndScissorMode();
        return;
    }
    const float R = area.width * 0.27f;
    const float breathe = std::sin(time * 2.0f) * R * 0.015f;
    const Vector2 head{area.x + area.width / 2, area.y + area.height * 0.42f + breathe};
    const Facing facing = flip ? Facing::Left : Facing::Right;

    drawBackHair(look, head, R, area.y + area.height, facing, 1.0f);
    // Bahu dan leher
    DrawEllipse(static_cast<int>(head.x), static_cast<int>(area.y + area.height + R * 0.25f), R * 1.6f, R * 1.15f, toColor(look.outfit));
    DrawEllipse(static_cast<int>(head.x), static_cast<int>(area.y + area.height - R * 0.55f), R * 0.75f, R * 0.25f, shade(look.outfit, 1.5f, 1.0f));
    DrawRectangleV({head.x - R * 0.25f, head.y + R * 0.7f}, {R * 0.5f, R * 0.7f}, toColor(kSkinShade));
    drawHead(look, head, R, Facing::Down, time, 1.0f);
    EndScissorMode();
}

void drawTile(Tile tile, int tx, int ty, float T, float time) {
    const float px = static_cast<float>(tx) * T;
    const float py = static_cast<float>(ty) * T;
    const unsigned h = hash2(tx, ty);
    const Color grass{110, 190, 92, 255};
    const Color grassDark{88, 164, 76, 255};

    auto drawGrass = [&]() {
        DrawRectangleV({px, py}, {T, T}, grass);
        for (int i = 0; i < 3; ++i) {
            const float gx = px + static_cast<float>((h >> (i * 5)) % 26) + 3;
            const float gy = py + static_cast<float>((h >> (i * 7 + 3)) % 24) + 5;
            DrawLineEx({gx, gy}, {gx - 2, gy - 5}, 1.5f, grassDark);
            DrawLineEx({gx, gy}, {gx + 2, gy - 5}, 1.5f, grassDark);
        }
    };

    switch (tile) {
        case Tile::Grass:
            drawGrass();
            break;
        case Tile::Flowers: {
            drawGrass();
            const Color petals[3] = {{255, 240, 120, 255}, {255, 150, 190, 255}, {250, 250, 255, 255}};
            for (int i = 0; i < 4; ++i) {
                const float fx = px + 5 + static_cast<float>((h >> (i * 4)) % 22);
                const float fy = py + 5 + static_cast<float>((h >> (i * 6 + 2)) % 22);
                DrawCircleV({fx, fy}, 2.6f, petals[(h >> i) % 3]);
                DrawCircleV({fx, fy}, 1.0f, {230, 140, 60, 255});
            }
            break;
        }
        case Tile::Path:
            DrawRectangleV({px, py}, {T, T}, {222, 196, 142, 255});
            DrawCircleV({px + 6 + static_cast<float>(h % 20), py + 8 + static_cast<float>((h >> 8) % 16)}, 1.8f, {190, 165, 115, 255});
            DrawCircleV({px + 8 + static_cast<float>((h >> 4) % 18), py + 6 + static_cast<float>((h >> 12) % 20)}, 1.3f, {200, 175, 125, 255});
            break;
        case Tile::Tree:
            drawGrass();
            DrawRectangleV({px + T * 0.42f, py + T * 0.55f}, {T * 0.16f, T * 0.4f}, {110, 72, 46, 255});
            DrawCircleV({px + T * 0.5f, py + T * 0.42f}, T * 0.42f, {46, 120, 66, 255});
            DrawCircleV({px + T * 0.38f, py + T * 0.34f}, T * 0.24f, {64, 148, 82, 255});
            DrawCircleV({px + T * 0.62f, py + T * 0.3f}, T * 0.16f, {82, 168, 96, 255});
            break;
        case Tile::Rock:
            drawGrass();
            DrawEllipse(static_cast<int>(px + T / 2), static_cast<int>(py + T * 0.62f), T * 0.38f, T * 0.28f, {130, 134, 146, 255});
            DrawEllipse(static_cast<int>(px + T * 0.42f), static_cast<int>(py + T * 0.52f), T * 0.16f, T * 0.1f, {175, 178, 190, 255});
            break;
        case Tile::Water:
        case Tile::Bridge: {
            DrawRectangleV({px, py}, {T, T}, {66, 142, 222, 255});
            const float w = std::sin(time * 2.0f + static_cast<float>(tx) * 0.9f + static_cast<float>(ty)) * 3.0f;
            DrawLineEx({px + 4, py + 10 + w}, {px + 14, py + 10 + w}, 2.0f, {150, 205, 250, 200});
            DrawLineEx({px + 16, py + 22 - w}, {px + 27, py + 22 - w}, 2.0f, {150, 205, 250, 200});
            if (tile == Tile::Bridge) {
                DrawRectangleV({px + 2, py}, {T - 4, T}, {160, 110, 68, 255});
                for (int i = 0; i < 4; ++i)
                    DrawLineEx({px + 2, py + static_cast<float>(i) * 8 + 1}, {px + T - 2, py + static_cast<float>(i) * 8 + 1}, 1.5f, {112, 74, 44, 255});
                DrawRectangleV({px, py}, {3, T}, {112, 74, 44, 255});
                DrawRectangleV({px + T - 3, py}, {3, T}, {112, 74, 44, 255});
            }
            break;
        }
        case Tile::Roof:
            DrawRectangleV({px, py}, {T, T}, {196, 74, 64, 255});
            for (int i = 0; i < 4; ++i)
                DrawLineEx({px, py + static_cast<float>(i) * 8 + 6}, {px + T, py + static_cast<float>(i) * 8 + 6}, 1.5f, {150, 50, 46, 255});
            break;
        case Tile::HouseWall:
            DrawRectangleV({px, py}, {T, T}, {244, 226, 190, 255});
            DrawRectangleLinesEx({px, py, T, T}, 1.0f, {200, 175, 140, 255});
            if (h % 3 == 0) {
                DrawRectangleV({px + 10, py + 12}, {12, 20}, {120, 80, 52, 255});   // pintu
            } else {
                DrawRectangleV({px + 8, py + 8}, {16, 12}, {140, 200, 240, 255});    // jendela
                DrawLineEx({px + 16, py + 8}, {px + 16, py + 20}, 1.5f, {244, 226, 190, 255});
            }
            break;
        case Tile::CastleWall:
            DrawRectangleV({px, py}, {T, T}, {66, 60, 92, 255});
            DrawRectangleLinesEx({px + 1, py + 1, T / 2 - 1, T / 2 - 1}, 1.0f, {46, 40, 70, 255});
            DrawRectangleLinesEx({px + T / 2, py + 1, T / 2 - 1, T / 2 - 1}, 1.0f, {46, 40, 70, 255});
            DrawRectangleLinesEx({px + T / 4, py + T / 2, T / 2, T / 2 - 1}, 1.0f, {46, 40, 70, 255});
            if (ty == 0 && tx % 2 == 0) DrawRectangleV({px + 6, py + 4}, {T - 12, 6}, {150, 80, 200, 120});
            break;
        case Tile::CastleFloor:
            DrawRectangleV({px, py}, {T, T}, {104, 92, 128, 255});
            DrawRectangleLinesEx({px, py, T, T}, 1.0f, {80, 70, 104, 255});
            break;
    }
}

void drawSignpost(Vector2 feet, float s) {
    DrawRectangleV({feet.x - 2 * s, feet.y - 18 * s}, {4 * s, 18 * s}, {120, 80, 50, 255});
    DrawRectangleRounded({feet.x - 11 * s, feet.y - 24 * s, 22 * s, 11 * s}, 0.2f, 4, {170, 120, 72, 255});
    DrawRectangleRoundedLinesEx({feet.x - 11 * s, feet.y - 24 * s, 22 * s, 11 * s}, 0.2f, 4, 1.0f, {100, 66, 40, 255});
    DrawLineEx({feet.x - 7 * s, feet.y - 20 * s}, {feet.x + 7 * s, feet.y - 20 * s}, 1.0f, {90, 60, 36, 255});
    DrawLineEx({feet.x - 7 * s, feet.y - 17 * s}, {feet.x + 4 * s, feet.y - 17 * s}, 1.0f, {90, 60, 36, 255});
}

}  // namespace art
