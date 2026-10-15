#include "TitleScene.h"

#include <cmath>
#include <memory>

#include "OverworldScene.h"
#include "engine/Input.h"
#include "engine/SceneManager.h"
#include "engine/Ui.h"
#include "game/Art.h"

TitleScene::TitleScene() : preview_(Party::createStartingParty()) {}

void TitleScene::update(float dt) {
    time_ += dt;
    if (input::upPressed() || input::downPressed()) choice_ = 1 - choice_;
    if (!input::confirm()) return;
    if (choice_ == 0) scenes().replaceAll(std::make_unique<OverworldScene>());
    else scenes().pop();   // stack kosong = keluar dari game
}

void TitleScene::draw() const {
    const int W = GetScreenWidth();
    const int H = GetScreenHeight();
    DrawRectangleGradientV(0, 0, W, H, {16, 12, 40, 255}, {90, 50, 120, 255});
    for (int i = 0; i < 60; ++i) {
        const float x = static_cast<float>((i * 157) % W);
        const float y = static_cast<float>((i * 89) % 300);
        const float tw = 0.5f + 0.5f * std::sin(time_ * 2.0f + static_cast<float>(i));
        DrawCircleV({x, y}, 1.5f, {255, 255, 255, static_cast<unsigned char>(80 + 150 * tw)});
    }
    DrawCircle(W - 150, 110, 60, {255, 210, 150, 255});
    DrawCircle(W - 136, 102, 58, {30, 20, 60, 255});

    ui::textCentered("AETHEL CHRONICLES", W / 2.0f, 60, 56, ui::kAccent);
    ui::textCentered("Heroines of the Eclipse", W / 2.0f, 122, 24, ui::kText);

    for (std::size_t i = 0; i < preview_.size(); ++i) {
        const float x = W / 2.0f + (static_cast<float>(i) - 1.0f) * 150.0f;
        art::drawCharacter(preview_.member(i).getAppearance(), {x, 330}, 2.4f, time_ + static_cast<float>(i),
                           art::Facing::Down, false);
        ui::textCentered(preview_.member(i).getName(), x, 345, 18, ui::kText);
        ui::textCentered(preview_.member(i).getTitle(), x, 366, 14, ui::kMuted);
    }

    const char* items[] = {"Mulai Petualangan", "Keluar"};
    for (int i = 0; i < 2; ++i) {
        const float y = 412.0f + static_cast<float>(i) * 34.0f;
        ui::textCentered(items[i], W / 2.0f, y, 24, i == choice_ ? ui::kAccent : ui::kText);
        if (i == choice_) ui::cursor(W / 2.0f - 140, y + 12, time_);
    }
    ui::textCentered("Panah/WASD: gerak   Z/Enter: pilih   X/Esc: kembali", W / 2.0f,
                     static_cast<float>(H) - 30, 14, ui::kMuted);
}
