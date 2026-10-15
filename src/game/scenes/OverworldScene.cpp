#include "OverworldScene.h"

#include <algorithm>

#include "BattleScene.h"
#include "DialogueScene.h"
#include "EndingScene.h"
#include "engine/Input.h"
#include "engine/SceneManager.h"
#include "engine/Ui.h"
#include "game/Story.h"

namespace {
template <typename T>
std::vector<std::unique_ptr<Monster>> make(int count) {
    std::vector<std::unique_ptr<Monster>> out;
    for (int i = 0; i < count; ++i) {
        const std::string suffix = count > 1 ? std::string(" ") + static_cast<char>('A' + i) : "";
        out.push_back(std::make_unique<T>(T().getName() + suffix));
    }
    return out;
}
}  // namespace

OverworldScene::OverworldScene()
    : party_(Party::createStartingParty()), map_(world::mapRows()) {
    player_ = std::make_unique<Player>(world::kPlayerStart.x, world::kPlayerStart.y,
                                       party_.member(0).getAppearance());
    buildWorld();
}

void OverworldScene::buildWorld() {
    using world::Spot;
    auto add = [this](std::unique_ptr<MapEntity> e) { entities_.push_back(std::move(e)); };

    add(std::make_unique<Npc>(world::kGuildGirl.x, world::kGuildGirl.y, "Elise", story::eliseLook(),
                              story::guildGirl(party_)));
    add(std::make_unique<Innkeeper>(world::kInnkeeper.x, world::kInnkeeper.y, "Momo", story::momoLook(),
                                    story::innGreeting(), story::innRested(party_)));
    add(std::make_unique<Signpost>(world::kSignpost.x, world::kSignpost.y, story::signpost()));

    add(std::make_unique<EnemySymbol>(world::kSlimesA.x, world::kSlimesA.y, "Slime x2", Slime().getAppearance(),
                                      [] { return make<Slime>(2); }, Backdrop::Field));
    add(std::make_unique<EnemySymbol>(world::kSlimesB.x, world::kSlimesB.y, "Slime x3", Slime().getAppearance(),
                                      [] { return make<Slime>(3); }, Backdrop::Field));
    add(std::make_unique<EnemySymbol>(world::kWispPack.x, world::kWispPack.y, "Wisp + Slime",
                                      EmberWisp().getAppearance(), [] {
                                          auto list = make<EmberWisp>(1);
                                          list.push_back(std::make_unique<Slime>());
                                          list.push_back(std::make_unique<EmberWisp>("Ember Wisp B"));
                                          return list;
                                      },
                                      Backdrop::Field));

    auto golem = std::make_unique<EnemySymbol>(world::kGolem.x, world::kGolem.y, "Frost Golem",
                                               FrostGolem().getAppearance(), [] { return make<FrostGolem>(1); },
                                               Backdrop::Bridge, story::golemIntro(party_));
    golem_ = golem.get();
    add(std::move(golem));

    add(std::make_unique<EnemySymbol>(
        world::kWitch.x, world::kWitch.y, "Noctyra", story::witchLook(), [] { return make<FallenWitch>(1); },
        Backdrop::Castle, story::witchIntro(party_), [this](WorldServices& w) {
            w.showDialogue(story::ending(party_), [this] { scenes().replaceAll(std::make_unique<EndingScene>(true)); });
        }));
}

void OverworldScene::onEnter() {
    if (!introShown_) {
        introShown_ = true;
        showDialogue(story::intro(party_));
    }
}

MapEntity* OverworldScene::entityAt(int x, int y) const {
    for (const auto& e : entities_)
        if (e->occupies(x, y)) return e.get();
    return nullptr;
}

void OverworldScene::showDialogue(DialogueScript script, std::function<void()> onFinish) {
    scenes().push(std::make_unique<DialogueScene>(std::move(script), std::move(onFinish)));
}

void OverworldScene::startBattle(std::vector<std::unique_ptr<Monster>> enemies, Backdrop backdrop,
                                 std::function<void()> onVictory) {
    scenes().push(std::make_unique<BattleScene>(
        party_, std::move(enemies), backdrop, [this, onVictory](BattleScene::Outcome outcome) {
            if (outcome == BattleScene::Outcome::Victory) {
                party_.reviveFallenAfterVictory();
                if (onVictory) onVictory();
            } else {
                scenes().replaceAll(std::make_unique<EndingScene>(false));
            }
        }));
}

void OverworldScene::update(float dt) {
    time_ += dt;
    for (auto& e : entities_) e->update(dt);
    player_->update(dt);
    entities_.erase(std::remove_if(entities_.begin(), entities_.end(),
                                   [this](const std::unique_ptr<MapEntity>& e) {
                                       if (e->isRemoved() && e.get() == golem_) golem_ = nullptr;
                                       return e->isRemoved();
                                   }),
                    entities_.end());

    if (player_->isMoving()) return;

    if (input::confirm()) {
        int fx = 0, fy = 0;
        player_->facedTile(fx, fy);
        if (MapEntity* e = entityAt(fx, fy)) {
            e->onInteract(*this);   // polymorphism: NPC bicara, penginapan memulihkan, musuh bertarung
            return;
        }
    }

    const Direction dir = input::heldDirection();
    if (dir == Direction::None) return;
    player_->face(dir);
    int tx = 0, ty = 0;
    player_->facedTile(tx, ty);
    if (MapEntity* e = entityAt(tx, ty)) {
        e->onBump(*this);
        return;
    }
    if (map_.isWalkable(tx, ty)) player_->startMove(dir);
}

std::string OverworldScene::objective() const {
    if (golem_) return "Tujuan: kalahkan Frost Golem yang menjaga jembatan utara";
    return "Tujuan: masuki Kastil Gerhana dan hadapi Noctyra";
}

void OverworldScene::draw() const {
    ClearBackground({40, 90, 50, 255});
    for (int y = 0; y < map_.height(); ++y)
        for (int x = 0; x < map_.width(); ++x) art::drawTile(map_.at(x, y), x, y, kTileSize, time_);

    // Gambar dari atas ke bawah supaya yang lebih dekat menutupi yang jauh.
    std::vector<const MapEntity*> order;
    for (const auto& e : entities_) order.push_back(e.get());
    order.push_back(player_.get());
    std::stable_sort(order.begin(), order.end(),
                     [](const MapEntity* a, const MapEntity* b) { return a->tileY() < b->tileY(); });
    for (const MapEntity* e : order) e->draw();   // polymorphism: tiap entitas menggambar dirinya

    drawHud();
}

void OverworldScene::drawHud() const {
    const float W = static_cast<float>(GetScreenWidth());
    const std::string goal = objective();
    const float gw = static_cast<float>(ui::textWidth(goal, 16)) + 24;
    DrawRectangleRounded({W / 2 - gw / 2, 6, gw, 26}, 0.4f, 6, {0, 0, 0, 150});
    ui::textCentered(goal, W / 2, 11, 16, ui::kAccent);

    for (std::size_t i = 0; i < party_.size(); ++i) {
        const Heroine& h = party_.member(i);
        const float x = 8.0f + static_cast<float>(i) * 128.0f;
        const float y = static_cast<float>(GetScreenHeight()) - 44.0f;
        DrawRectangleRounded({x, y, 122, 38}, 0.3f, 6, {0, 0, 0, 150});
        ui::text(h.getName(), x + 8, y + 4, 12, h.isAlive() ? ui::kText : ui::kHpLow);
        ui::bar({x + 8, y + 19, 106, 6}, static_cast<float>(h.getHp()) / h.getMaxHp(), ui::kHp);
        ui::bar({x + 8, y + 28, 106, 5}, static_cast<float>(h.getMp()) / std::max(1, h.getMaxMp()), ui::kMp);
    }
}
