#include "Settings.h"

int Settings::getLevelIndexByPath(const std::string & path) {
    for (size_t i=0; i<levels.size(); i++) {
        if (levels[i].path == path) {
            return i;
        }
    }

    return -1;
}

std::optional<const Settings::Level> Settings::getLevelByPath(const std::string & path) {
    for (auto& lvl : levels) {
        if (lvl.path == path) {
            return lvl;
        }
    }

    return std::nullopt;
}

std::optional<const Settings::Level> Settings::getLevelByIndex(size_t index) {
    if (index >= 0 && index < levels.size()) {
        return levels[index];
    }

    return std::nullopt;
}

std::optional<const Settings::Level> Settings::getFirstWorldLevel() {
    for (auto& lvl : levels) {
        if (lvl.world) {
            return lvl;
        }
    }

    return std::nullopt;
}