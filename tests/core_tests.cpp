// Uji logika inti tanpa grafis. Jalankan: ./aethel_tests
#include <cstdio>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#include "core/Battle.h"
#include "core/Party.h"
#include "core/Skills.h"
#include "core/TileMap.h"
#include "core/WorldData.h"

namespace {
int g_failed = 0;
int g_passed = 0;

void check(bool condition, const std::string& name) {
    if (condition) {
        ++g_passed;
        std::printf("  [OK]   %s\n", name.c_str());
    } else {
        ++g_failed;
        std::printf("  [FAIL] %s\n", name.c_str());
    }
}

// ---------- Encapsulation ----------
void testEncapsulation() {
    std::puts("Encapsulation");
    Swordswoman aria("Aria");
    const int maxHp = aria.getMaxHp();

    aria.takeDamage(99999);
    check(aria.getHp() == 0, "HP tidak pernah negatif");
    check(!aria.isAlive(), "HP 0 berarti pingsan");
    check(aria.heal(50) == 0 && aria.getHp() == 0, "karakter pingsan tidak bisa di-heal");
    check(aria.revive(maxHp * 10) && aria.getHp() == maxHp, "revive dibatasi max HP");
    check(!aria.revive(10), "revive ditolak jika masih hidup");

    aria.heal(9999);
    check(aria.getHp() == maxHp, "heal tidak melebihi max HP");

    const int mp = aria.getMp();
    check(!aria.spendMp(mp + 1) && aria.getMp() == mp, "spendMp gagal jika MP kurang, MP utuh");

    const int baseAtk = aria.getAttack();
    aria.applyBuff(Stat::Attack, 30, 3);
    aria.applyBuff(Stat::Attack, 30, 3);
    check(aria.getAttack() == baseAtk * 130 / 100, "buff searah tidak menumpuk");
    aria.applyBuff(Stat::Attack, 500, 3);
    check(aria.buffPercent(Stat::Attack) <= 100, "besar buff dibatasi");

    Party party = Party::createStartingParty();
    check(!party.add(std::make_unique<Mage>("Extra")), "party maksimal 3 anggota");
}

// ---------- Static polymorphism (overloading) + elemen ----------
void testDamageOverloads() {
    std::puts("Static polymorphism: takeDamage overload");
    FrostGolem golem;
    const int before = golem.getHp();
    golem.takeDamage(10);
    check(golem.getHp() == before - 10, "takeDamage(int) memberi damage murni");

    FrostGolem g2, g3;
    Attack fire{50, Element::Fire, false};
    Attack ice{50, Element::Ice, false};
    const int fireDmg = g2.takeDamage(fire);
    const int iceDmg = g3.takeDamage(ice);
    check(fireDmg > iceDmg, "Fire efektif ke Golem (Ice), Ice ditahan");

    Swordswoman a("A");
    a.defend();
    const int halved = a.takeDamage(40);
    check(halved == 20, "bertahan membagi dua damage");
}

// ---------- Dynamic polymorphism ----------
void testPolymorphism() {
    std::puts("Dynamic polymorphism");
    Party party = Party::createStartingParty();
    std::vector<std::string> titles;
    for (Combatant* c : party.asCombatants()) titles.push_back(c->getTitle());
    check(titles == std::vector<std::string>{"Swordswoman", "Mage", "Priestess"},
          "getTitle() berbeda per kelas lewat pointer Combatant*");

    // Pasif berbeda lewat onTurnStart() yang sama.
    BattleLog log;
    Mage& lumi = static_cast<Mage&>(party.member(1));
    lumi.spendMp(20);
    const int mpBefore = lumi.getMp();
    Combatant& asBase = lumi;
    asBase.onTurnStart(log);
    check(lumi.getMp() == mpBefore + 4, "Mage::onTurnStart memulihkan MP");

    // ReviveSkill meng-override isValidTarget.
    Priestess sera("Sera");
    Swordswoman down("Down");
    down.takeDamage(9999);
    const Skill* revive = nullptr;
    const Skill* heal = nullptr;
    for (const Skill* s : sera.getSkills()) {
        if (s->getName() == "Revive") revive = s;
        if (s->getName() == "Heal") heal = s;
    }
    check(revive && revive->isValidTarget(sera, down), "Revive: target pingsan valid");
    check(heal && !heal->isValidTarget(sera, down), "Heal: target pingsan tidak valid");

    // Golem punya pola 3 langkah.
    FrostGolem golem;
    std::mt19937 rng(1);
    Swordswoman t("T");
    std::vector<Combatant*> opp{&t};
    std::vector<Combatant*> allies{&golem};
    golem.onBattleStart();
    const auto a1 = golem.decideAction(opp, allies, rng);
    const auto a2 = golem.decideAction(opp, allies, rng);
    const auto a3 = golem.decideAction(opp, allies, rng);
    check(a1.skill->getName() == "Attack" && a2.skill->getName() == "Gather Frost" &&
              a3.skill->getName() == "Glacier Smash",
          "FrostGolem::decideAction mengikuti pola");
}

// ---------- Aturan Battle ----------
void testBattleRules() {
    std::puts("Battle (abstraction + validasi)");
    Party party = Party::createStartingParty();
    std::vector<std::unique_ptr<Monster>> enemies;
    enemies.push_back(std::make_unique<Slime>());
    Battle battle(party.asCombatants(), std::move(enemies), 7);

    check(battle.isWaitingForPlayer(), "heroine tercepat mendapat giliran pertama");
    Combatant* actor = battle.getCurrentActor();
    Combatant* other = actor == &party.member(0) ? &party.member(1) : &party.member(0);
    const Skill* atk = &other->basicAttack();
    check(!battle.submitPlayerAction(Action::useSkill(other, atk, {battle.getEnemies()[0]})),
          "aksi ditolak jika bukan gilirannya");
    check(!battle.submitPlayerAction(Action::useSkill(actor, &actor->basicAttack(), {&party.member(0)})),
          "serangan ke sekutu ditolak");

    int guard = 0;
    while (!battle.isOver() && guard++ < 100) {
        if (battle.isWaitingForPlayer()) {
            Combatant* a = battle.getCurrentActor();
            const auto targets = battle.getTargetCandidates(*a, a->basicAttack());
            battle.submitPlayerAction(Action::useSkill(a, &a->basicAttack(), {targets[0]}));
        } else {
            battle.runEnemyTurn();
        }
    }
    check(battle.getResult() == Battle::Result::Victory, "party mengalahkan slime");
}

// ---------- Simulasi: boss bisa dikalahkan dengan strategi wajar ----------
Battle::Result simulateBoss(unsigned seed) {
    Party party = Party::createStartingParty();
    std::vector<std::unique_ptr<Monster>> enemies;
    enemies.push_back(std::make_unique<FallenWitch>());
    Battle battle(party.asCombatants(), std::move(enemies), seed);

    auto findSkill = [](Combatant* c, const std::string& name) -> const Skill* {
        for (const Skill* s : c->getSkills()) if (s->getName() == name) return s;
        return nullptr;
    };
    int guard = 0;
    while (!battle.isOver() && guard++ < 500) {
        if (!battle.isWaitingForPlayer()) { battle.runEnemyTurn(); continue; }
        Combatant* a = battle.getCurrentActor();
        Combatant* boss = battle.getEnemies()[0];
        const Skill* chosen = &a->basicAttack();
        std::vector<Combatant*> targets{boss};

        if (a->getTitle() == "Priestess") {
            const Skill* revive = findSkill(a, "Revive");
            const Skill* heal = findSkill(a, "Heal");
            const auto fallen = battle.getTargetCandidates(*a, *revive);
            Combatant* weakest = nullptr;
            for (Combatant* h : battle.getHeroes())
                if (h->isAlive() && (!weakest || h->getHp() * 100 / h->getMaxHp() <
                                                    weakest->getHp() * 100 / weakest->getMaxHp()))
                    weakest = h;
            if (!fallen.empty() && revive->canUse(*a)) { chosen = revive; targets = {fallen[0]}; }
            else if (weakest && weakest->getHp() * 2 < weakest->getMaxHp() && heal->canUse(*a)) {
                chosen = heal; targets = {weakest};
            } else if (findSkill(a, "Holy Light")->canUse(*a)) chosen = findSkill(a, "Holy Light");
        } else if (a->getTitle() == "Swordswoman") {
            if (findSkill(a, "Flame Slash")->canUse(*a)) chosen = findSkill(a, "Flame Slash");
        } else if (a->getTitle() == "Mage") {
            if (findSkill(a, "Ice Lance")->canUse(*a)) chosen = findSkill(a, "Ice Lance");
        }
        if (!chosen->canUse(*a)) {
            battle.submitPlayerAction(Action::defend(a));
            continue;
        }
        battle.submitPlayerAction(Action::useSkill(a, chosen, targets));
    }
    return battle.getResult();
}

void testBossBalance() {
    std::puts("Keseimbangan boss");
    int wins = 0;
    const int runs = 50;
    for (unsigned s = 1; s <= runs; ++s)
        if (simulateBoss(s) == Battle::Result::Victory) ++wins;
    std::printf("  boss dikalahkan %d/%d simulasi\n", wins, runs);
    check(wins >= runs * 7 / 10, "boss bisa dikalahkan dengan strategi wajar");
}

// ---------- Peta ----------
void testMap() {
    std::puts("TileMap");
    TileMap map(world::mapRows());
    check(map.width() == 30 && map.height() == 17, "ukuran peta 30x17");
    check(!map.isWalkable(-1, 0) && !map.isWalkable(100, 100), "di luar peta tidak bisa dilewati");
    check(map.isWalkable(world::kPlayerStart.x, world::kPlayerStart.y), "posisi awal bisa dipijak");

    // BFS: dari posisi awal, depan kastil harus bisa dicapai (abaikan entitas).
    std::vector<bool> seen(static_cast<std::size_t>(map.width() * map.height()), false);
    std::queue<world::Spot> q;
    q.push(world::kPlayerStart);
    seen[static_cast<std::size_t>(world::kPlayerStart.y * map.width() + world::kPlayerStart.x)] = true;
    const int dx[4] = {1, -1, 0, 0}, dy[4] = {0, 0, 1, -1};
    while (!q.empty()) {
        auto p = q.front(); q.pop();
        for (int i = 0; i < 4; ++i) {
            const int nx = p.x + dx[i], ny = p.y + dy[i];
            if (!map.isWalkable(nx, ny)) continue;
            auto idx = static_cast<std::size_t>(ny * map.width() + nx);
            if (seen[idx]) continue;
            seen[idx] = true;
            q.push({nx, ny});
        }
    }
    auto reach = [&](world::Spot s) { return seen[static_cast<std::size_t>(s.y * map.width() + s.x)]; };
    check(reach(world::kWitch) && reach(world::kGolem) && reach(world::kSlimesA) &&
              reach(world::kSlimesB) && reach(world::kWispPack) && reach(world::kInnkeeper),
          "semua titik penting bisa dicapai");
}
}  // namespace

int main() {
    testEncapsulation();
    testDamageOverloads();
    testPolymorphism();
    testBattleRules();
    testBossBalance();
    testMap();
    std::printf("\n%d lulus, %d gagal\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
