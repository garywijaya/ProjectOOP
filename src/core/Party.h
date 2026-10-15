#pragma once
#include <memory>
#include <vector>

#include "Heroines.h"

// ENCAPSULATION: Party memiliki (own) heroine-heroine-nya.
// Aturan yang dijaga: maksimal 3 anggota, tidak boleh anggota kosong,
// dan setelah menang, anggota yang pingsan bangkit dengan HP minimal.
class Party {
public:
    static constexpr std::size_t kMaxMembers = 3;

    static Party createStartingParty();

    bool add(std::unique_ptr<Heroine> heroine);
    std::size_t size() const { return members_.size(); }
    const Heroine& member(std::size_t index) const { return *members_.at(index); }
    Heroine& member(std::size_t index) { return *members_.at(index); }

    std::vector<Combatant*> asCombatants() const;
    bool isWiped() const;
    void restAll();                 // dipakai penginapan
    void reviveFallenAfterVictory();

private:
    std::vector<std::unique_ptr<Heroine>> members_;
};
