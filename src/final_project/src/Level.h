#pragma once

#include <fstream>

#include "GameEngine.h"
#include "EntityMemoryPool.h"
#include "EntityManager.h"

#include "Compress.h"
#include "Profiler.h"

#include "json.hpp"
using json = nlohmann::json;

class Level {
    EntityMemoryPool pool = EntityMemoryPool::Instance();
    EntityManager    entityManager;
    uint32_t         bgColor;

public:

    Level() : pool(EntityMemoryPool::Instance()) {};
    Level(EntityMemoryPool p, EntityManager entityManager, sf::Color bgColor) : pool(p), entityManager(entityManager), bgColor(bgColor.toInteger()) {};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Level, pool, entityManager, bgColor);

    static void loadFromFile(const std::string & path, GameEngine* game, EntityManager & em, sf::Color& bg) {
        PROFILE_FUNCTION();

        std::cout << "Level::loadFromFile(" << path << ")" << std::endl;

        if (!std::filesystem::exists(path)) {
            return;
        }

        EntityMemoryPool::Instance().clear();
        em = EntityManager();
        uint32_t bgColor;

        std::ifstream f(path, std::ios::binary); // std::ios::binary for windows!!!
        std::stringstream ss;
        LZMA::decompressStream(f, ss);
        f.close();

        {
            PROFILE_SCOPE("json::parse()");

            auto r = json::parse(ss);

            r.at("pool").get_to(EntityMemoryPool::Instance());
            r.at("entityManager").get_to(em);
            r.at("bgColor").get_to(bgColor);
            bg = sf::Color(bgColor);
        }

        for (auto e : em.getEntities()) {
            auto& a = e.getComponent<CAnimation>().animation;
            // Саму тексутуру загружать не нужно, т.к. используется одна большая текстура
            // Но у анимации нужно обновлять позицию каждой текстуры в большой текстуре,
            // т.к. в процессе построения большой текстуры позиции малых текстур могли измениться.
            if (a.getTexture().name != "") {
                a.setSprite(game->assets().getTexture(a.getTexture().name));
            }
        }

        em.update();
    }

    static void save(const std::string & path, const EntityManager & em, sf::Color bgColor) {
        PROFILE_FUNCTION();
        json j = Level(EntityMemoryPool::Instance(), em, bgColor);

        std::stringstream ss;
        ss << j.dump() << std::endl;
        std::ofstream f(path, std::ios::binary);
        LZMA::compressStream(ss, f);
        f.close();
    }
};
