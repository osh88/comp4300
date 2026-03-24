#include "Entity.h"
#include <iostream>

bool Entity::isActive() const {
    return m_active;
}

const std::string& Entity::tag() const {
    return m_tag;
}

const size_t Entity::id() const {
    return m_id;
}

void Entity::destroy() {
    // TODO
    m_active = false;
}

void Entity::print() const {
    std::cout << "Entity(" << m_tag << ":" << m_id << ")" << std::endl;
}

bool Entity::collision(std::shared_ptr<Entity> e) const {
    if (!cCollision || !e->cCollision) {
        return false;
    }

    auto p1 = Vec2(cShape->circle.getPosition().x, cShape->circle.getPosition().y);
    auto p2 = Vec2(e->cShape->circle.getPosition().x, e->cShape->circle.getPosition().y);

    return p1.dist(p2) < cCollision->radius + e->cCollision->radius;
}