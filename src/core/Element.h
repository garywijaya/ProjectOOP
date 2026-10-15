#pragma once
#include <string>

// Elemen serangan dan pertahanan. Dipakai untuk menghitung kelemahan musuh.
enum class Element { Physical, Fire, Ice, Light, Dark };

std::string elementName(Element element);

// Pengali damage: 1.5 jika lemah, 0.5 jika tahan (elemen sama), 1.0 normal.
double elementMultiplier(Element attack, Element defender);

// Paket informasi serangan. Dipakai oleh overload Combatant::takeDamage(const Attack&).
struct Attack {
    int power = 0;
    Element element = Element::Physical;
    bool ignoreDefense = false;
};
