#pragma once
#include "core/Party.h"
#include "engine/Scene.h"

class TitleScene : public Scene {
public:
    TitleScene();
    void update(float dt) override;
    void draw() const override;

private:
    Party preview_;   // hanya untuk menampilkan ketiga heroine di layar judul
    int choice_ = 0;
    float time_ = 0.0f;
};
