#include "Story.h"

#include "core/Monsters.h"

namespace story {
namespace {
DialogueLine say(const Heroine& h, std::string text, bool right = false) {
    return {h.getName(), std::move(text), h.getAppearance(), right};
}
DialogueLine say(std::string name, const Appearance& look, std::string text, bool right = true) {
    return {std::move(name), std::move(text), look, right};
}
DialogueLine narrate(std::string text) { return {"", std::move(text), std::nullopt, false}; }
}  // namespace

Appearance eliseLook() {
    Appearance a;
    a.hair = {150, 100, 70};
    a.outfit = {70, 120, 90};
    a.eyes = {110, 80, 50};
    a.hairStyle = HairStyle::Short;
    a.accessory = Accessory::Ribbon;
    return a;
}

Appearance momoLook() {
    Appearance a;
    a.hair = {255, 170, 200};
    a.outfit = {240, 150, 90};
    a.eyes = {200, 90, 140};
    a.hairStyle = HairStyle::Twintails;
    return a;
}

Appearance witchLook() { return FallenWitch().getAppearance(); }

DialogueScript intro(const Party& p) {
    const Heroine& aria = p.member(0);
    const Heroine& lumi = p.member(1);
    const Heroine& sera = p.member(2);
    return {
        narrate("Cahaya putih menyilaukan... Saat membuka mata, kamu berada di dunia lain bernama Aethel."),
        say(aria, "Berhasil! Pemanggilan berhasil! Kamu Tactician yang diramalkan, kan?"),
        say(lumi, "Hmm. Kelihatannya biasa saja. Yakin ritualnya tidak salah?", true),
        say(sera, "Lumi, jangan begitu. Maaf ya, Tactician. Aku Seraphine, ini Aria dan Lumi."),
        say(sera, "Penyihir Noctyra menutup langit Aethel dengan gerhana abadi. Kami butuh strategimu."),
        say(aria, "Golem es menjaga jembatan ke utara, dan kastil Noctyra ada di baliknya. Ayo, pimpin kami!"),
        narrate("Kontrol: Panah/WASD untuk berjalan, Z/Enter untuk bicara dan memilih, X/Esc untuk kembali."),
    };
}

DialogueScript guildGirl(const Party& p) {
    return {
        say("Elise", eliseLook(), "Selamat datang di Guild Aethel! Ada tips untuk Tactician baru."),
        say("Elise", eliseLook(), "Elemen itu penting. Api efektif ke es, cahaya efektif ke kegelapan, dan sebaliknya."),
        say("Elise", eliseLook(), "Elemen yang sama justru saling menahan. Jangan pakai es untuk melawan golem es ya."),
        say("Elise", eliseLook(), "Kalau MP habis, pilih Bertahan: damage berkurang setengah dan MP pulih sedikit."),
        say(p.member(0), "Dicatat! Terima kasih, Elise.", false),
    };
}

DialogueScript innGreeting() {
    return {
        say("Momo", momoLook(), "Penginapan Momo~ Untuk pahlawan Aethel, istirahatnya gratis!"),
        narrate("Party beristirahat dengan nyenyak..."),
    };
}

DialogueScript innRested(const Party& p) {
    return {
        narrate("HP dan MP seluruh party pulih sepenuhnya."),
        say(p.member(1), "Kasurnya empuk. Aku mau tinggal di sini saja.", false),
        say("Momo", momoLook(), "Hehe, semangat ya!"),
    };
}

DialogueScript signpost() {
    return {narrate("Utara: Jembatan Es. Lebih jauh lagi: Kastil Gerhana. Penginapan Momo ada di timur desa.")};
}

DialogueScript golemIntro(const Party& p) {
    return {
        narrate("Frost Golem menghalangi jembatan. Udara di sekitarnya membeku."),
        say(p.member(1), "Es melawan es? Percuma. Pakai Fireball-ku atau Flame Slash Aria.", false),
        say(p.member(0), "Siap! Hati-hati, kalau dia mengumpulkan es, serangan berikutnya kena semua!", false),
    };
}

DialogueScript witchIntro(const Party& p) {
    const Appearance witch = witchLook();
    return {
        say("Noctyra", witch, "Jadi kalian yang menghancurkan golem kesayanganku."),
        say(p.member(0), "Kembalikan matahari Aethel, Noctyra!", false),
        say("Noctyra", witch, "Matahari? Aku sudah lupa rasanya hangat. Kenapa harus kukembalikan?"),
        say(p.member(2), "Cahaya Light-ku akan menembus kegelapanmu.", false),
        say("Noctyra", witch, "Menarik. Tunjukkan padaku apakah Tactician kalian memang layak!"),
    };
}

DialogueScript ending(const Party& p) {
    const Appearance witch = witchLook();
    return {
        say("Noctyra", witch, "Kenapa... cahaya kalian terasa begitu hangat..."),
        say(p.member(2), "Kegelapan yang mengikatmu sudah hilang. Kamu bebas sekarang.", false),
        say("Noctyra", witch, "...Kalau begitu, bolehkah aku ikut melihat matahari terbit bersama kalian?"),
        say(p.member(1), "Asal jangan bikin gerhana lagi.", false),
        narrate("Gerhana runtuh. Untuk pertama kalinya dalam seratus tahun, matahari terbit di Aethel."),
        say(p.member(0), "Kita berhasil! Terima kasih, Tactician!", false),
    };
}

}  // namespace story
