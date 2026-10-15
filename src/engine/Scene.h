#pragma once

class SceneManager;

// ABSTRACTION: satu "layar" game (judul, peta, dialog, battle, ending).
// Loop utama hanya tahu Scene, tidak tahu isinya. Setiap scene konkret meng-override
// update() dan draw(), sehingga loop utama cukup memanggil keduanya (POLYMORPHISM).
class Scene {
public:
    virtual ~Scene() = default;

    virtual void onEnter() {}
    virtual void update(float dt) = 0;
    virtual void draw() const = 0;

    // Overlay (misal dialog) digambar di atas scene di bawahnya, bukan menggantikannya.
    virtual bool isOverlay() const { return false; }

protected:
    SceneManager& scenes() const { return *manager_; }

private:
    friend class SceneManager;
    SceneManager* manager_ = nullptr;
};
