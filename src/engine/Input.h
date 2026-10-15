#pragma once

// Pemetaan tombol di satu tempat. Kalau kontrol mau diganti, cukup ubah Input.cpp.
enum class Direction { None, Up, Down, Left, Right };

namespace input {
bool confirm();     // Z, Enter, Space
bool cancel();      // X, Esc, Backspace
bool upPressed();   // termasuk auto-repeat saat ditahan, untuk menu
bool downPressed();
bool leftPressed();
bool rightPressed();
Direction heldDirection();   // untuk berjalan di peta (panah atau WASD)
}  // namespace input
