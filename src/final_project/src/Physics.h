#pragma once

#include "Entity.h"
#include "Vec2.h"

struct Intersect {
    bool status;
    Vec2 result;
};

class Physics {
public:
    static Vec2 GetOverlap(Entity a, Entity b);
    static Vec2 GetPreviousOverlap(Entity a, Entity b);
    static bool IsInside(const Vec2 & pos, Entity e);
    static Intersect LineIntersect(const Vec2 & a, const Vec2 & b, const Vec2 & c, const Vec2 & d, float deviation = 0.0);
    static bool EntityIntersect(const Vec2 & a, const Vec2 & b, Entity e);
};