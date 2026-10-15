#include "Element.h"

std::string elementName(Element element) {
    switch (element) {
        case Element::Physical: return "Physical";
        case Element::Fire:     return "Fire";
        case Element::Ice:      return "Ice";
        case Element::Light:    return "Light";
        case Element::Dark:     return "Dark";
    }
    return "?";
}

double elementMultiplier(Element attack, Element defender) {
    if (attack == Element::Physical || defender == Element::Physical) return 1.0;
    if (attack == defender) return 0.5;
    const bool fireIce   = (attack == Element::Fire  && defender == Element::Ice) ||
                           (attack == Element::Ice   && defender == Element::Fire);
    const bool lightDark = (attack == Element::Light && defender == Element::Dark) ||
                           (attack == Element::Dark  && defender == Element::Light);
    return (fireIce || lightDark) ? 1.5 : 1.0;
}
