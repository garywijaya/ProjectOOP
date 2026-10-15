#include <memory>

#include "engine/Application.h"
#include "game/scenes/TitleScene.h"

int main() {
    Application app(960, 544, "Aethel Chronicles - Heroines of the Eclipse");
    app.run(std::make_unique<TitleScene>());
    return 0;
}
