#pragma once
#include "engine/Scene.h"

// Layar akhir: tamat (menang) atau game over (kalah). Z kembali ke judul.
class EndingScene : public Scene {
public:
    explicit EndingScene(bool victory) : victory_(victory) {}
    void update(float dt) override;
    void draw() const override;

private:
    bool victory_;
    float time_ = 0.0f;
};
