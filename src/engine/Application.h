#pragma once
#include <memory>
#include <string>

#include "SceneManager.h"

// ABSTRACTION: membungkus pembuatan window, loop 60 FPS, dan penutupan raylib.
// main() cukup: Application app(...); app.run(scene pertama);
class Application {
public:
    Application(int width, int height, const std::string& title);
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    void run(std::unique_ptr<Scene> firstScene);

private:
    SceneManager scenes_;
};
