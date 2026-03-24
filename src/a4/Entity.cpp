#include "Entity.h"
#include <tuple>

Entity::Entity(const size_t i)
        : m_id(i) {}

bool Entity::isActive() const {
    return EntityMemoryPool::Instance().isActive(m_id);
}

const std::string& Entity::tag() const {
    return EntityMemoryPool::Instance().getTag(m_id);
}

size_t Entity::id() const {
    return m_id;
}

void Entity::destroy() {
    EntityMemoryPool::Instance().removeEntity(m_id);
}

bool Entity::hasAnimation(const std::string & name) const {
    if (hasComponent<CAnimation>()) {
        return getComponent<CAnimation>().animation.getName() == name;
    }

    return false;
}

void Entity::onDestroy(std::function<void()> f) {
    EntityMemoryPool::Instance().setOnDestroyF(m_id, f);
}