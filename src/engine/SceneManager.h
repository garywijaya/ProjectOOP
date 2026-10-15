#pragma once
#include <memory>
#include <vector>

#include "Scene.h"

// Tumpukan (stack) scene. Contoh: Overworld di bawah, Dialog di atasnya, lalu Battle.
//
// ENCAPSULATION: stack tidak bisa diakses langsung. Perubahan (push/pop/replace) ditunda
// sampai applyPending() dipanggil di titik aman dalam frame, sehingga scene tidak pernah
// terhapus saat kodenya sendiri masih berjalan (mencegah crash use-after-free).
class SceneManager {
public:
    void push(std::unique_ptr<Scene> scene);
    void pop();
    void replaceAll(std::unique_ptr<Scene> scene);

    void applyPending();
    void update(float dt);
    void draw() const;
    bool empty() const { return stack_.empty() && pending_.empty(); }

private:
    struct Operation {
        enum class Kind { Push, Pop, ReplaceAll };
        Kind kind;
        std::unique_ptr<Scene> scene;
    };

    std::vector<std::unique_ptr<Scene>> stack_;
    std::vector<Operation> pending_;
};
