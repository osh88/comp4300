//
// Created by Oleg Shtykov on 24.09.2025.
//

#include "entity_manager.h"
#include <memory>

std::shared_ptr<Entity> EntityManager::addEntity(const std::string& tag) {
    auto e = std::shared_ptr<Entity>(new Entity(tag, ++m_totalEntities));
    m_toAdd.push_back(e);
    return e;
}

void EntityManager::update() {
    for (auto e : m_toAdd) {
        m_entities.push_back(e);
        m_entityMap[e->tag()].push_back(e);
    }
    m_toAdd.clear();

    // delete all entities with m_alive == false
}

// ----------------------------------------------------------------
// ----------------------------------------------------------------

//void spawnEnemy() {
//    EntityManager m_entities;
//    auto e = m_entities.addEntity("enemy");
//    e->cTransform = std::make_shared<CTransform>(args);
//    e->cShape = std::make_shared<CShape>(args);
//}
//
//void collisions() {
//    EntityManager m_entities;
//
//    for (auto b : m_entities.getEntities("bullet")) {
//        for (auto e : m_entities.getEntities("enemy")) {
//            if (Physics::CheckCollision(b, e)) {
//                b->destroy();
//                e->destroy();
//            }
//        }
//    }
//}