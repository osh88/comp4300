#include "Artefact.h"
#include <optional>
#include <iostream>

std::optional<Artefact> Artefact::make(const std::string & name) {
    if (name == "sword") {
        return Artefact( name, AG_WEAPON, AT_DAMAGE, "SwordRight", 2.0, 1, 0 );
    }

    if (name == "arrow") {
        return Artefact( name, AG_WEAPON, AT_DAMAGE, "Arrow", 2.0, 10, 0 );
    }

    if (name == "light_arrow") {
        return Artefact( name, AG_WEAPON, AT_DAMAGE, "ArrowLight", 4.0, 10, 0 );
    }

    if (name == "health") {
        return Artefact( name, AG_ABILITY, AT_HEALING, "Heart", 3.0, 1, 0 );
    }

    if (name == "invincibility") {
        return Artefact( name, AG_ABILITY, AT_INVINCIBILITY, "Invincibility", 0, 1, 600 ); // 10 sec
    }

    if (name == "antigravity") {
        return Artefact( name, AG_ABILITY, AT_ANTIGRAVITY, "Earth", 0.3, 1, 600 ); // 10 sec
    }

    if (name == "speed") {
        return Artefact( name, AG_ABILITY, AT_SPEED, "Speed", 10, 1, 600 ); // 10 sec
    }

    std::cout << "Error: makeArtefact(): unknown artefact " << name << std::endl;

    return std::nullopt;
}