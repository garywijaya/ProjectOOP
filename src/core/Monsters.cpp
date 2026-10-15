#include "Monsters.h"

#include <algorithm>

#include "Skills.h"

namespace {
Appearance monsterLook(BodyType body, Rgb main, Rgb secondary, Rgb eyes) {
    Appearance a;
    a.body = body;
    a.hair = main;
    a.outfit = secondary;
    a.eyes = eyes;
    return a;
}
}  // namespace

// ---------------- Monster (helper bersama) ----------------
Combatant* Monster::randomTarget(const std::vector<Combatant*>& targets, std::mt19937& rng) {
    if (targets.empty()) return nullptr;
    std::uniform_int_distribution<std::size_t> pick(0, targets.size() - 1);
    return targets[pick(rng)];
}

Combatant* Monster::lowestHpTarget(const std::vector<Combatant*>& targets) {
    if (targets.empty()) return nullptr;
    return *std::min_element(targets.begin(), targets.end(), [](Combatant* a, Combatant* b) {
        return a->getHp() < b->getHp();
    });
}

const Skill* Monster::findSkill(const std::string& name) const {
    for (const Skill* s : getSkills())
        if (s->getName() == name) return s;
    return &basicAttack();
}

// ---------------- Slime ----------------
Slime::Slime(std::string name)
    : Monster(std::move(name), {42, 0, 15, 4, 6}, Element::Physical,
              monsterLook(BodyType::Slime, {90, 200, 120}, {50, 150, 90}, {20, 40, 30})) {
    addSkill(std::make_unique<BuffSkill>("Sticky Goo", "SPD target turun.", 0,
                                         TargetType::SingleEnemy, Stat::Speed, -30, 2));
}

Action Slime::decideAction(const std::vector<Combatant*>& opponents,
                           const std::vector<Combatant*>&, std::mt19937& rng) {
    std::uniform_int_distribution<int> roll(1, 100);
    const Skill* skill = roll(rng) <= 25 ? findSkill("Sticky Goo") : &basicAttack();
    return Action::useSkill(this, skill, {randomTarget(opponents, rng)});
}

// ---------------- Ember Wisp ----------------
EmberWisp::EmberWisp(std::string name)
    : Monster(std::move(name), {36, 0, 18, 3, 16}, Element::Fire,
              monsterLook(BodyType::Wisp, {255, 140, 50}, {255, 220, 90}, {60, 20, 10})) {
    addSkill(std::make_unique<DamageSkill>("Ember", "Percikan api.", 0, TargetType::SingleEnemy,
                                           1.2, Element::Fire));
}

Action EmberWisp::decideAction(const std::vector<Combatant*>& opponents,
                               const std::vector<Combatant*>&, std::mt19937&) {
    return Action::useSkill(this, findSkill("Ember"), {lowestHpTarget(opponents)});
}

// ---------------- Frost Golem ----------------
FrostGolem::FrostGolem(std::string name)
    : Monster(std::move(name), {280, 0, 24, 16, 5}, Element::Ice,
              monsterLook(BodyType::Golem, {150, 200, 235}, {90, 130, 170}, {120, 240, 255})) {
    addSkill(std::make_unique<BuffSkill>("Gather Frost", "Mengumpulkan es, DEF naik.", 0,
                                         TargetType::Self, Stat::Defense, 50, 2));
    addSkill(std::make_unique<DamageSkill>("Glacier Smash", "Hantaman es ke semua lawan.", 0,
                                           TargetType::AllEnemies, 1.35, Element::Ice));
}

Action FrostGolem::decideAction(const std::vector<Combatant*>& opponents,
                                const std::vector<Combatant*>&, std::mt19937& rng) {
    const int step = patternStep_;
    patternStep_ = (patternStep_ + 1) % 3;
    switch (step) {
        case 1:  return Action::useSkill(this, findSkill("Gather Frost"), {this});
        case 2:  return Action::useSkill(this, findSkill("Glacier Smash"), opponents);
        default: return Action::useSkill(this, &basicAttack(), {randomTarget(opponents, rng)});
    }
}

// ---------------- Fallen Witch (boss) ----------------
FallenWitch::FallenWitch(std::string name)
    : Monster(std::move(name), {500, 999, 28, 12, 13}, Element::Dark,
              [] {
                  Appearance a;
                  a.body = BodyType::Heroine;
                  a.hair = {70, 40, 110};
                  a.outfit = {35, 25, 55};
                  a.eyes = {230, 60, 90};
                  a.hairStyle = HairStyle::Long;
                  a.accessory = Accessory::WitchHat;
                  return a;
              }()) {
    addSkill(std::make_unique<DamageSkill>("Shadow Bolt", "Peluru bayangan.", 0,
                                           TargetType::SingleEnemy, 1.5, Element::Dark));
    addSkill(std::make_unique<BuffSkill>("Hex", "Kutukan, ATK target turun.", 0,
                                         TargetType::SingleEnemy, Stat::Attack, -30, 3));
    addSkill(std::make_unique<DamageSkill>("Eclipse", "Gerhana kegelapan ke semua lawan.", 0,
                                           TargetType::AllEnemies, 1.1, Element::Dark));
    addSkill(std::make_unique<HealSkill>("Dark Pact", "Menyerap kegelapan untuk pulih.", 0,
                                         TargetType::Self, 70));
}

void FallenWitch::onBattleStart() {
    enraged_ = false;
    pactUsed_ = false;
    turnCount_ = 0;
}

void FallenWitch::onTurnStart(BattleLog& log) {
    Monster::onTurnStart(log);
    ++turnCount_;
    if (!enraged_ && getHp() * 2 < getMaxHp()) {
        enraged_ = true;
        applyBuff(Stat::Attack, 20, 99);
        log.add("Noctyra: \"Kalian... cukup kuat. Baiklah, aku tidak menahan diri lagi!\"");
        log.addEvent({this, 20, BattleEvent::Type::BuffUp});
    }
}

Action FallenWitch::decideAction(const std::vector<Combatant*>& opponents,
                                 const std::vector<Combatant*>&, std::mt19937& rng) {
    if (enraged_ && !pactUsed_ && getHp() * 10 < getMaxHp() * 3) {
        pactUsed_ = true;
        return Action::useSkill(this, findSkill("Dark Pact"), {this});
    }
    if (enraged_ && turnCount_ % 2 == 0) return Action::useSkill(this, findSkill("Eclipse"), opponents);
    if (!enraged_ && turnCount_ % 3 == 0) return Action::useSkill(this, findSkill("Eclipse"), opponents);

    std::uniform_int_distribution<int> roll(1, 100);
    if (roll(rng) <= 25) {
        // Hex diarahkan ke heroine dengan ATK tertinggi.
        Combatant* strongest = *std::max_element(opponents.begin(), opponents.end(),
            [](Combatant* a, Combatant* b) { return a->getAttack() < b->getAttack(); });
        return Action::useSkill(this, findSkill("Hex"), {strongest});
    }
    return Action::useSkill(this, findSkill("Shadow Bolt"), {randomTarget(opponents, rng)});
}
