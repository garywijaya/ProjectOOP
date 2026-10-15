#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "Art.h"
#include "WorldServices.h"
#include "core/Dialogue.h"
#include "core/TileMap.h"
#include "engine/GameObject.h"
#include "engine/Input.h"

inline constexpr float kTileSize = 32.0f;

// INHERITANCE: semua benda di peta adalah MapEntity (is-a GameObject).
// Mereka berbagi posisi petak dan status "dihapus", tapi reaksi saat diajak bicara
// (onInteract) atau ditabrak (onBump) berbeda-beda: inilah titik POLYMORPHISM.
class MapEntity : public GameObject {
public:
    MapEntity(int tileX, int tileY) : tileX_(tileX), tileY_(tileY) {}

    int tileX() const { return tileX_; }
    int tileY() const { return tileY_; }
    bool occupies(int x, int y) const { return !removed_ && x == tileX_ && y == tileY_; }
    bool isRemoved() const { return removed_; }

    virtual bool isSolid() const { return true; }
    virtual void onInteract(WorldServices& /*world*/) {}   // pemain menekan Z sambil menghadap
    virtual void onBump(WorldServices& /*world*/) {}       // pemain berjalan menabrak
    virtual std::string label() const { return ""; }       // nama kecil di atas kepala

    void update(float dt) override { time_ += dt; }

protected:
    Vector2 feetPosition() const {
        return {static_cast<float>(tileX_) * kTileSize + kTileSize / 2,
                static_cast<float>(tileY_) * kTileSize + kTileSize - 3};
    }
    void markRemoved() { removed_ = true; }
    float time() const { return time_; }

    int tileX_;
    int tileY_;

private:
    bool removed_ = false;
    float time_ = 0.0f;
};

// Pemain: bergerak per petak dengan animasi geser yang halus.
class Player : public MapEntity {
public:
    Player(int tileX, int tileY, const Appearance& look);

    bool isMoving() const { return moving_; }
    art::Facing facing() const { return facing_; }
    void face(Direction dir);
    void startMove(Direction dir);
    void facedTile(int& x, int& y) const;
    void update(float dt) override;
    void draw() const override;

private:
    const Appearance& look_;
    art::Facing facing_ = art::Facing::Up;
    bool moving_ = false;
    float progress_ = 0.0f;
    int fromX_ = 0;
    int fromY_ = 0;
};

// NPC biasa: berbicara dengan naskah dialognya.
class Npc : public MapEntity {
public:
    Npc(int tileX, int tileY, std::string name, Appearance look, DialogueScript script);
    void onInteract(WorldServices& world) override;
    std::string label() const override { return name_; }
    void draw() const override;

protected:
    void faceToward(const world::Spot& target);
    const DialogueScript& script() const { return script_; }

private:
    std::string name_;
    Appearance look_;
    DialogueScript script_;
    art::Facing facing_ = art::Facing::Down;
};

// Penjaga penginapan: setelah bicara, seluruh party dipulihkan.
class Innkeeper : public Npc {
public:
    Innkeeper(int tileX, int tileY, std::string name, Appearance look, DialogueScript greeting,
              DialogueScript afterRest);
    void onInteract(WorldServices& world) override;

private:
    DialogueScript afterRest_;
};

// Papan petunjuk: benda, bukan karakter, tapi tetap bisa dibaca.
class Signpost : public MapEntity {
public:
    Signpost(int tileX, int tileY, DialogueScript text);
    void onInteract(WorldServices& world) override;
    void draw() const override;

private:
    DialogueScript text_;
};

// Simbol musuh di peta: diajak bicara atau ditabrak akan memulai battle.
class EnemySymbol : public MapEntity {
public:
    using Spawner = std::function<std::vector<std::unique_ptr<Monster>>()>;

    EnemySymbol(int tileX, int tileY, std::string name, Appearance look, Spawner spawner,
                Backdrop backdrop, DialogueScript intro = {},
                std::function<void(WorldServices&)> afterVictory = {});

    void onInteract(WorldServices& world) override { engage(world); }
    void onBump(WorldServices& world) override { engage(world); }
    std::string label() const override { return name_; }
    void draw() const override;

private:
    void engage(WorldServices& world);
    void beginBattle(WorldServices& world);

    std::string name_;
    Appearance look_;
    Spawner spawner_;
    Backdrop backdrop_;
    DialogueScript intro_;
    std::function<void(WorldServices&)> afterVictory_;
    bool engaged_ = false;
};
