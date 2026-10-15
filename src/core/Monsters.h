#pragma once
#include <random>
#include <vector>

#include "Action.h"
#include "Combatant.h"

// INHERITANCE: Monster is-a Combatant yang dikendalikan AI.
//
// POLYMORPHISM (dinamis): Battle tidak tahu jenis monster apa yang sedang beraksi.
// Battle hanya memanggil decideAction(), dan tiap kelas turunan punya strategi sendiri.
class Monster : public Combatant {
public:
    using Combatant::Combatant;

    bool isPlayerControlled() const override { return false; }

    virtual Action decideAction(const std::vector<Combatant*>& opponents,
                                const std::vector<Combatant*>& allies, std::mt19937& rng) = 0;

protected:
    // Utilitas bersama untuk semua monster.
    static Combatant* randomTarget(const std::vector<Combatant*>& targets, std::mt19937& rng);
    static Combatant* lowestHpTarget(const std::vector<Combatant*>& targets);
    const Skill* findSkill(const std::string& name) const;
};

// Slime: AI paling sederhana, serang target acak, kadang melambatkan.
class Slime : public Monster {
public:
    explicit Slime(std::string name = "Slime");
    std::string getTitle() const override { return "Slime"; }
    Action decideAction(const std::vector<Combatant*>& opponents,
                        const std::vector<Combatant*>& allies, std::mt19937& rng) override;
};

// Ember Wisp: cepat dan licik, selalu mengincar heroine dengan HP terendah.
class EmberWisp : public Monster {
public:
    explicit EmberWisp(std::string name = "Ember Wisp");
    std::string getTitle() const override { return "Wisp"; }
    Action decideAction(const std::vector<Combatant*>& opponents,
                        const std::vector<Combatant*>& allies, std::mt19937& rng) override;
};

// Frost Golem: pola tetap 3 langkah (serang, mengumpulkan es, Glacier Smash ke semua).
class FrostGolem : public Monster {
public:
    explicit FrostGolem(std::string name = "Frost Golem");
    std::string getTitle() const override { return "Golem"; }
    void onBattleStart() override { patternStep_ = 0; }
    Action decideAction(const std::vector<Combatant*>& opponents,
                        const std::vector<Combatant*>& allies, std::mt19937& rng) override;

private:
    int patternStep_ = 0;
};

// Noctyra: boss terakhir dengan dua fase dan satu kali Dark Pact (heal diri).
class FallenWitch : public Monster {
public:
    explicit FallenWitch(std::string name = "Noctyra");
    std::string getTitle() const override { return "Fallen Witch"; }
    void onBattleStart() override;
    void onTurnStart(BattleLog& log) override;
    Action decideAction(const std::vector<Combatant*>& opponents,
                        const std::vector<Combatant*>& allies, std::mt19937& rng) override;
    bool isEnraged() const { return enraged_; }

private:
    bool enraged_ = false;
    bool pactUsed_ = false;
    int turnCount_ = 0;
};
