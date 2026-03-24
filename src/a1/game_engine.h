//
// Created by Oleg Shtykov on 24.09.2025.
//

#ifndef COMP4300_GAME_ENGINE_H
#define COMP4300_GAME_ENGINE_H


#include "entity_manager.h"

class GameEngine {
    std::shared_ptr<EntityManager> m_entityManager;

public:
    GameEngine()
        : m_entityManager (std::make_shared<EntityManager>())
    {}

    void mainLoop();

    std::shared_ptr<EntityManager> getEntityManager() const {
        return m_entityManager;
    }
};


#endif //COMP4300_GAME_ENGINE_H
