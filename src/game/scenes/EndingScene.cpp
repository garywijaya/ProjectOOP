#include "EndingScene.h"

#include <cmath>
#include <memory>

#include "TitleScene.h"
#include "engine/Input.h"
#include "engine/SceneManager.h"
#include "engine/Ui.h"

void EndingScene::update(float dt) {
    time_ += dt;
    if (time_ > 1.0f && input::confirm()) scenes().replaceAll(std::make_unique<TitleScene>());
}

void EndingScene::draw() const {
    const int W = GetScreenWidth();
    const int H = GetScreenHeight();
    if (victory_) {
        DrawRectangleGradientV(0, 0, W, H, {255, 190, 120, 255}, {120, 170, 240, 255});
        DrawCircle(W / 2, H + 40, 220.0f + std::sin(time_) * 6.0f, {255, 235, 170, 255});
        ui::textCentered("TAMAT", W / 2.0f, 120, 72, RAYWHITE);
        ui::textCentered("Matahari kembali terbit di Aethel.", W / 2.0f, 210, 24, RAYWHITE);
        ui::textCentered("Terima kasih telah bermain Aethel Chronicles!", W / 2.0f, 250, 20, RAYWHITE);
    } else {
        DrawRectangleGradientV(0, 0, W, H, {20, 10, 30, 255}, {60, 20, 40, 255});
        ui::textCentered("GAME OVER", W / 2.0f, 150, 72, ui::kHpLow);
        ui::textCentered("Gerhana masih menutupi Aethel...", W / 2.0f, 240, 24, ui::kText);
    }
    if (time_ > 1.0f) ui::textCentered("[Z] kembali ke judul", W / 2.0f, static_cast<float>(H) - 80, 20, ui::kText);
}
