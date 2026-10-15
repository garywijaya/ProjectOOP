#pragma once
#include "Combatant.h"

// INHERITANCE: Heroine is-a Combatant yang dikendalikan pemain.
// Kelas ini tetap abstrak: tiap heroine wajib menentukan title, battle cry, dan pasifnya.
class Heroine : public Combatant {
public:
    using Combatant::Combatant;

    bool isPlayerControlled() const override { return true; }
    virtual std::string getBattleCry() const = 0;
    virtual std::string getPassiveDescription() const = 0;
};

// Aria: petarung garis depan. Pasif "Last Stand": saat HP < 30%, ATK naik sekali per battle.
class Swordswoman : public Heroine {
public:
    explicit Swordswoman(std::string name);
    std::string getTitle() const override { return "Swordswoman"; }
    std::string getBattleCry() const override;
    std::string getPassiveDescription() const override;
    void onTurnStart(BattleLog& log) override;
    void onBattleStart() override { lastStandUsed_ = false; }

private:
    bool lastStandUsed_ = false;
};

// Lumi: penyihir es. Pasif "Mana Flow": memulihkan MP setiap giliran.
class Mage : public Heroine {
public:
    explicit Mage(std::string name);
    std::string getTitle() const override { return "Mage"; }
    std::string getBattleCry() const override;
    std::string getPassiveDescription() const override;
    void onTurnStart(BattleLog& log) override;
};

// Seraphine: penyembuh. Pasif "Grace": memulihkan sedikit HP sendiri setiap giliran.
class Priestess : public Heroine {
public:
    explicit Priestess(std::string name);
    std::string getTitle() const override { return "Priestess"; }
    std::string getBattleCry() const override;
    std::string getPassiveDescription() const override;
    void onTurnStart(BattleLog& log) override;
};
