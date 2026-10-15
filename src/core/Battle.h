#pragma once
#include <memory>
#include <random>
#include <vector>

#include "Action.h"
#include "BattleLog.h"
#include "Combatant.h"
#include "Monsters.h"

// ABSTRACTION: Battle menyembunyikan seluruh aturan pertarungan turn-based
// (urutan giliran berdasarkan SPD, pasif awal giliran, validasi aksi, AI monster,
// penentuan menang/kalah). Tampilan (BattleScene) hanya perlu:
//   - bertanya isWaitingForPlayer() / getCurrentActor()
//   - mengirim pilihan pemain lewat submitPlayerAction()
//   - memanggil runEnemyTurn() saat giliran monster
//
// ENCAPSULATION: urutan giliran, indeks giliran, dan hasil battle tidak bisa diubah dari luar.
// submitPlayerAction() menolak aksi yang tidak sah (bukan gilirannya, MP kurang, target salah).
class Battle {
public:
    enum class Result { Ongoing, Victory, Defeat };

    Battle(std::vector<Combatant*> heroes, std::vector<std::unique_ptr<Monster>> enemies,
           unsigned seed);

    Result getResult() const { return result_; }
    bool isOver() const { return result_ != Result::Ongoing; }
    bool isWaitingForPlayer() const;
    Combatant* getCurrentActor() const { return current_; }
    int getRound() const { return round_; }

    const std::vector<Combatant*>& getHeroes() const { return heroes_; }
    const std::vector<Combatant*>& getEnemies() const { return enemies_; }
    std::vector<Combatant*> getTargetCandidates(const Combatant& user, const Skill& skill) const;
    std::vector<Combatant*> getUpcomingTurns(std::size_t count) const;

    BattleLog& getLog() { return log_; }
    const BattleLog& getLog() const { return log_; }

    bool submitPlayerAction(const Action& action);
    void runEnemyTurn();

private:
    void startRound();
    void beginNextTurn();
    void execute(const Action& action);
    void updateResult();
    bool ownsSkill(const Combatant& actor, const Skill* skill) const;
    bool isHero(const Combatant* c) const;
    std::vector<Combatant*> aliveOpponentsOf(const Combatant& c) const;
    std::vector<Combatant*> aliveAlliesOf(const Combatant& c) const;
    std::vector<Combatant*> alliesOf(const Combatant& c) const;
    std::vector<Combatant*> opponentsOf(const Combatant& c) const;

    std::vector<Combatant*> heroes_;                       // tidak dimiliki (milik Party)
    std::vector<std::unique_ptr<Monster>> ownedEnemies_;   // dimiliki battle ini
    std::vector<Combatant*> enemies_;
    std::vector<Monster*> monsters_;
    std::vector<Combatant*> turnOrder_;
    std::size_t turnIndex_ = 0;
    Combatant* current_ = nullptr;
    int round_ = 0;
    Result result_ = Result::Ongoing;
    BattleLog log_;
    std::mt19937 rng_;
};
