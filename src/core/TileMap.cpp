#include "TileMap.h"

#include <stdexcept>

TileMap::TileMap(const std::vector<std::string>& rows) {
    if (rows.empty() || rows[0].empty()) throw std::invalid_argument("TileMap: peta kosong");
    width_ = static_cast<int>(rows[0].size());
    height_ = static_cast<int>(rows.size());
    tiles_.reserve(static_cast<std::size_t>(width_ * height_));
    for (const std::string& row : rows) {
        if (static_cast<int>(row.size()) != width_)
            throw std::invalid_argument("TileMap: panjang baris tidak sama");
        for (char c : row) tiles_.push_back(fromChar(c));
    }
}

bool TileMap::inBounds(int x, int y) const { return x >= 0 && y >= 0 && x < width_ && y < height_; }

Tile TileMap::at(int x, int y) const {
    if (!inBounds(x, y)) return Tile::Tree;
    return tiles_[static_cast<std::size_t>(y * width_ + x)];
}

bool TileMap::isWalkable(int x, int y) const {
    switch (at(x, y)) {
        case Tile::Grass:
        case Tile::Flowers:
        case Tile::Path:
        case Tile::Bridge:
        case Tile::CastleFloor:
            return true;
        default:
            return false;
    }
}

Tile TileMap::fromChar(char c) {
    switch (c) {
        case '.': return Tile::Grass;
        case ',': return Tile::Flowers;
        case '=': return Tile::Path;
        case '#': return Tile::Tree;
        case 'o': return Tile::Rock;
        case '~': return Tile::Water;
        case 'b': return Tile::Bridge;
        case 'R': return Tile::Roof;
        case 'H': return Tile::HouseWall;
        case 'C': return Tile::CastleWall;
        case 'c': return Tile::CastleFloor;
        default:  throw std::invalid_argument(std::string("TileMap: karakter tidak dikenal: ") + c);
    }
}
