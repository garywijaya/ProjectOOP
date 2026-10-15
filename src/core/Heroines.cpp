#include "Heroines.h"

#include <memory>

#include "Skills.h"

namespace {
Appearance makeLook(Rgb hair, Rgb outfit, Rgb eyes, HairStyle style, Accessory acc) {
    Appearance a;
    a.body = BodyType::Heroine;
    a.hair = hair;
    a.outfit = outfit;
    a.eyes = eyes;
    a.hairStyle = style;
    a.accessory = acc;
    return a;
}
}  // namespace

// ---------------- Swordswoman ----------------
Swordswoman::Swordswoman(std::string name)
    : Heroine(std::move(name), {130, 32, 22, 12, 12}, Element::Fire,
              makeLook({214, 56, 64}, {60, 70, 110}, {255, 170, 60}, HairStyle::Ponytail,
                       Accessory::Ribbon)) {
    addSkill(std::make_unique<DamageSkill>("Flame Slash", "Tebasan api ke satu musuh.", 6,
                                           TargetType::SingleEnemy, 1.6, Element::Fire));
    addSkill(std::make_unique<DamageSkill>("Blade Dance", "Menyerang semua musuh.", 10,
                                           TargetType::AllEnemies, 0.9, Element::Physical));
    addSkill(std::make_unique<BuffSkill>("War Cry", "ATK diri naik 30% (3 giliran).", 6,
                                         TargetType::Self, Stat::Attack, 30, 3));
}

std::string Swordswoman::getBattleCry() const { return "Pedangku tidak akan tumpul!"; }
std::string Swordswoman::getPassiveDescription() const {
    return "Last Stand: saat HP < 30%, ATK +40% (sekali per battle).";
}

void Swordswoman::onTurnStart(BattleLog& log) {
    Heroine::onTurnStart(log);
    if (!lastStandUsed_ && isAlive() && getHp() * 100 < getMaxHp() * 30) {
        lastStandUsed_ = true;
        applyBuff(Stat::Attack, 40, 3);
        log.add(getName() + " mengaktifkan Last Stand! ATK naik 40%.");
        log.addEvent({this, 40, BattleEvent::Type::BuffUp});
    }
}

// ---------------- Mage ----------------
Mage::Mage(std::string name)
    : Heroine(std::move(name), {90, 60, 26, 7, 14}, Element::Ice,
              makeLook({170, 210, 255}, {70, 60, 140}, {90, 160, 255}, HairStyle::Twintails,
                       Accessory::Tiara)) {
    addSkill(std::make_unique<DamageSkill>("Ice Lance", "Tombak es ke satu musuh.", 7,
                                           TargetType::SingleEnemy, 1.8, Element::Ice));
    addSkill(std::make_unique<DamageSkill>("Fireball", "Bola api, efektif ke musuh es.", 8,
                                           TargetType::SingleEnemy, 1.6, Element::Fire));
    addSkill(std::make_unique<DamageSkill>("Blizzard", "Badai es ke semua musuh.", 16,
                                           TargetType::AllEnemies, 1.1, Element::Ice));
}

std::string Mage::getBattleCry() const { return "Hmph, ini cuma pemanasan."; }
std::string Mage::getPassiveDescription() const { return "Mana Flow: +4 MP setiap giliran."; }

void Mage::onTurnStart(BattleLog& log) {
    Heroine::onTurnStart(log);
    const int gained = restoreMp(4);
    if (gained > 0) log.addEvent({this, gained, BattleEvent::Type::MpRestore});
}

// ---------------- Priestess ----------------
Priestess::Priestess(std::string name)
    : Heroine(std::move(name), {100, 58, 18, 10, 10}, Element::Light,
              makeLook({250, 220, 120}, {240, 240, 250}, {120, 200, 140}, HairStyle::Long,
                       Accessory::Tiara)) {
    addSkill(std::make_unique<HealSkill>("Heal", "Memulihkan HP satu sekutu.", 7,
                                         TargetType::SingleAlly, 32));
    addSkill(std::make_unique<DamageSkill>("Holy Light", "Cahaya suci, efektif ke Dark.", 8,
                                           TargetType::SingleEnemy, 1.8, Element::Light));
    addSkill(std::make_unique<BuffSkill>("Blessing", "DEF semua sekutu naik 40%.", 10,
                                         TargetType::AllAllies, Stat::Defense, 40, 3));
    addSkill(std::make_unique<ReviveSkill>("Revive", "Membangkitkan sekutu yang pingsan.", 14, 40));
}

std::string Priestess::getBattleCry() const { return "Semoga cahaya melindungi kita."; }
std::string Priestess::getPassiveDescription() const {
    return "Grace: memulihkan 5% HP sendiri setiap giliran.";
}

void Priestess::onTurnStart(BattleLog& log) {
    Heroine::onTurnStart(log);
    const int healed = heal(getMaxHp() * 5 / 100);
    if (healed > 0) log.addEvent({this, healed, BattleEvent::Type::Heal});
}
