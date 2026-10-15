#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Appearance.h"
#include "BattleLog.h"
#include "Element.h"
#include "Skill.h"

enum class Stat { Attack, Defense, Speed };

// Kelas dasar ABSTRAK untuk semua yang bisa bertarung (heroine maupun monster).
//
// ENCAPSULATION: HP, MP, buff, dan status bertahan disimpan private.
// Satu-satunya cara mengubahnya adalah lewat takeDamage(), heal(), spendMp(), revive(),
// dan applyBuff(), yang semuanya menjaga aturan (HP tidak negatif, tidak melebihi max,
// karakter yang pingsan tidak bisa di-heal, buff tidak bisa ditumpuk tanpa batas).
class Combatant {
public:
    struct BaseStats {
        int maxHp = 1;
        int maxMp = 0;
        int attack = 1;
        int defense = 0;
        int speed = 1;
    };

    Combatant(std::string name, BaseStats stats, Element element, Appearance look);
    virtual ~Combatant() = default;

    Combatant(const Combatant&) = delete;
    Combatant& operator=(const Combatant&) = delete;

    // ---------- Akses baca (tidak bisa mengubah state) ----------
    const std::string& getName() const { return name_; }
    int getHp() const { return hp_; }
    int getMaxHp() const { return stats_.maxHp; }
    int getMp() const { return mp_; }
    int getMaxMp() const { return stats_.maxMp; }
    int getAttack() const;   // sudah termasuk efek buff
    int getDefense() const;
    int getSpeed() const;
    Element getElement() const { return element_; }
    const Appearance& getAppearance() const { return look_; }
    bool isAlive() const { return hp_ > 0; }
    bool isDefending() const { return defending_; }
    int buffPercent(Stat stat) const;

    // ---------- Operasi terkontrol (aturan dijaga di sini) ----------
    // STATIC POLYMORPHISM (overloading): dua cara menerima damage.
    int takeDamage(int rawAmount);              // damage murni, tanpa elemen
    int takeDamage(const Attack& attack);       // dihitung dengan elemen dan defense

    int heal(int amount);                       // gagal (0) jika sudah pingsan
    bool revive(int hpAmount);                  // hanya berhasil jika sedang pingsan
    bool spendMp(int cost);                     // gagal jika MP kurang
    int restoreMp(int amount);
    void applyBuff(Stat stat, int percent, int turns);
    void defend();
    void fullRestore();

    // ---------- Skill ----------
    const Skill& basicAttack() const { return *basicAttack_; }
    std::vector<const Skill*> getSkills() const;

    // ---------- Titik polymorphism ----------
    virtual std::string getTitle() const = 0;          // "Swordswoman", "Slime", ...
    virtual bool isPlayerControlled() const = 0;
    // Dipanggil di awal giliran. Kelas turunan menambahkan pasif masing-masing,
    // lalu memanggil versi dasar (yang mengurus durasi buff dan status bertahan).
    virtual void onTurnStart(BattleLog& log);
    // Dipanggil sekali saat pertarungan dimulai, untuk mereset state khusus per battle.
    virtual void onBattleStart() {}

protected:
    void addSkill(std::unique_ptr<Skill> skill);

private:
    struct Buff {
        Stat stat;
        int percent;
        int turnsLeft;
    };

    int applyModifier(int base, Stat stat) const;

    std::string name_;
    BaseStats stats_;
    Element element_;
    Appearance look_;
    int hp_;
    int mp_;
    bool defending_ = false;
    std::vector<Buff> buffs_;
    std::unique_ptr<Skill> basicAttack_;
    std::vector<std::unique_ptr<Skill>> skills_;
};
