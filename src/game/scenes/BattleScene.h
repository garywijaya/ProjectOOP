#pragma once
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "core/Battle.h"
#include "core/Party.h"
#include "engine/Scene.h"
#include "game/WorldServices.h"
#include "raylib.h"

// Tampilan dan kontrol battle. Semua aturan ada di kelas Battle (core);
// scene ini hanya menerjemahkan input menjadi Action dan menggambar hasilnya.
class BattleScene : public Scene {
public:
    enum class Outcome { Victory, Defeat };

    BattleScene(Party& party, std::vector<std::unique_ptr<Monster>> enemies, Backdrop backdrop,
                std::function<void(Outcome)> onFinish);

    void update(float dt) override;
    void draw() const override;

private:
    enum class Phase { Intro, Command, SkillSelect, TargetSelect, Animating, EnemyTurn, Result };

    struct Popup {
        Vector2 pos;
        std::string text;
        Color color;
        float life;
    };

    void decideNextPhase();
    void chooseSkill(const Skill* skill);
    void submit(const Action& action);
    void collectEvents();
    void updateCommand();
    void updateSkillSelect();
    void updateTargetSelect();

    Vector2 positionOf(const Combatant* c) const;
    float scaleOf(const Combatant* c) const;
    std::string infoText() const;

    void drawBackdrop() const;
    void drawCombatants() const;
    void drawTopPanel() const;
    void drawCommandPanel() const;
    void drawStatusPanel() const;
    void drawResult() const;

    Party& party_;
    Battle battle_;
    Backdrop backdrop_;
    std::function<void(Outcome)> onFinish_;

    Phase phase_ = Phase::Intro;
    int commandIndex_ = 0;
    int skillIndex_ = 0;
    int targetIndex_ = 0;
    const Skill* pendingSkill_ = nullptr;
    std::vector<Combatant*> candidates_;
    float timer_ = 0.9f;
    float time_ = 0.0f;
    bool finished_ = false;
    std::vector<Popup> popups_;
    std::map<const Combatant*, float> shake_;
};
