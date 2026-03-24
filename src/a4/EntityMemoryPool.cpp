#include "EntityMemoryPool.h"

EntityMemoryPool::EntityMemoryPool(size_t maxEntities) {
    m_numEntities = maxEntities;

    m_pool = std::make_tuple(
        std::vector<CTransform>(maxEntities),
        std::vector<CLifespan>(maxEntities),
        std::vector<CDamage>(maxEntities),
        std::vector<CInvincibility>(maxEntities),
        std::vector<CHealth>(maxEntities),
        std::vector<CInput>(maxEntities),
        std::vector<CBoundingBox>(maxEntities),
        std::vector<CAnimation>(maxEntities),
        std::vector<CState>(maxEntities),
        std::vector<CFollowPlayer>(maxEntities),
        std::vector<CPatrol>(maxEntities)
    );

    m_tags = std::vector<std::string>(maxEntities);
    m_active = std::vector<bool>(maxEntities);
    m_onDestroyF = std::vector<DestroyF>(maxEntities);
}

size_t EntityMemoryPool::addEntity(const std::string &tag) {
    for (size_t i=1; i<m_active.size(); i++) {
        if (!m_active[i]) {
            m_active[i] = true;
            m_tags[i] = tag;
            m_onDestroyF[i] = nullptr;
            return i;
        }
    }

    return 0;
}

void EntityMemoryPool::removeEntity(size_t entityID) {
    if (m_onDestroyF[entityID]) {
        m_onDestroyF[entityID]();
    }

    m_onDestroyF[entityID] = nullptr;
    m_active[entityID] = false;
    
    removeComponent<CTransform>(entityID);
    removeComponent<CLifespan>(entityID);
    removeComponent<CDamage>(entityID);
    removeComponent<CInvincibility>(entityID);
    removeComponent<CHealth>(entityID);
    removeComponent<CInput>(entityID);
    removeComponent<CBoundingBox>(entityID);
    removeComponent<CAnimation>(entityID);
    removeComponent<CState>(entityID);
    removeComponent<CFollowPlayer>(entityID);
    removeComponent<CPatrol>(entityID);
}

void EntityMemoryPool::setOnDestroyF(size_t entityID, DestroyF f) {
    m_onDestroyF[entityID] = f;
}

void EntityMemoryPool::clear() {
    for (size_t i=0; i<m_numEntities; i++) {
        if (m_active[i]) {
            removeEntity(i);
        }
    }
}