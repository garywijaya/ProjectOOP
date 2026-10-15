#pragma once
#include <optional>
#include <string>
#include <vector>

#include "Appearance.h"

// Satu baris dialog ala visual novel.
struct DialogueLine {
    std::string speaker;
    std::string text;
    std::optional<Appearance> portrait;   // kosong = narasi tanpa gambar
    bool portraitOnRight = false;
};

using DialogueScript = std::vector<DialogueLine>;
