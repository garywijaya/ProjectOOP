#include "Battle.h"

#include <algorithm>

Battle::Battle(std::vector<Combatant*> heroes, std::vector<std::unique_ptr<Monster>> enemies,
               unsigned seed)
    : heroes_(std::move(heroes)), ownedEnemies_(std::move(enemies)), rng_(seed) {
    for (auto& m : ownedEnemies_) {
        enemies_.push_back(m.get());
        monsters_.push_back(m.get());
    }
    for (Combatant* c : heroes_) c->onBattleStart();
    for (Combatant* c : enemies_) c->onBattleStart();

    std::string names;
    for (std::size_t i = 0; i < enemies_.size(); ++i)
        names += (i ? ", " : "") + enemies_[i]->getName();
    log_.add("Muncul: " + names + "!");

    updateResult();
    if (!isOver()) {
        startRound();
        beginNextTurn();
    }
}

bool Battle::isWaitingForPlayer() const {
    return !isOver() && current_ && current_->isPlayerControlled();
}

bool Battle::isHero(const Combatant* c) const {
    return std::find(heroes_.begin(), heroes_.end(), c) != heroes_.end();
}

std::vector<Combatant*> Battle::alliesOf(const Combatant& c) const {
    return isHero(&c) ? heroes_ : enemies_;
}

std::vector<Combatant*> Battle::opponentsOf(const Combatant& c) const {
    return isHero(&c) ? enemies_ : heroes_;
}

std::vector<Combatant*> Battle::aliveAlliesOf(const Combatant& c) const {
    std::vector<Combatant*> out;
    for (Combatant* x : alliesOf(c)) if (x->isAlive()) out.push_back(x);
    return out;
}

std::vector<Combatant*> Battle::aliveOpponentsOf(const Combatant& c) const {
    std::vector<Combatant*> out;
    for (Combatant* x : opponentsOf(c)) if (x->isAlive()) out.push_back(x);
    return out;
}

std::vector<Combatant*> Battle::getTargetCandidates(const Combatant& user, const Skill& skill) const {
    std::vector<Combatant*> pool;
    if (skill.getTargetType() == TargetType::Self) pool = {const_cast<Combatant*>(&user)};
    else pool = skill.targetsAllies() ? alliesOf(user) : opponentsOf(user);

    std::vector<Combatant*> out;
    for (Combatant* c : pool)
        if (skill.isValidTarget(user, *c)) out.push_back(c);
    return out;
}

std::vector<Combatant*> Battle::getUpcomingTurns(std::size_t count) const {
    std::vector<Combatant*> out;
    for (std::size_t i = turnIndex_; i < turnOrder_.size() && out.size() < count; ++i)
        if (turnOrder_[i]->isAlive()) out.push_back(turnOrder_[i]);
    return out;
}

bool Battle::ownsSkill(const Combatant& actor, const Skill* skill) const {
    if (skill == &actor.basicAttack()) return true;
    const auto skills = actor.getSkills();
    return std::find(skills.begin(), skills.end(), skill) != skills.end();
}

void Battle::startRound() {
    ++round_;
    turnOrder_.clear();
    for (Combatant* c : heroes_) if (c->isAlive()) turnOrder_.push_back(c);
    for (Combatant* c : enemies_) if (c->isAlive()) turnOrder_.push_back(c);
    // Urutan berdasarkan SPD (setelah buff). stable_sort: kalau sama, heroine duluan.
    std::stable_sort(turnOrder_.begin(), turnOrder_.end(),
                     [](Combatant* a, Combatant* b) { return a->getSpeed() > b->getSpeed(); });
    turnIndex_ = 0;
}

void Battle::beginNextTurn() {
    current_ = nullptr;
    while (!isOver()) {
        if (turnIndex_ >= turnOrder_.size()) startRound();
        Combatant* next = turnOrder_[turnIndex_];
        if (!next->isAlive()) {
            ++turnIndex_;
            continue;
        }
        current_ = next;
        current_->onTurnStart(log_);
        updateResult();
        return;
    }
}

bool Battle::submitPlayerAction(const Action& action) {
    if (!isWaitingForPlayer() || action.actor != current_) return false;

    Action checked = action;
    if (action.kind == Action::Kind::UseSkill) {
        if (!action.skill || !ownsSkill(*current_, action.skill)) return false;
        if (!action.skill->canUse(*current_)) return false;
        const auto candidates = getTargetCandidates(*current_, *action.skill);
        if (candidates.empty()) return false;
        if (action.skill->targetsMultiple() || action.skill->getTargetType() == TargetType::Self) {
            checked.targets = candidates;
        } else {
            if (action.targets.size() != 1) return false;
            if (std::find(candidates.begin(), candidates.end(), action.targets[0]) == candidates.end())
                return false;
        }
    }
    execute(checked);
    return true;
}

void Battle::runEnemyTurn() {
    if (isOver() || !current_ || current_->isPlayerControlled()) return;
    Monster* monster = nullptr;
    for (Monster* m : monsters_) if (m == current_) monster = m;
    if (!monster) return;

    // Polymorphism: tiap jenis monster memutuskan aksinya sendiri.
    Action action = monster->decideAction(aliveOpponentsOf(*monster), aliveAlliesOf(*monster), rng_);
    action.actor = monster;
    if (action.kind == Action::Kind::UseSkill && action.skill && action.skill->targetsMultiple())
        action.targets = getTargetCandidates(*monster, *action.skill);
    execute(action);
}

void Battle::execute(const Action& action) {
    Combatant* actor = action.actor;
    if (action.kind == Action::Kind::Defend) {
        actor->defend();
        actor->restoreMp(4);
        log_.add(actor->getName() + " bertahan (damage berikutnya setengah, +4 MP).");
    } else if (action.skill) {
        action.skill->use(*actor, action.targets, log_);
    }
    updateResult();
    ++turnIndex_;
    beginNextTurn();
}

void Battle::updateResult() {
    const bool heroesAlive =
        std::any_of(heroes_.begin(), heroes_.end(), [](Combatant* c) { return c->isAlive(); });
    const bool enemiesAlive =
        std::any_of(enemies_.begin(), enemies_.end(), [](Combatant* c) { return c->isAlive(); });
    if (!enemiesAlive && result_ == Result::Ongoing) {
        result_ = Result::Victory;
        log_.add("Kemenangan!");
    } else if (!heroesAlive && result_ == Result::Ongoing) {
        result_ = Result::Defeat;
        log_.add("Party tumbang...");
    }
}
