cmake_minimum_required(VERSION 3.16)
project(AethelChronicles LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ---------- Core logic (tanpa grafis) ----------
add_library(aethel_core STATIC
    src/core/Element.cpp
    src/core/BattleLog.cpp
    src/core/Combatant.cpp
    src/core/Skill.cpp
    src/core/Skills.cpp
    src/core/Heroines.cpp
    src/core/Monsters.cpp
    src/core/Battle.cpp
    src/core/Party.cpp
    src/core/TileMap.cpp
)
target_include_directories(aethel_core PUBLIC src)

add_executable(aethel_tests tests/core_tests.cpp)
target_link_libraries(aethel_tests PRIVATE aethel_core)

# ---------- raylib (otomatis diunduh jika belum terpasang) ----------
find_package(raylib 5.5 QUIET)
if(NOT raylib_FOUND)
    include(FetchContent)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(raylib
        URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz)
    FetchContent_MakeAvailable(raylib)
endif()

# ---------- Game ----------
add_executable(AethelChronicles
    src/main.cpp
    src/engine/SceneManager.cpp
    src/engine/Application.cpp
    src/engine/Input.cpp
    src/engine/Ui.cpp
    src/game/Art.cpp
    src/game/MapEntities.cpp
    src/game/Story.cpp
    src/game/scenes/TitleScene.cpp
    src/game/scenes/OverworldScene.cpp
    src/game/scenes/DialogueScene.cpp
    src/game/scenes/BattleScene.cpp
    src/game/scenes/EndingScene.cpp
)
target_link_libraries(AethelChronicles PRIVATE aethel_core raylib)
if(WIN32)
    target_link_libraries(AethelChronicles PRIVATE winmm)
endif()
if(MINGW)
    target_link_options(AethelChronicles PRIVATE -static -static-libgcc -static-libstdc++)
endif()
