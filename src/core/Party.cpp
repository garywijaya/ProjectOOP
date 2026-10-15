#include "Party.h"

#include <algorithm>

Party Party::createStartingParty() {
    Party party;
    party.add(std::make_unique<Swordswoman>("Aria"));
    party.add(std::make_unique<Mage>("Lumi"));
    party.add(std::make_unique<Priestess>("Seraphine"));
    return party;
}

bool Party::add(std::unique_ptr<Heroine> heroine) {
    if (!heroine || members_.size() >= kMaxMembers) return false;
    members_.push_back(std::move(heroine));
    return true;
}

std::vector<Combatant*> Party::asCombatants() const {
    std::vector<Combatant*> out;
    for (const auto& h : members_) out.push_back(h.get());
    return out;
}

bool Party::isWiped() const {
    return std::none_of(members_.begin(), members_.end(),
                        [](const std::unique_ptr<Heroine>& h) { return h->isAlive(); });
}

void Party::restAll() {
    for (auto& h : members_) h->fullRestore();
}

void Party::reviveFallenAfterVictory() {
    for (auto& h : members_) h->revive(1);   // revive() sendiri menolak jika masih hidup
}
