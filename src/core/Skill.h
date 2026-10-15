#pragma once
#include <string>
#include <vector>

class Combatant;
class BattleLog;

enum class TargetType { SingleEnemy, AllEnemies, SingleAlly, AllAllies, Self };

// ABSTRACTION + POLYMORPHISM: kelas dasar abstrak untuk semua skill.
//
// Pemakai skill (Battle, UI, AI monster) cukup memanggil use(). Detail pengecekan MP,
// pemotongan MP, validasi target, dan efek spesifik disembunyikan di dalam.
// use() memakai pola Template Method: alurnya tetap, bagian yang berbeda (apply,
// isValidTarget) di-override oleh kelas turunan.
class Skill {
public:
    Skill(std::string name, std::string description, int mpCost, TargetType targetType);
    virtual ~Skill() = default;

    const std::string& getName() const { return name_; }
    const std::string& getDescription() const { return description_; }
    int getMpCost() const { return mpCost_; }
    TargetType getTargetType() const { return targetType_; }

    bool targetsAllies() const;
    bool targetsMultiple() const;
    bool canUse(const Combatant& user) const;

    // Default: target harus masih hidup. ReviveSkill meng-override aturan ini.
    virtual bool isValidTarget(const Combatant& user, const Combatant& target) const;

    // Antarmuka utama. Mengembalikan false jika skill gagal dipakai (misal MP kurang).
    bool use(Combatant& user, const std::vector<Combatant*>& targets, BattleLog& log) const;

protected:
    // Efek spesifik tiap jenis skill pada satu target.
    virtual void apply(Combatant& user, Combatant& target, BattleLog& log) const = 0;

private:
    std::string name_;
    std::string description_;
    int mpCost_;
    TargetType targetType_;
};
