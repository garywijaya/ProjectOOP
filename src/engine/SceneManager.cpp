#include "SceneManager.h"

void SceneManager::push(std::unique_ptr<Scene> scene) {
    if (scene) pending_.push_back({Operation::Kind::Push, std::move(scene)});
}

void SceneManager::pop() { pending_.push_back({Operation::Kind::Pop, nullptr}); }

void SceneManager::replaceAll(std::unique_ptr<Scene> scene) {
    if (scene) pending_.push_back({Operation::Kind::ReplaceAll, std::move(scene)});
}

void SceneManager::applyPending() {
    // Operasi bisa menambah operasi baru (lewat onEnter), jadi proses sampai habis.
    while (!pending_.empty()) {
        std::vector<Operation> ops;
        ops.swap(pending_);
        for (Operation& op : ops) {
            switch (op.kind) {
                case Operation::Kind::Push:
                    op.scene->manager_ = this;
                    stack_.push_back(std::move(op.scene));
                    stack_.back()->onEnter();
                    break;
                case Operation::Kind::Pop:
                    if (!stack_.empty()) stack_.pop_back();
                    break;
                case Operation::Kind::ReplaceAll:
                    stack_.clear();
                    op.scene->manager_ = this;
                    stack_.push_back(std::move(op.scene));
                    stack_.back()->onEnter();
                    break;
            }
        }
    }
}

void SceneManager::update(float dt) {
    if (!stack_.empty()) stack_.back()->update(dt);   // hanya scene teratas yang aktif
}

void SceneManager::draw() const {
    if (stack_.empty()) return;
    std::size_t first = stack_.size() - 1;
    while (first > 0 && stack_[first]->isOverlay()) --first;
    for (std::size_t i = first; i < stack_.size(); ++i) stack_[i]->draw();
}
