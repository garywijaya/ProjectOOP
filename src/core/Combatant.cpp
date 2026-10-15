#include "Combatant.h"

#include <algorithm>
#include <cmath>

#include "Skills.h"

namespace {
constexpr int kMinBuffPercent = -50;
constexpr int kMaxBuffPercent = 100;

std::string statName(Stat stat) {
    switch (stat) {
        case Stat::Attack:  return "ATK";
        case Stat::Defense: return "DEF";
        case Stat::Speed:   return "SPD";
    }
    return "?";
}
}  // namespace

Combatant::Combatant(std::string name, BaseStats stats, Element element, Appearance look)
    : name_(std::move(name)),
      stats_(stats),
      element_(element),
      look_(look),
      hp_(std::max(1, stats.maxHp)),
      mp_(std::max(0, stats.maxMp)),
      basicAttack_(std::make_unique<DamageSkill>("Attack", "Serangan biasa.", 0,
                                                 TargetType::SingleEnemy, 1.0, Element::Physical)) {
    stats_.maxHp = hp_;
    stats_.maxMp = mp_;
}

int Combatant::applyModifier(int base, Stat stat) const {
    const int percent = buffPercent(stat);
    return std::max(1, base * (100 + percent) / 100);
}

int Combatant::buffPercent(Stat stat) const {
    int total = 0;
    for (const Buff& b : buffs_)
        if (b.stat == stat) total += b.percent;
    return total;
}

int Combatant::getAttack() const { return applyModifier(stats_.attack, Stat::Attack); }
int Combatant::getDefense() const { return applyModifier(stats_.defense, Stat::Defense); }
int Combatant::getSpeed() const { return applyModifier(stats_.speed, Stat::Speed); }

int Combatant::takeDamage(int rawAmount) {
    if (!isAlive() || rawAmount <= 0) return 0;
    int amount = rawAmount;
    if (defending_) amount = std::max(1, amount / 2);
    amount = std::min(amount, hp_);
    hp_ -= amount;
    return amount;
}

int Combatant::takeDamage(const Attack& attack) {
    const double multiplier = elementMultiplier(attack.element, element_);
    double value = attack.power * multiplier;
    if (!attack.ignoreDefense) value -= getDefense() * 0.5;
    const int amount = std::max(1, static_cast<int>(std::lround(value)));
    return takeDamage(amount);   // memakai overload pertama supaya aturan HP tetap di satu tempat
}

int Combatant::heal(int amount) {
    if (!isAlive() || amount <= 0) return 0;
    const int healed = std::min(amount, stats_.maxHp - hp_);
    hp_ += healed;
    return healed;
}

bool Combatant::revive(int hpAmount) {
    if (isAlive()) return false;
    hp_ = std::clamp(hpAmount, 1, stats_.maxHp);
    buffs_.clear();
    defending_ = false;
    return true;
}

bool Combatant::spendMp(int cost) {
    if (cost < 0 || mp_ < cost) return false;
    mp_ -= cost;
    return true;
}

int Combatant::restoreMp(int amount) {
    if (!isAlive() || amount <= 0) return 0;
    const int restored = std::min(amount, stats_.maxMp - mp_);
    mp_ += restored;
    return restored;
}

void Combatant::applyBuff(Stat stat, int percent, int turns) {
    if (!isAlive() || turns <= 0 || percent == 0) return;
    percent = std::clamp(percent, kMinBuffPercent, kMaxBuffPercent);
    // Aturan: buff/debuff dengan arah yang sama pada stat yang sama tidak menumpuk,
    // hanya diperbarui (supaya stat tidak bisa dinaikkan tanpa batas).
    for (Buff& b : buffs_) {
        if (b.stat == stat && (b.percent > 0) == (percent > 0)) {
            b.percent = percent;
            b.turnsLeft = std::max(b.turnsLeft, turns);
            return;
        }
    }
    buffs_.push_back({stat, percent, turns});
}

void Combatant::defend() {
    if (isAlive()) defending_ = true;
}

void Combatant::fullRestore() {
    hp_ = stats_.maxHp;
    mp_ = stats_.maxMp;
    buffs_.clear();
    defending_ = false;
}

std::vector<const Skill*> Combatant::getSkills() const {
    std::vector<const Skill*> out;
    out.reserve(skills_.size());
    for (const auto& s : skills_) out.push_back(s.get());
    return out;
}

void Combatant::addSkill(std::unique_ptr<Skill> skill) {
    if (skill) skills_.push_back(std::move(skill));
}

void Combatant::onTurnStart(BattleLog& log) {
    defending_ = false;
    for (Buff& b : buffs_) --b.turnsLeft;
    auto expired = std::remove_if(buffs_.begin(), buffs_.end(), [&](const Buff& b) {
        if (b.turnsLeft > 0) return false;
        log.add(name_ + ": efek " + statName(b.stat) + (b.percent > 0 ? " up" : " down") + " habis.");
        return true;
    });
    buffs_.erase(expired, buffs_.end());
}
