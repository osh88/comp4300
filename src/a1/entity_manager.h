//
// Created by Oleg Shtykov on 24.09.2025.
//

#ifndef COMP4300_ENTITY_MANAGER_H
#define COMP4300_ENTITY_MANAGER_H

#include <map>
#include <vector>
#include <memory>
#include "entity.h"

typedef std::vector<std::shared_ptr<Entity>> EntityVec;
typedef std::map   <std::string, EntityVec>  EntityMap;

class EntityManager {
    EntityVec m_entities;
    EntityVec m_toAdd;
    EntityMap m_entityMap;
    size_t    m_totalEntities = 0;

public:

    EntityManager() {};

    void update();
    std::shared_ptr<Entity> addEntity(const std::string& tag);
    EntityVec& getEntities();
    EntityVec& getEntities(const std::string& tag);
};

#endif //COMP4300_ENTITY_MANAGER_H
