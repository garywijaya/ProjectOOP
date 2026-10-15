#include "BattleScene.h"

#include <algorithm>
#include <cmath>
#include <random>

#include "engine/Input.h"
#include "engine/SceneManager.h"
#include "engine/Ui.h"
#include "game/Art.h"

namespace {
const char* kCommands[] = {"Serang", "Skill", "Bertahan"};
constexpr int kCommandCount = 3;

unsigned freshSeed() {
    std::random_device rd;
    return rd();
}
}  // namespace

BattleScene::BattleScene(Party& party, std::vector<std::unique_ptr<Monster>> enemies, Backdrop backdrop,
                         std::function<void(Outcome)> onFinish)
    : party_(party),
      battle_(party.asCombatants(), std::move(enemies), freshSeed()),
      backdrop_(backdrop),
      onFinish_(std::move(onFinish)) {}

// ------------------------------------------------------------------ alur
void BattleScene::decideNextPhase() {
    collectEvents();
    if (battle_.isOver()) {
        phase_ = Phase::Result;
        timer_ = 0.8f;
    } else if (battle_.isWaitingForPlayer()) {
        phase_ = Phase::Command;
        commandIndex_ = 0;
    } else {
        phase_ = Phase::EnemyTurn;
        timer_ = 0.55f;
    }
}

void BattleScene::submit(const Action& action) {
    if (battle_.submitPlayerAction(action)) {
        collectEvents();
        phase_ = Phase::Animating;
        timer_ = 0.55f;
    }
}

void BattleScene::chooseSkill(const Skill* skill) {
    Combatant* actor = battle_.getCurrentActor();
    if (!skill->canUse(*actor)) return;
    candidates_ = battle_.getTargetCandidates(*actor, *skill);
    if (candidates_.empty()) return;
    if (skill->targetsMultiple() || skill->getTargetType() == TargetType::Self) {
        submit(Action::useSkill(actor, skill, candidates_));
        return;
    }
    pendingSkill_ = skill;
    targetIndex_ = 0;
    phase_ = Phase::TargetSelect;
}

void BattleScene::collectEvents() {
    for (const BattleEvent& e : battle_.getLog().takeEvents()) {
        Vector2 p = positionOf(e.target);
        p.y -= 70.0f * scaleOf(e.target) / 2.0f;
        Popup popup{p, "", WHITE, 1.1f};
        switch (e.type) {
            case BattleEvent::Type::Damage:
                popup.text = std::to_string(e.amount);
                popup.color = {255, 240, 240, 255};
                shake_[e.target] = 0.3f;
                break;
            case BattleEvent::Type::Heal:      popup.text = "+" + std::to_string(e.amount); popup.color = ui::kHp; break;
            case BattleEvent::Type::Revive:    popup.text = "BANGKIT!"; popup.color = ui::kAccent; break;
            case BattleEvent::Type::BuffUp:    popup.text = "UP!"; popup.color = {255, 170, 80, 255}; break;
            case BattleEvent::Type::BuffDown:  popup.text = "DOWN"; popup.color = {200, 130, 255, 255}; break;
            case BattleEvent::Type::MpRestore: popup.text = "+" + std::to_string(e.amount) + " MP"; popup.color = ui::kMp; break;
        }
        if (e.type == BattleEvent::Type::Heal && e.amount == 0) continue;
        popups_.push_back(popup);
    }
}

void BattleScene::update(float dt) {
    time_ += dt;
    for (Popup& p : popups_) {
        p.life -= dt;
        p.pos.y -= 40.0f * dt;
    }
    popups_.erase(std::remove_if(popups_.begin(), popups_.end(), [](const Popup& p) { return p.life <= 0; }),
                  popups_.end());
    for (auto& [who, t] : shake_) t = std::max(0.0f, t - dt);

    switch (phase_) {
        case Phase::Intro:
            timer_ -= dt;
            if (timer_ <= 0 || input::confirm()) decideNextPhase();
            break;
        case Phase::Command:      updateCommand(); break;
        case Phase::SkillSelect:  updateSkillSelect(); break;
        case Phase::TargetSelect: updateTargetSelect(); break;
        case Phase::Animating:
            timer_ -= dt;
            if (timer_ <= 0) decideNextPhase();
            break;
        case Phase::EnemyTurn:
            timer_ -= dt;
            if (timer_ <= 0) {
                battle_.runEnemyTurn();
                collectEvents();
                phase_ = Phase::Animating;
                timer_ = 0.65f;
            }
            break;
        case Phase::Result:
            timer_ -= dt;
            if (!finished_ && timer_ <= 0 && input::confirm()) {
                finished_ = true;
                scenes().pop();
                if (onFinish_)
                    onFinish_(battle_.getResult() == Battle::Result::Victory ? Outcome::Victory : Outcome::Defeat);
            }
            break;
    }
}

void BattleScene::updateCommand() {
    if (input::upPressed()) commandIndex_ = (commandIndex_ + kCommandCount - 1) % kCommandCount;
    if (input::downPressed()) commandIndex_ = (commandIndex_ + 1) % kCommandCount;
    if (!input::confirm()) return;

    Combatant* actor = battle_.getCurrentActor();
    switch (commandIndex_) {
        case 0: chooseSkill(&actor->basicAttack()); break;
        case 1:
            skillIndex_ = 0;
            phase_ = Phase::SkillSelect;
            break;
        case 2: submit(Action::defend(actor)); break;
        default: break;
    }
}

void BattleScene::updateSkillSelect() {
    const auto skills = battle_.getCurrentActor()->getSkills();
    const int n = static_cast<int>(skills.size());
    if (input::cancel()) { phase_ = Phase::Command; return; }
    if (n == 0) return;
    if (input::upPressed()) skillIndex_ = (skillIndex_ + n - 1) % n;
    if (input::downPressed()) skillIndex_ = (skillIndex_ + 1) % n;
    if (input::confirm()) chooseSkill(skills[static_cast<std::size_t>(skillIndex_)]);
}

void BattleScene::updateTargetSelect() {
    const int n = static_cast<int>(candidates_.size());
    if (input::cancel()) {
        phase_ = pendingSkill_ == &battle_.getCurrentActor()->basicAttack() ? Phase::Command : Phase::SkillSelect;
        return;
    }
    if (input::upPressed() || input::leftPressed()) targetIndex_ = (targetIndex_ + n - 1) % n;
    if (input::downPressed() || input::rightPressed()) targetIndex_ = (targetIndex_ + 1) % n;
    if (input::confirm())
        submit(Action::useSkill(battle_.getCurrentActor(), pendingSkill_,
                                {candidates_[static_cast<std::size_t>(targetIndex_)]}));
}

// ------------------------------------------------------------------ posisi
Vector2 BattleScene::positionOf(const Combatant* c) const {
    const auto& heroes = battle_.getHeroes();
    for (std::size_t i = 0; i < heroes.size(); ++i) {
        if (heroes[i] != c) continue;
        const Vector2 slots[3] = {{285, 225}, {215, 295}, {285, 365}};
        Vector2 p = slots[std::min<std::size_t>(i, 2)];
        if (c == battle_.getCurrentActor() && !battle_.isOver()) p.x += 24;
        return p;
    }
    const auto& enemies = battle_.getEnemies();
    const std::size_t n = enemies.size();
    for (std::size_t i = 0; i < n; ++i) {
        if (enemies[i] != c) continue;
        if (n == 1) return {700, 340};
        if (n == 2) return i == 0 ? Vector2{650, 255} : Vector2{790, 345};
        const Vector2 slots[3] = {{640, 230}, {800, 290}, {660, 365}};
        return slots[std::min<std::size_t>(i, 2)];
    }
    return {480, 300};
}

float BattleScene::scaleOf(const Combatant* c) const {
    switch (c->getAppearance().body) {
        case BodyType::Heroine: return c->isPlayerControlled() ? 2.0f : 2.7f;
        case BodyType::Slime:   return 2.2f;
        case BodyType::Wisp:    return 2.0f;
        case BodyType::Golem:   return 1.9f;
    }
    return 2.0f;
}

std::string BattleScene::infoText() const {
    const Combatant* actor = battle_.getCurrentActor();
    if (phase_ == Phase::SkillSelect && actor) {
        const auto skills = actor->getSkills();
        if (!skills.empty()) return skills[static_cast<std::size_t>(skillIndex_)]->getDescription();
    }
    if (phase_ == Phase::TargetSelect && !candidates_.empty())
        return "Pilih target: " + candidates_[static_cast<std::size_t>(targetIndex_)]->getName();
    return "";
}

// ------------------------------------------------------------------ gambar
void BattleScene::draw() const {
    drawBackdrop();
    drawCombatants();
    for (const Popup& p : popups_) {
        const float a = std::clamp(p.life / 0.4f, 0.0f, 1.0f);
        const int w = ui::textWidth(p.text, 28);
        DrawText(p.text.c_str(), static_cast<int>(p.pos.x - w / 2.0f) + 2, static_cast<int>(p.pos.y) + 2, 28,
                 {0, 0, 0, static_cast<unsigned char>(180 * a)});
        DrawText(p.text.c_str(), static_cast<int>(p.pos.x - w / 2.0f), static_cast<int>(p.pos.y), 28,
                 {p.color.r, p.color.g, p.color.b, static_cast<unsigned char>(255 * a)});
    }
    drawTopPanel();
    drawCommandPanel();
    drawStatusPanel();
    if (phase_ == Phase::Result) drawResult();
}

void BattleScene::drawBackdrop() const {
    const int W = GetScreenWidth();
    switch (backdrop_) {
        case Backdrop::Field:
            DrawRectangleGradientV(0, 0, W, 300, {120, 180, 245, 255}, {200, 230, 255, 255});
            DrawEllipse(480, 520, 700, 300, {104, 180, 92, 255});
            DrawEllipse(480, 540, 640, 250, {120, 196, 104, 255});
            DrawCircle(820, 90, 34, {255, 245, 200, 255});
            break;
        case Backdrop::Bridge:
            DrawRectangleGradientV(0, 0, W, 300, {150, 190, 230, 255}, {225, 240, 255, 255});
            DrawRectangle(0, 250, W, 80, {90, 160, 225, 255});
            DrawEllipse(480, 540, 680, 280, {200, 225, 245, 255});
            for (int i = 0; i < 6; ++i)
                DrawLineEx({static_cast<float>(i * 170 + 20), 280}, {static_cast<float>(i * 170 + 90), 280}, 3, {200, 230, 255, 180});
            break;
        case Backdrop::Castle: {
            DrawRectangleGradientV(0, 0, W, 320, {20, 10, 40, 255}, {80, 40, 100, 255});
            DrawCircle(800, 100, 50, {255, 200, 140, 255});
            DrawCircle(812, 94, 48, {25, 12, 45, 255});   // gerhana
            DrawEllipse(480, 540, 700, 280, {70, 58, 96, 255});
            for (int i = 0; i < 7; ++i)
                DrawRectangle(i * 150 + 10, 120, 40, 200, {40, 30, 64, 255});
            break;
        }
    }
}

void BattleScene::drawCombatants() const {
    auto drawOne = [&](const Combatant* c, bool facingLeft) {
        Vector2 p = positionOf(c);
        const auto it = shake_.find(c);
        if (it != shake_.end() && it->second > 0) p.x += std::sin(time_ * 70.0f) * 6.0f;
        const float alpha = c->isAlive() ? 1.0f : 0.25f;
        art::drawCharacter(c->getAppearance(), p, scaleOf(c), time_,
                           facingLeft ? art::Facing::Left : art::Facing::Right, false, alpha);

        if (!c->isPlayerControlled() && c->isAlive()) {
            const float w = 70;
            ui::bar({p.x - w / 2, p.y + 8, w, 7}, static_cast<float>(c->getHp()) / c->getMaxHp(), ui::kHpLow);
            const int tw = ui::textWidth(c->getName(), 14);
            ui::textShadow(c->getName(), p.x - tw / 2.0f, p.y + 18, 14, ui::kText);
        }
        if (c == battle_.getCurrentActor() && !battle_.isOver()) {
            const float top = p.y - 62.0f * scaleOf(c) / 2.0f - 22 + std::sin(time_ * 6) * 4;
            DrawTriangle({p.x - 9, top}, {p.x, top + 12}, {p.x + 9, top}, ui::kAccent);
        }
        if (phase_ == Phase::TargetSelect && !candidates_.empty() &&
            candidates_[static_cast<std::size_t>(targetIndex_)] == c) {
            const float top = p.y - 62.0f * scaleOf(c) / 2.0f - 26 + std::sin(time_ * 8) * 4;
            DrawTriangle({p.x - 11, top}, {p.x, top + 15}, {p.x + 11, top}, {255, 90, 110, 255});
            DrawEllipseLines(static_cast<int>(p.x), static_cast<int>(p.y), 28, 8, {255, 90, 110, 255});
        }
    };
    for (const Combatant* e : battle_.getEnemies()) drawOne(e, true);
    for (const Combatant* h : battle_.getHeroes()) drawOne(h, false);
}

void BattleScene::drawTopPanel() const {
    const Rectangle r{16, 10, static_cast<float>(GetScreenWidth()) - 32, 74};
    ui::panel(r);
    const std::string info = infoText();
    if (!info.empty()) {
        ui::textWrapped(info, {r.x + 16, r.y + 12, r.width - 32, r.height - 16}, 20, ui::kAccent);
        return;
    }
    const auto lines = battle_.getLog().lastLines(3);
    float y = r.y + 8;
    for (const std::string& l : lines) {
        ui::text(l, r.x + 16, y, 18, ui::kText);
        y += 21;
    }
    // Urutan giliran berikutnya
    const auto upcoming = battle_.getUpcomingTurns(4);
    if (!upcoming.empty() && !battle_.isOver()) {
        std::string order = "Giliran: ";
        for (std::size_t i = 0; i < upcoming.size(); ++i) order += (i ? " > " : "") + upcoming[i]->getName();
        const int w = ui::textWidth(order, 14);
        ui::textShadow(order, r.x + r.width - w - 10, r.y + r.height + 6, 14, ui::kMuted);
    }
}

void BattleScene::drawCommandPanel() const {
    const Rectangle r{16, 392, 270, 140};
    ui::panel(r);
    const Combatant* actor = battle_.getCurrentActor();
    if (!actor || !actor->isPlayerControlled() || battle_.isOver()) {
        ui::text(phase_ == Phase::EnemyTurn || phase_ == Phase::Animating ? "..." : "", r.x + 20, r.y + 20, 22, ui::kMuted);
        return;
    }
    ui::text(actor->getName(), r.x + 18, r.y + 10, 18, ui::kAccent);

    if (phase_ == Phase::SkillSelect || (phase_ == Phase::TargetSelect && pendingSkill_ != &actor->basicAttack())) {
        const auto skills = actor->getSkills();
        for (std::size_t i = 0; i < skills.size(); ++i) {
            const Skill* s = skills[i];
            const float y = r.y + 34 + static_cast<float>(i) * 24;
            const Color c = s->canUse(*actor) ? ui::kText : ui::kMuted;
            ui::text(s->getName(), r.x + 40, y, 18, c);
            const std::string cost = std::to_string(s->getMpCost()) + " MP";
            ui::text(cost, r.x + r.width - 18 - ui::textWidth(cost, 16), y + 1, 16, ui::kMp);
            if (static_cast<int>(i) == skillIndex_) ui::cursor(r.x + 22, y + 9, time_);
        }
        return;
    }
    for (int i = 0; i < kCommandCount; ++i) {
        const float y = r.y + 38 + static_cast<float>(i) * 30;
        ui::text(kCommands[i], r.x + 44, y, 22, ui::kText);
        if (i == commandIndex_ && phase_ == Phase::Command) ui::cursor(r.x + 24, y + 11, time_);
    }
}

void BattleScene::drawStatusPanel() const {
    const Rectangle r{300, 392, static_cast<float>(GetScreenWidth()) - 316, 140};
    ui::panel(r);
    const auto& heroes = battle_.getHeroes();
    for (std::size_t i = 0; i < heroes.size(); ++i) {
        const Combatant* h = heroes[i];
        const float y = r.y + 14 + static_cast<float>(i) * 40;
        const bool active = h == battle_.getCurrentActor() && !battle_.isOver();
        if (active) DrawRectangleRounded({r.x + 8, y - 5, r.width - 16, 38}, 0.3f, 6, {255, 255, 255, 22});
        ui::text(h->getName(), r.x + 20, y, 20, h->isAlive() ? (active ? ui::kAccent : ui::kText) : ui::kHpLow);
        ui::text(h->getTitle(), r.x + 20, y + 20, 12, ui::kMuted);

        const float hpRatio = static_cast<float>(h->getHp()) / h->getMaxHp();
        ui::bar({r.x + 160, y + 4, 200, 12}, hpRatio, hpRatio < 0.3f ? ui::kHpLow : ui::kHp);
        ui::text("HP " + std::to_string(h->getHp()) + "/" + std::to_string(h->getMaxHp()), r.x + 160, y + 18, 12, ui::kText);
        ui::bar({r.x + 380, y + 4, 130, 12}, static_cast<float>(h->getMp()) / std::max(1, h->getMaxMp()), ui::kMp);
        ui::text("MP " + std::to_string(h->getMp()) + "/" + std::to_string(h->getMaxMp()), r.x + 380, y + 18, 12, ui::kText);

        std::string status;
        if (h->isDefending()) status += "DEF ";
        if (h->buffPercent(Stat::Attack) > 0) status += "ATK+ ";
        if (h->buffPercent(Stat::Attack) < 0) status += "ATK- ";
        if (h->buffPercent(Stat::Defense) > 0) status += "DEF+ ";
        if (h->buffPercent(Stat::Speed) < 0) status += "SPD- ";
        if (!h->isAlive()) status = "PINGSAN";
        ui::text(status, r.x + 528, y + 6, 14, ui::kAccent);
    }
}

void BattleScene::drawResult() const {
    const int W = GetScreenWidth();
    DrawRectangle(0, 170, W, 150, {0, 0, 0, 170});
    const bool win = battle_.getResult() == Battle::Result::Victory;
    ui::textCentered(win ? "MENANG!" : "PARTY TUMBANG...", W / 2.0f, 195, 56, win ? ui::kAccent : ui::kHpLow);
    std::string sub = "Tekan Z untuk lanjut";
    if (win) {
        for (std::size_t i = 0; i < party_.size(); ++i) {
            if (party_.member(i).isAlive()) {
                sub = party_.member(i).getName() + ": \"" + party_.member(i).getBattleCry() + "\"";
                break;
            }
        }
    }
    ui::textCentered(sub, W / 2.0f, 262, 20, ui::kText);
    if (timer_ <= 0) ui::textCentered("[Z] lanjut", W / 2.0f, 292, 16, ui::kMuted);
}
