#include "Input.h"

#include "raylib.h"

namespace {
bool pressedOrRepeat(int key) { return IsKeyPressed(key) || IsKeyPressedRepeat(key); }
}  // namespace

namespace input {
bool confirm() { return IsKeyPressed(KEY_Z) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE); }
bool cancel() { return IsKeyPressed(KEY_X) || IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE); }
bool upPressed() { return pressedOrRepeat(KEY_UP) || pressedOrRepeat(KEY_W); }
bool downPressed() { return pressedOrRepeat(KEY_DOWN) || pressedOrRepeat(KEY_S); }
bool leftPressed() { return pressedOrRepeat(KEY_LEFT) || pressedOrRepeat(KEY_A); }
bool rightPressed() { return pressedOrRepeat(KEY_RIGHT) || pressedOrRepeat(KEY_D); }

Direction heldDirection() {
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) return Direction::Up;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) return Direction::Down;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) return Direction::Left;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) return Direction::Right;
    return Direction::None;
}
}  // namespace input
