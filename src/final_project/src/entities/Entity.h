#pragma once

#include "Components.h"
#include "EntityMemoryPool.h"

#include <tuple>
#include <string>

#include "json.hpp"
using json = nlohmann::json;

class EntityManager;

class Entity {
    friend class EntityManager;

    size_t m_id = 0;

    // constructor is private so we can never create
    // entities outside the EntityManager which had friend access
    Entity(const size_t id);

public:
    Entity() : m_id(0) {}

    void onDestroy(std::function<void()> f);
    void destroy();
    size_t id() const;
    bool isActive() const;
    const std::string & tag() const;
    bool hasAnimation(const std::string & name) const;
    Vec2 getSize();
    Vec2 getBoundingBox();

    template <typename T>
    bool hasComponent() const {
        return EntityMemoryPool::Instance().hasComponent<T>(m_id);
    }

    template <typename T, typename... TArgs>
    T & addComponent(TArgs&&... mArgs) {
        return EntityMemoryPool::Instance().addComponent<T>(m_id, std::forward<TArgs>(mArgs)...);
    }

    template <typename T>
    T & getComponent() {
        return EntityMemoryPool::Instance().getComponent<T>(m_id);
    }

    template <typename T>
    const T & getComponent() const {
        return EntityMemoryPool::Instance().getComponent<T>(m_id);
    }

    template <typename T>
    void removeComponent() {
        EntityMemoryPool::Instance().removeComponent<T>(m_id);
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Entity, m_id)
};
