#include "DialogueScene.h"

#include <cmath>

#include "engine/Input.h"
#include "engine/SceneManager.h"
#include "engine/Ui.h"
#include "game/Art.h"

namespace {
constexpr float kCharsPerSecond = 55.0f;
}

DialogueScene::DialogueScene(DialogueScript script, std::function<void()> onFinish)
    : script_(std::move(script)), onFinish_(std::move(onFinish)) {}

bool DialogueScene::lineComplete() const {
    return index_ >= script_.size() || revealed_ >= static_cast<float>(script_[index_].text.size());
}

void DialogueScene::update(float dt) {
    time_ += dt;
    if (finished_) return;
    if (script_.empty()) {
        finished_ = true;
        scenes().pop();
        if (onFinish_) onFinish_();
        return;
    }
    revealed_ += dt * kCharsPerSecond;

    if (input::confirm() || input::cancel()) {
        if (!lineComplete()) {
            revealed_ = static_cast<float>(script_[index_].text.size());
        } else if (++index_ >= script_.size()) {
            finished_ = true;
            scenes().pop();
            if (onFinish_) onFinish_();
        } else {
            revealed_ = 0.0f;
        }
    }
}

void DialogueScene::draw() const {
    if (index_ >= script_.size()) return;
    const DialogueLine& line = script_[index_];
    const float W = static_cast<float>(GetScreenWidth());
    const float H = static_cast<float>(GetScreenHeight());

    DrawRectangle(0, 0, static_cast<int>(W), static_cast<int>(H), {0, 0, 0, 90});

    if (line.portrait) {
        const Rectangle area = line.portraitOnRight ? Rectangle{W - 330, H - 470, 300, 340}
                                                    : Rectangle{30, H - 470, 300, 340};
        art::drawPortrait(*line.portrait, area, time_, line.portraitOnRight);
    }

    const Rectangle box{24, H - 160, W - 48, 140};
    ui::panel(box);
    if (!line.speaker.empty()) {
        const float nameW = static_cast<float>(ui::textWidth(line.speaker, 22)) + 28;
        const float nx = line.portraitOnRight ? box.x + box.width - nameW - 20 : box.x + 20;
        const Rectangle nameBox{nx, box.y - 20, nameW, 36};
        DrawRectangleRounded(nameBox, 0.3f, 6, ui::kPanelLight);
        DrawRectangleRoundedLinesEx(nameBox, 0.3f, 6, 2.0f, ui::kBorder);
        ui::text(line.speaker, nameBox.x + 14, nameBox.y + 7, 22, ui::kAccent);
    }
    const Color textColor = line.speaker.empty() ? Color{210, 215, 235, 255} : ui::kText;
    ui::textWrapped(line.text, {box.x + 26, box.y + 32, box.width - 52, box.height - 40}, 22, textColor,
                    static_cast<std::size_t>(revealed_));
    if (lineComplete()) ui::cursor(box.x + box.width - 34, box.y + box.height - 22, time_);
}
