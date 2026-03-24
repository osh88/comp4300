#pragma once

#include <string>
#include <fstream>
#include <filesystem>
#include <SFML/Window/Keyboard.hpp>

#include "Vec2.h"

#include "json.hpp"
using json = nlohmann::json;

enum Difficulty {EASY, MEDIUM, HARD};
class GameEngine;

class Settings {
    friend class GameEngine;

public:
    struct Level {
        std::string path = "";
        bool        userLevel = false;
        bool        world = false;
        bool        pass = false;
        std::string shader = "Firework0";

        Level() {};

        Level(const std::string & path, bool userLevel = false, bool world = false, bool pass = false, const std::string & shader = "Firework0") : path(path), userLevel(userLevel), world(world), pass(pass), shader(shader) {};

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Level, path, userLevel, world, pass, shader)
    };

    struct Action {
    public:
        std::string       name;
        std::string       action;
        sf::Keyboard::Key key;
        size_t            index;

        Action() {}

        Action(const std::string & n, const std::string & a, sf::Keyboard::Key k, size_t i)
            : name(n), action(a), key(k), index(i)
        {}

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Action, name, action, key)
    };

    class PlayerConfig {
    public:
        float speed     = 5;
        float maxSpeed  = 20;
        float jumpSpeed = -20;

        PlayerConfig() {};

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PlayerConfig, speed, maxSpeed, jumpSpeed)
    };

    std::string  assetsPath    = "config/assets.txt";
    int          musicVolume   = 50;
    int          effectsVolume = 50;
    Difficulty   difficulty    = Difficulty::MEDIUM;
    bool         fullScreen    = false;
    bool         mute          = false;
    PlayerConfig playerConfig  = PlayerConfig();

    std::vector<Level> levels {
        {"config/levels/world.bin",  false, true,  true,  "Firework4"},
        {"config/levels/level0.bin", false, false, false, "Firework0"},
        {"config/levels/level1.bin", false, false, false, "Firework1"},
        {"config/levels/level2.bin", false, false, false, "Firework2"},
        {"config/levels/level3.bin", false, false, false, "Firework3"},
        {"config/levels/final.bin",  false, false, false, "Firework4"}
    };

    std::map<std::string, Action> keys {
        {"MOVE_UP",    {"Прыжок",   "MOVE_UP",    sf::Keyboard::Key::Up,    0}},
        {"MOVE_LEFT",  {"Влево",    "MOVE_LEFT",  sf::Keyboard::Key::Left,  0}},
        {"MOVE_RIGHT", {"Вправо",   "MOVE_RIGHT", sf::Keyboard::Key::Right, 0}},
        {"SHOOT",      {"Стрелять", "SHOOT",      sf::Keyboard::Key::Space, 0}}
    };

    Settings() {};
    int getLevelIndexByPath(const std::string & path);
    std::optional<const Settings::Level> getLevelByPath(const std::string & path);
    std::optional<const Settings::Level> getLevelByIndex(size_t index);
    std::optional<const Settings::Level> getFirstWorldLevel();

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Settings, assetsPath, musicVolume, effectsVolume, difficulty, fullScreen, mute, playerConfig, levels, keys)

protected:

    static Settings loadFromFile(const std::string & path) {
        if (!std::filesystem::exists(path)) {
            return Settings();
        }

        std::ifstream f(path);
        auto r = json::parse(f).get<Settings>();
        f.close();

        // Нужно, чтобы массив не перемещался
        r.levels.reserve(100);

        return r;
    }

    void save(const std::string & path) {
        json j = *this;
        std::ofstream f(path);
        f << j.dump(4) << std::endl;
        f.close();
    }
};
