#pragma once
#include <functional>

#include "core/Dialogue.h"
#include "engine/Scene.h"

// Dialog ala visual novel: potret karakter, kotak nama, dan efek teks diketik.
class DialogueScene : public Scene {
public:
    DialogueScene(DialogueScript script, std::function<void()> onFinish);

    void update(float dt) override;
    void draw() const override;
    bool isOverlay() const override { return true; }

private:
    bool lineComplete() const;

    DialogueScript script_;
    std::function<void()> onFinish_;
    std::size_t index_ = 0;
    float revealed_ = 0.0f;
    float time_ = 0.0f;
    bool finished_ = false;
};
