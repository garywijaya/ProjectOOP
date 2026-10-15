# Aethel Chronicles - Heroines of the Eclipse

Mini JRPG 2D (C++17 + raylib 5.5) untuk tugas OOP Pillars.

## Build
Butuh CMake 3.16+ dan compiler C++17 (MSVC / MinGW / g++). raylib diunduh otomatis.

    cmake -S . -B build
    cmake --build build --config Release

Jalankan `build/AethelChronicles` (Windows MSVC: `build/Release/AethelChronicles.exe`).
Test logika: `build/aethel_tests`.

## Kontrol
Panah/WASD jalan, Z/Enter bicara & pilih, X/Esc kembali.

## Struktur
- src/core   : logika murni tanpa grafis (Combatant, Heroine, Monster, Skill, Battle, Party, TileMap)
- src/engine : mini engine (Scene, SceneManager, Application, Input, Ui)
- src/game   : peta, entitas, cerita, scene (Title, Overworld, Dialogue, Battle, Ending)
- tests      : test logika inti
# ProjectOOP
# ProjectOOP
