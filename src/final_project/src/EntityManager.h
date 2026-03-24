#pragma once

#include "Entity.h"
#include <vector>
#include <map>
#include <algorithm>

#include "json.hpp"
using json = nlohmann::json;

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
    const std::vector<EntityVec> getEntities(std::vector<std::string> tags);
    void sort(std::function<bool(const Entity&, const Entity&)>);
    void sortEntitiesByZ();

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(EntityManager, m_entities, m_entitiesToAdd, m_entityMap)
};