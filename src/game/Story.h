#pragma once
#include "core/Dialogue.h"
#include "core/Party.h"

// Semua naskah cerita dikumpulkan di sini supaya mudah diedit tanpa menyentuh logika.
namespace story {
Appearance eliseLook();
Appearance momoLook();
Appearance witchLook();

DialogueScript intro(const Party& party);
DialogueScript guildGirl(const Party& party);
DialogueScript innGreeting();
DialogueScript innRested(const Party& party);
DialogueScript signpost();
DialogueScript golemIntro(const Party& party);
DialogueScript witchIntro(const Party& party);
DialogueScript ending(const Party& party);
}  // namespace story
