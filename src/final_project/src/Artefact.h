#pragma once

#include <string>
#include <optional>
#include <vector>

enum ArtefactGroup { AG_NONE, AG_WEAPON, AG_ABILITY };
enum ArtefactType  { AT_NONE, AT_DAMAGE, AT_HEALING, AT_INVINCIBILITY, AT_ANTIGRAVITY, AT_SPEED };
static const std::vector<std::string> ArtefactsList = { "arrow", "light_arrow", "health", "invincibility", "antigravity", "speed" }; // "sword" спавнить меч не нужно, т.к. меч не отчуждаем

class Artefact {
public:
    std::string   name          = "";
    ArtefactGroup group         = AG_NONE;
    ArtefactType  type          = AT_NONE;
    std::string   invAnimation  = "";
    float         value         = 1.0;
    int           count         = 1;
    int           lifetime      = 0;

    Artefact() {};
    Artefact(const std::string & name, ArtefactGroup group, ArtefactType type, const std::string & invAnimation, float value = 1.0, int count = 1, int lifetime = 0)
        : name(name), group(group), type(type), invAnimation(invAnimation), value(value), count(count), lifetime(lifetime)
    {};

    static std::optional<Artefact> make(const std::string & name);
};
