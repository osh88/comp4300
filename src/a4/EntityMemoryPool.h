#pragma once

#include "Components.h"

#include <vector>

const int MAX_ENTITIES = 10000;

typedef std::tuple<
    std::vector<CTransform>,
    std::vector<CLifespan>,
    std::vector<CDamage>,
    std::vector<CInvincibility>,
    std::vector<CHealth>,
    std::vector<CInput>,
    std::vector<CBoundingBox>,
    std::vector<CAnimation>,
    std::vector<CState>,
    std::vector<CFollowPlayer>,
    std::vector<CPatrol>
> EntityComponentVectorTulpe;

typedef std::function<void()> DestroyF;

class EntityMemoryPool {
    size_t                     m_numEntities;
    EntityComponentVectorTulpe m_pool;
    std::vector<std::string>   m_tags;
    std::vector<bool>          m_active;
    std::vector<DestroyF>      m_onDestroyF;

    EntityMemoryPool(size_t maxEntities);

public:

    size_t addEntity(const std::string &tag);
    void removeEntity(size_t entityID);
    void setOnDestroyF(size_t entityID, DestroyF f);
    void clear();

    static EntityMemoryPool & Instance() {
        static EntityMemoryPool pool(MAX_ENTITIES);
        return pool;
    }

    template <typename T>
    T& getComponent(size_t entityID) {
        return std::get<std::vector<T>>(m_pool)[entityID];
    }

    template <typename T>
    const T & getComponent(size_t entityID) const {
        return std::get<std::vector<T>>(m_pool)[entityID];
    }

    template <typename T>
    bool hasComponent(size_t entityID) const {
        return getComponent<T>(entityID).has;
    }

    template <typename T>
    void removeComponent(size_t entityID) {
        getComponent<T>(entityID).has = false;
    }

    template <typename T, typename... TArgs>
    T & addComponent(size_t entityID, TArgs&&... mArgs) {
        auto & component = getComponent<T>(entityID);
        component = T(std::forward<TArgs>(mArgs)...);
        component.has = true;
        return component;
    }

    const std::string & getTag(size_t entityID) const {
        return m_tags[entityID];
    }

    bool isActive(size_t entityID) const {
        return m_active[entityID];
    }
};