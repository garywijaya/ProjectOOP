#include "MapEntities.h"

#include <cmath>

#include "engine/Ui.h"

namespace {
constexpr float kMoveSpeed = 5.5f;   // petak per detik

void drawLabel(const std::string& text, Vector2 feet) {
    if (text.empty()) return;
    const int w = ui::textWidth(text, 10);
    DrawRectangleRounded({feet.x - w / 2.0f - 4, feet.y - 46, static_cast<float>(w) + 8, 14}, 0.5f, 6, {0, 0, 0, 140});
    DrawText(text.c_str(), static_cast<int>(feet.x - w / 2.0f), static_cast<int>(feet.y - 44), 10, RAYWHITE);
}
}  // namespace

// ---------------- Player ----------------
Player::Player(int tileX, int tileY, const Appearance& look)
    : MapEntity(tileX, tileY), look_(look), fromX_(tileX), fromY_(tileY) {}

void Player::face(Direction dir) {
    switch (dir) {
        case Direction::Up:    facing_ = art::Facing::Up; break;
        case Direction::Down:  facing_ = art::Facing::Down; break;
        case Direction::Left:  facing_ = art::Facing::Left; break;
        case Direction::Right: facing_ = art::Facing::Right; break;
        case Direction::None:  break;
    }
}

void Player::facedTile(int& x, int& y) const {
    x = tileX_;
    y = tileY_;
    switch (facing_) {
        case art::Facing::Up:    --y; break;
        case art::Facing::Down:  ++y; break;
        case art::Facing::Left:  --x; break;
        case art::Facing::Right: ++x; break;
    }
}

void Player::startMove(Direction dir) {
    if (moving_ || dir == Direction::None) return;
    face(dir);
    fromX_ = tileX_;
    fromY_ = tileY_;
    facedTile(tileX_, tileY_);
    moving_ = true;
    progress_ = 0.0f;
}

void Player::update(float dt) {
    MapEntity::update(dt);
    if (!moving_) return;
    progress_ += dt * kMoveSpeed;
    if (progress_ >= 1.0f) {
        progress_ = 1.0f;
        moving_ = false;
    }
}

void Player::draw() const {
    Vector2 feet = feetPosition();
    if (moving_) {
        const float t = 1.0f - progress_;
        feet.x -= static_cast<float>(tileX_ - fromX_) * kTileSize * t;
        feet.y -= static_cast<float>(tileY_ - fromY_) * kTileSize * t;
    }
    art::drawCharacter(look_, feet, 0.85f, time(), facing_, moving_);
}

// ---------------- Npc ----------------
Npc::Npc(int tileX, int tileY, std::string name, Appearance look, DialogueScript script)
    : MapEntity(tileX, tileY), name_(std::move(name)), look_(look), script_(std::move(script)) {}

void Npc::faceToward(const world::Spot& target) {
    const int dx = target.x - tileX_;
    const int dy = target.y - tileY_;
    if (std::abs(dx) > std::abs(dy)) facing_ = dx > 0 ? art::Facing::Right : art::Facing::Left;
    else facing_ = dy > 0 ? art::Facing::Down : art::Facing::Up;
}

void Npc::onInteract(WorldServices& world) {
    faceToward(world.playerTile());
    world.showDialogue(script_);
}

void Npc::draw() const {
    art::drawCharacter(look_, feetPosition(), 0.85f, time(), facing_, false);
    drawLabel(name_, feetPosition());
}

// ---------------- Innkeeper ----------------
Innkeeper::Innkeeper(int tileX, int tileY, std::string name, Appearance look, DialogueScript greeting,
                     DialogueScript afterRest)
    : Npc(tileX, tileY, std::move(name), look, std::move(greeting)), afterRest_(std::move(afterRest)) {}

void Innkeeper::onInteract(WorldServices& world) {
    faceToward(world.playerTile());
    world.showDialogue(script(), [this, &world] {
        world.restParty();
        world.showDialogue(afterRest_);
    });
}

// ---------------- Signpost ----------------
Signpost::Signpost(int tileX, int tileY, DialogueScript text) : MapEntity(tileX, tileY), text_(std::move(text)) {}

void Signpost::onInteract(WorldServices& world) { world.showDialogue(text_); }

void Signpost::draw() const { art::drawSignpost(feetPosition(), 1.0f); }

// ---------------- EnemySymbol ----------------
EnemySymbol::EnemySymbol(int tileX, int tileY, std::string name, Appearance look, Spawner spawner,
                         Backdrop backdrop, DialogueScript intro,
                         std::function<void(WorldServices&)> afterVictory)
    : MapEntity(tileX, tileY),
      name_(std::move(name)),
      look_(look),
      spawner_(std::move(spawner)),
      backdrop_(backdrop),
      intro_(std::move(intro)),
      afterVictory_(std::move(afterVictory)) {}

void EnemySymbol::engage(WorldServices& world) {
    if (engaged_ || isRemoved()) return;
    engaged_ = true;
    if (intro_.empty()) beginBattle(world);
    else world.showDialogue(intro_, [this, &world] { beginBattle(world); });
}

void EnemySymbol::beginBattle(WorldServices& world) {
    world.startBattle(spawner_(), backdrop_, [this, &world] {
        markRemoved();
        if (afterVictory_) afterVictory_(world);
    });
}

void EnemySymbol::draw() const {
    const float scale = look_.body == BodyType::Golem ? 0.6f : 0.85f;
    art::drawCharacter(look_, feetPosition(), scale, time(), art::Facing::Down, false);
    // Tanda "!" berkedip supaya pemain tahu ini musuh.
    if (std::fmod(time(), 1.2f) < 0.8f) {
        const Vector2 f = feetPosition();
        DrawCircleV({f.x + 12, f.y - 34}, 6, {230, 60, 70, 230});
        DrawText("!", static_cast<int>(f.x + 10), static_cast<int>(f.y - 41), 14, RAYWHITE);
    }
    drawLabel(name_, feetPosition());
}
