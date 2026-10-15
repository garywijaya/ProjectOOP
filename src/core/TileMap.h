#pragma once
#include <string>
#include <vector>

enum class Tile { Grass, Flowers, Path, Tree, Rock, Water, Bridge, Roof, HouseWall, CastleWall, CastleFloor };

// ENCAPSULATION: grid peta disimpan private dan hanya bisa dibaca.
// at() aman dipanggil dengan koordinat di luar peta (dianggap pohon / tidak bisa dilewati),
// sehingga kode pemain tidak perlu mengecek batas array sendiri.
class TileMap {
public:
    // Legenda: '.' rumput, ',' bunga, '=' jalan, '#' pohon, 'o' batu, '~' air, 'b' jembatan,
    //          'R' atap, 'H' dinding rumah, 'C' dinding kastil, 'c' lantai kastil.
    explicit TileMap(const std::vector<std::string>& rows);

    int width() const { return width_; }
    int height() const { return height_; }
    bool inBounds(int x, int y) const;
    Tile at(int x, int y) const;
    bool isWalkable(int x, int y) const;

private:
    static Tile fromChar(char c);

    int width_ = 0;
    int height_ = 0;
    std::vector<Tile> tiles_;
};
