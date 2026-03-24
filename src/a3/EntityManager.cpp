#include "EntityManager.h"

void EntityManager::update() {
    // TODO: add entities from m_entitiesToAdd the proper location(s)
    //       - add them to the vector of all entities
    //       - add them to the vector inside the map, with the tag as a key

    for (auto e : m_entitiesToAdd) {
        m_entities.push_back(e);
        m_entityMap[e->tag()].push_back(e);
    }
    m_entitiesToAdd.clear();

    // remove dead entities from the vector of all entities
    removeDeadEntities(m_entities);

    // remove dead entities from each vector in the entity map
    // C++17 way of iterating through [key, value] pairs in a map
    for (auto & [tag, entityVec] : m_entityMap) {
        removeDeadEntities(entityVec);
    }
}

void EntityManager::removeDeadEntities(EntityVec &vec) {
    auto new_end = std::remove_if(vec.begin(), vec.end(), [](auto n) {
        return !n->isActive(); // Returns true for even numbers (to be removed)
    });

    // Erase the elements from the new_end to the original end
    vec.erase(new_end, vec.end());
}

std::shared_ptr<Entity> EntityManager::addEntity(const std::string &tag) {
    auto entity = std::shared_ptr<Entity>(new Entity(m_totalEntities++, tag));

    m_entitiesToAdd.push_back(entity);

    return entity;
}

const EntityVec & EntityManager::getEntities() {
    return m_entities;
}

const EntityVec & EntityManager::getEntities(const std::string & tag) {
    return m_entityMap[tag];
}