#pragma once
#include <string>
#include <vector>

class Combatant;

// Event visual (angka damage/heal melayang). Diisi oleh logika, dibaca oleh tampilan.
struct BattleEvent {
    enum class Type { Damage, Heal, Revive, BuffUp, BuffDown, MpRestore };
    const Combatant* target = nullptr;
    int amount = 0;
    Type type = Type::Damage;
};

// Catatan pertarungan. Encapsulation: luar kelas hanya bisa menambah atau membaca,
// tidak bisa menghapus atau mengubah baris lama.
class BattleLog {
public:
    void add(const std::string& line);
    void addEvent(const BattleEvent& event);

    const std::vector<std::string>& lines() const { return lines_; }
    std::vector<std::string> lastLines(std::size_t count) const;

    // Mengambil sekaligus mengosongkan event yang belum ditampilkan.
    std::vector<BattleEvent> takeEvents();

private:
    static constexpr std::size_t kMaxLines = 200;
    std::vector<std::string> lines_;
    std::vector<BattleEvent> pendingEvents_;
};
