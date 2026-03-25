#include "EntityManager.h"
#include "EntityMemoryPool.h"
#include "Entity.h"
#include "Profiler.h"

EntityManager::EntityManager() {
    //EntityMemoryPool::Instance().clear();
}

void EntityManager::update() {
    PROFILE_FUNCTION();

    // TODO: add entities from m_entitiesToAdd the proper location(s)
    //       - add them to the vector of all entities
    //       - add them to the vector inside the map, with the tag as a key

    for (auto e : m_entitiesToAdd) {
        m_entities.push_back(e);
        m_entityMap[e.tag()].push_back(e);
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
        // ! Затираем все данные повторно, т.к. между уничтожением
        // (обнулением) сущности и удалением ее из менеджера сущностей
        // в нее могут быть внесены изменения.
        if (!n.isActive()) { n.destroy(); };

        return !n.isActive();
    });

    // Erase the elements from the new_end to the original end
    vec.erase(new_end, vec.end());
}

Entity EntityManager::addEntity(const std::string &tag) {
    auto id = EntityMemoryPool::Instance().addEntity(tag);

    auto entity = Entity(id);
    m_entitiesToAdd.push_back(entity);

    return entity;
}

const EntityVec & EntityManager::getEntities() {
    return m_entities;
}

const EntityVec & EntityManager::getEntities(const std::string & tag) {
    return m_entityMap[tag];
}

const std::vector<EntityVec> EntityManager::getEntities(std::vector<std::string> tags) {
    std::vector<EntityVec> r;

    for (auto tag : tags) {
        if (auto v = m_entityMap.find(tag); v != m_entityMap.end()) {
            r.push_back(v->second);
        }
    }

    return r;
}

void EntityManager::sort(std::function<bool(const Entity&, const Entity&)> f) {
    std::sort(m_entities.begin(), m_entities.end(), f);
}

void EntityManager::sortEntitiesByZ() {
    sort([](const Entity& a, const Entity& b) {
        return (a.getComponent<CTransform>().z < b.getComponent<CTransform>().z);
    });
}
