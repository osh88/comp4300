//
// Created by Oleg Shtykov on 24.09.2025.
//

#ifndef COMP4300_ENTITY_H
#define COMP4300_ENTITY_H

#include <cstddef>
#include <string>

class Entity {
    const size_t      m_id    = 0;
    const std::string m_tag   = "Default";
    bool              m_alive = true;

    friend class EntityManager;
    //friend std::shared_ptr<Entity> EntityManager::addEntity(const std::string& tag);

    Entity(const std::string& tag, size_t id)
            : m_id (id)
            , m_tag (tag)
    {};

public:
//    std::shared_ptr<CTransform> cTransform;
//    std::shared_ptr<CName> cName;
//    std::shared_ptr<CShape> cShape;
//    std::shared_ptr<CBBox> cBBox;

    void destroy() {
        m_alive = false;
    };

    const std::string& tag() {
        return m_tag;
    }
};

#endif //COMP4300_ENTITY_H
