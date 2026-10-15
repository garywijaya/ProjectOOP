#include "Skills.h"

#include <cmath>

DamageSkill::DamageSkill(std::string name, std::string description, int mpCost, TargetType target,
                         double multiplier, Element element)
    : Skill(std::move(name), std::move(description), mpCost, target),
      multiplier_(multiplier),
      element_(element) {}

void DamageSkill::apply(Combatant& user, Combatant& target, BattleLog& log) const {
    Attack attack;
    attack.power = static_cast<int>(std::lround(user.getAttack() * multiplier_));
    attack.element = element_;
    const double mult = elementMultiplier(element_, target.getElement());
    const int dealt = target.takeDamage(attack);
    std::string line = "  " + target.getName() + " menerima " + std::to_string(dealt) + " damage";
    if (mult > 1.0) line += " (lemah!)";
    else if (mult < 1.0) line += " (tahan)";
    log.add(line + ".");
    log.addEvent({&target, dealt, BattleEvent::Type::Damage});
    if (!target.isAlive()) log.add("  " + target.getName() + " tumbang!");
}

HealSkill::HealSkill(std::string name, std::string description, int mpCost, TargetType target,
                     int basePower)
    : Skill(std::move(name), std::move(description), mpCost, target), basePower_(basePower) {}

void HealSkill::apply(Combatant& user, Combatant& target, BattleLog& log) const {
    const int healed = target.heal(basePower_ + user.getAttack());
    log.add("  " + target.getName() + " pulih " + std::to_string(healed) + " HP.");
    log.addEvent({&target, healed, BattleEvent::Type::Heal});
}

BuffSkill::BuffSkill(std::string name, std::string description, int mpCost, TargetType target,
                     Stat stat, int percent, int turns)
    : Skill(std::move(name), std::move(description), mpCost, target),
      stat_(stat),
      percent_(percent),
      turns_(turns) {}

void BuffSkill::apply(Combatant& /*user*/, Combatant& target, BattleLog& log) const {
    target.applyBuff(stat_, percent_, turns_);
    const char* statText = stat_ == Stat::Attack ? "ATK" : stat_ == Stat::Defense ? "DEF" : "SPD";
    log.add("  " + target.getName() + ": " + statText + (percent_ > 0 ? " naik " : " turun ") +
            std::to_string(std::abs(percent_)) + "%.");
    log.addEvent({&target, percent_,
                  percent_ > 0 ? BattleEvent::Type::BuffUp : BattleEvent::Type::BuffDown});
}

ReviveSkill::ReviveSkill(std::string name, std::string description, int mpCost, int hpPercent)
    : Skill(std::move(name), std::move(description), mpCost, TargetType::SingleAlly),
      hpPercent_(hpPercent) {}

bool ReviveSkill::isValidTarget(const Combatant& user, const Combatant& target) const {
    return &user != &target && !target.isAlive();
}

void ReviveSkill::apply(Combatant& /*user*/, Combatant& target, BattleLog& log) const {
    const int hp = target.getMaxHp() * hpPercent_ / 100;
    if (target.revive(hp)) {
        log.add("  " + target.getName() + " bangkit kembali!");
        log.addEvent({&target, target.getHp(), BattleEvent::Type::Revive});
    }
}
