#pragma once
#include "raylib.h"
#include "core/Appearance.h"
#include "core/TileMap.h"

// Semua gambar dibuat secara prosedural (bentuk dasar raylib), jadi tidak butuh file aset
// dan tidak ada masalah lisensi. Kalau nanti ingin memakai sprite PNG, cukup ganti isi
// fungsi-fungsi ini tanpa mengubah kelas lain.
namespace art {

enum class Facing { Down, Up, Left, Right };

Color toColor(Rgb c, float alpha = 1.0f);

// Karakter utuh (chibi) dengan kaki di posisi feet.
void drawCharacter(const Appearance& look, Vector2 feet, float scale, float time,
                   Facing facing, bool walking, float alpha = 1.0f);

// Potret setengah badan untuk dialog visual novel.
void drawPortrait(const Appearance& look, Rectangle area, float time, bool flip);

void drawTile(Tile tile, int tx, int ty, float tileSize, float time);
void drawSignpost(Vector2 feet, float scale);

}  // namespace art
