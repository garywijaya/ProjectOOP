#pragma once
#include "Combatant.h"
#include "Element.h"
#include "Skill.h"

// INHERITANCE: setiap kelas di bawah ini adalah (is-a) Skill.
// Semuanya berbagi nama, biaya MP, tipe target, dan alur use(),
// tetapi masing-masing punya efek berbeda lewat override apply().

class DamageSkill : public Skill {
public:
    DamageSkill(std::string name, std::string description, int mpCost, TargetType target,
                double multiplier, Element element);
    double getMultiplier() const { return multiplier_; }
    Element getElement() const { return element_; }

protected:
    void apply(Combatant& user, Combatant& target, BattleLog& log) const override;

private:
    double multiplier_;
    Element element_;
};

class HealSkill : public Skill {
public:
    HealSkill(std::string name, std::string description, int mpCost, TargetType target,
              int basePower);

protected:
    void apply(Combatant& user, Combatant& target, BattleLog& log) const override;

private:
    int basePower_;
};

class BuffSkill : public Skill {
public:
    // percent negatif berarti debuff (dipakai ke musuh).
    BuffSkill(std::string name, std::string description, int mpCost, TargetType target,
              Stat stat, int percent, int turns);

protected:
    void apply(Combatant& user, Combatant& target, BattleLog& log) const override;

private:
    Stat stat_;
    int percent_;
    int turns_;
};

class ReviveSkill : public Skill {
public:
    ReviveSkill(std::string name, std::string description, int mpCost, int hpPercent);

    // Override aturan target: justru hanya sekutu yang sedang pingsan.
    bool isValidTarget(const Combatant& user, const Combatant& target) const override;

protected:
    void apply(Combatant& user, Combatant& target, BattleLog& log) const override;

private:
    int hpPercent_;
};
