#include "Physics.h"
#include "Components.h"

Vec2 Physics::GetOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b) {
    // TODO return the overlap rectangle size of the bounding boxes of entity a and b

    if (!a->hasComponent<CBoundingBox>() || !b->hasComponent<CBoundingBox>()) {
        return Vec2(0, 0);
    }

    auto ap = a->getComponent<CTransform>().pos;
    auto bp = b->getComponent<CTransform>().pos;
    auto delta = Vec2(std::abs(bp.x-ap.x), std::abs(bp.y-ap.y));

    auto as = a->getComponent<CBoundingBox>().halfSize;
    auto bs = b->getComponent<CBoundingBox>().halfSize;
    auto overlap = Vec2(as.x + bs.x - delta.x, as.y + bs.y - delta.y);

    return overlap;
}

Vec2 Physics::GetPreviousOverlap(std::shared_ptr<Entity> a, std::shared_ptr<Entity> b) {
    // TODO return the previous overlap rectangle size of the bounding boxes of entity a and b
    //      previous overlap uses the entity's previous position

    if (!a->hasComponent<CBoundingBox>() || !b->hasComponent<CBoundingBox>()) {
        return Vec2(0, 0);
    }

    auto ap = a->getComponent<CTransform>().prevPos;
    auto bp = b->getComponent<CTransform>().prevPos;
    auto delta = Vec2(std::abs(bp.x-ap.x), std::abs(bp.y-ap.y));

    auto as = a->getComponent<CBoundingBox>().halfSize;
    auto bs = b->getComponent<CBoundingBox>().halfSize;
    auto overlap = Vec2(as.x + bs.x - delta.x, as.y + bs.y - delta.y);

    return overlap;
}
