#include "Application.h"

#include <algorithm>

#include "raylib.h"

Application::Application(int width, int height, const std::string& title) {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(width, height, title.c_str());
    SetExitKey(KEY_NULL);   // ESC dipakai sebagai tombol "kembali", bukan keluar
    SetTargetFPS(60);
}

Application::~Application() { CloseWindow(); }

void Application::run(std::unique_ptr<Scene> firstScene) {
    scenes_.replaceAll(std::move(firstScene));
    scenes_.applyPending();
    while (!WindowShouldClose() && !scenes_.empty()) {
        const float dt = std::min(GetFrameTime(), 1.0f / 20.0f);
        scenes_.update(dt);
        scenes_.applyPending();

        BeginDrawing();
        ClearBackground(BLACK);
        scenes_.draw();
        EndDrawing();
    }
}
