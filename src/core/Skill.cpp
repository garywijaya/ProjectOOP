#include "Skill.h"

#include <algorithm>

#include "BattleLog.h"
#include "Combatant.h"

Skill::Skill(std::string name, std::string description, int mpCost, TargetType targetType)
    : name_(std::move(name)),
      description_(std::move(description)),
      mpCost_(std::max(0, mpCost)),
      targetType_(targetType) {}

bool Skill::targetsAllies() const {
    return targetType_ == TargetType::SingleAlly || targetType_ == TargetType::AllAllies ||
           targetType_ == TargetType::Self;
}

bool Skill::targetsMultiple() const {
    return targetType_ == TargetType::AllEnemies || targetType_ == TargetType::AllAllies;
}

bool Skill::canUse(const Combatant& user) const {
    return user.isAlive() && user.getMp() >= mpCost_;
}

bool Skill::isValidTarget(const Combatant& /*user*/, const Combatant& target) const {
    return target.isAlive();
}

bool Skill::use(Combatant& user, const std::vector<Combatant*>& targets, BattleLog& log) const {
    if (!canUse(user)) {
        log.add(user.getName() + " tidak punya cukup MP untuk " + name_ + ".");
        return false;
    }
    std::vector<Combatant*> valid;
    for (Combatant* t : targets)
        if (t && isValidTarget(user, *t)) valid.push_back(t);
    if (valid.empty()) {
        log.add(name_ + " tidak punya target yang valid.");
        return false;
    }
    user.spendMp(mpCost_);
    log.add(user.getName() + " memakai " + name_ + "!");
    for (Combatant* t : valid) apply(user, *t, log);
    return true;
}
