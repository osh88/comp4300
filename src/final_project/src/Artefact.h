#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <vector>

enum class ArtefactGroup { NONE, WEAPON, ABILITY };
enum class ArtefactType  { NONE, DAMAGE, HEALING, INVINCIBILITY, ANTIGRAVITY, SPEED };
static const std::vector<std::string> ArtefactsList = { "arrow", "light_arrow", "health", "invincibility", "antigravity", "speed" }; // "sword" спавнить меч не нужно, т.к. меч не отчуждаем

class Artefact {
public:
    std::string   name          = "";
    ArtefactGroup group         = ArtefactGroup::NONE;
    ArtefactType  type          = ArtefactType::NONE;
    std::string   invAnimation  = "";
    float         value         = 1.0;
    int           count         = 1;
    int           lifetime      = 0;

    Artefact() {};
    Artefact(std::string_view name, ArtefactGroup group, ArtefactType type, std::string_view invAnimation, float value = 1.0, int count = 1, int lifetime = 0)
        : name(name), group(group), type(type), invAnimation(invAnimation), value(value), count(count), lifetime(lifetime)
    {};

    static std::optional<Artefact> make(const std::string & name);
};
