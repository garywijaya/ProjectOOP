#pragma once
#include <memory>
#include <vector>

#include "core/Party.h"
#include "core/TileMap.h"
#include "engine/Scene.h"
#include "game/MapEntities.h"
#include "game/WorldServices.h"

// Peta dunia. Scene ini juga mengimplementasikan WorldServices (interface),
// sehingga NPC dan musuh bisa meminta dialog atau battle tanpa tahu detail scene.
class OverworldScene : public Scene, public WorldServices {
public:
    OverworldScene();

    void onEnter() override;
    void update(float dt) override;
    void draw() const override;

    // WorldServices
    void showDialogue(DialogueScript script, std::function<void()> onFinish = {}) override;
    void startBattle(std::vector<std::unique_ptr<Monster>> enemies, Backdrop backdrop,
                     std::function<void()> onVictory) override;
    void restParty() override { party_.restAll(); }
    world::Spot playerTile() const override { return {player_->tileX(), player_->tileY()}; }

private:
    void buildWorld();
    MapEntity* entityAt(int x, int y) const;
    std::string objective() const;
    void drawHud() const;

    Party party_;
    TileMap map_;
    std::unique_ptr<Player> player_;
    std::vector<std::unique_ptr<MapEntity>> entities_;
    EnemySymbol* golem_ = nullptr;
    float time_ = 0.0f;
    bool introShown_ = false;
};
