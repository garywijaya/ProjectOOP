#pragma once
#include <functional>
#include <memory>
#include <vector>

#include "core/Dialogue.h"
#include "core/Monsters.h"
#include "core/WorldData.h"

enum class Backdrop { Field, Bridge, Castle };

// ABSTRACTION (interface): layanan yang boleh dipakai entitas di peta.
// NPC atau musuh tidak perlu tahu tentang SceneManager, BattleScene, atau Party.
// Mereka cukup meminta "tampilkan dialog" atau "mulai battle".
// OverworldScene adalah kelas yang mengimplementasikan interface ini.
class WorldServices {
public:
    virtual ~WorldServices() = default;

    virtual void showDialogue(DialogueScript script, std::function<void()> onFinish = {}) = 0;
    virtual void startBattle(std::vector<std::unique_ptr<Monster>> enemies, Backdrop backdrop,
                             std::function<void()> onVictory) = 0;
    virtual void restParty() = 0;
    virtual world::Spot playerTile() const = 0;
};
