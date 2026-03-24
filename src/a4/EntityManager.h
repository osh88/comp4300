#pragma once

#include "Entity.h"
#include <vector>
#include <map>

typedef std::vector<Entity>              EntityVec;
typedef std::map<std::string, EntityVec> EntityMap;

class EntityManager {
    EntityVec m_entities;          // all entities
    EntityVec m_entitiesToAdd;     // entities to add next update
    EntityMap m_entityMap;         // map from entity tag to vectors

    void removeDeadEntities(EntityVec & vec);

public:

    EntityManager();

    void update();

    Entity addEntity(const std::string & tag);

    const EntityVec & getEntities();
    const EntityVec & getEntities(const std::string & tag);
};