#pragma once

// Objek apa pun yang hidup di dalam scene: punya update per frame dan bisa digambar.
class GameObject {
public:
    virtual ~GameObject() = default;
    virtual void update(float /*dt*/) {}
    virtual void draw() const = 0;
};
