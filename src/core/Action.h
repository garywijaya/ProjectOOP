#pragma once
#include <vector>

class Combatant;
class Skill;

// Satu aksi dalam pertarungan, baik dipilih pemain lewat menu maupun oleh AI monster.
struct Action {
    enum class Kind { UseSkill, Defend };
    Kind kind = Kind::UseSkill;
    Combatant* actor = nullptr;
    const Skill* skill = nullptr;
    std::vector<Combatant*> targets;

    static Action defend(Combatant* actor) {
        Action a;
        a.kind = Kind::Defend;
        a.actor = actor;
        return a;
    }
    static Action useSkill(Combatant* actor, const Skill* skill, std::vector<Combatant*> targets) {
        Action a;
        a.kind = Kind::UseSkill;
        a.actor = actor;
        a.skill = skill;
        a.targets = std::move(targets);
        return a;
    }
};
