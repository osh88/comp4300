#include "Physics.h"
#include "Components.h"

Vec2 Physics::GetOverlap(Entity a, Entity b) {
    if (!a.hasComponent<CBoundingBox>() || !b.hasComponent<CBoundingBox>()) {
        return Vec2(0, 0);
    }

    auto ap = a.getComponent<CTransform>().pos + a.getComponent<CTransform>().velocity;
    auto bp = b.getComponent<CTransform>().pos + b.getComponent<CTransform>().velocity;
    auto delta = Vec2(abs(bp.x-ap.x), abs(bp.y-ap.y));

    auto as = a.getBoundingBox() / 2.0;
    auto bs = b.getBoundingBox() / 2.0;
    auto overlap = Vec2(as.x + bs.x - delta.x, as.y + bs.y - delta.y);

    if (overlap.x < 0) { overlap.x = 0; }
    if (overlap.y < 0) { overlap.y = 0; }

    if (overlap.x > 0 && ap.x < bp.x) { overlap.x = -overlap.x; }
    if (overlap.y > 0 && ap.y < bp.y) { overlap.y = -overlap.y; }

    return overlap;
}

Vec2 Physics::GetPreviousOverlap(Entity a, Entity b) {
    if (!a.hasComponent<CBoundingBox>() || !b.hasComponent<CBoundingBox>()) {
        return Vec2(0, 0);
    }

    auto ap = a.getComponent<CTransform>().pos;
    auto bp = b.getComponent<CTransform>().pos;
    auto delta = Vec2(abs(bp.x-ap.x), abs(bp.y-ap.y));

    auto as = a.getBoundingBox() / 2.0;
    auto bs = b.getBoundingBox() / 2.0;
    auto overlap = Vec2(as.x + bs.x - delta.x, as.y + bs.y - delta.y);

    if (overlap.x < 0) { overlap.x = 0; }
    if (overlap.y < 0) { overlap.y = 0; }

    if (overlap.x > 0 && ap.x < bp.x) { overlap.x = -overlap.x; }
    if (overlap.y > 0 && ap.y < bp.y) { overlap.y = -overlap.y; }

    return overlap;
}

bool Physics::IsInside(const Vec2 & pos, Entity e) {
    auto ep = e.getComponent<CTransform>().pos;
    auto es = e.getSize() / 2.0;
    auto dist = Vec2(fabs(pos.x-ep.x), fabs(pos.y-ep.y));
    return dist.x <= es.x && dist.y <= es.y;
}

Intersect Physics::LineIntersect(const Vec2 & a, const Vec2 & b, const Vec2 & c, const Vec2 & d, float deviation) {
    // TODO: implement this function
    // vectors: ab, cd
    // r = b-a, s = d-c
    // scalar multipliers: t, u
    //  b = a + t*r, d = c + u*s (t=u=1);
    // a + t*r = c + u*s;

    // a * a = 0
    // (a + r) * s = a*s + r*s
    // a + t*r = c + u*s

    // (a + t*r) x s = (c + u*s) x s
    // a x s + t(r x s) = c x s + u(s x s) // s x s = 0
    // a x s + t(r x s) = c x s
    // t(r x s) = c x s - a x s = (c - a) x s
    // t = ((c - a) x s) / (r x s)

    // (a + t*r) x r = (c + u*s) x r
    // a x r + t(r x r) = c x r + u(s x r)
    // a x r = c x r + u(s x r)
    // a x r - c x r = u(s x r)
    // (a - c) x r = u(s x r)
    // u = ((a - c) x r) / (s x r)

    // t = ((c - a) x s) / (r x s)
    // u = ((a - c) x r) / (s x r)
    // (r x s) = -(s x r)
    // u = -(r x (a - c)) / -(r x s)
    // u = (r x (a - c)) / (r x s) = (r x a - r x c) / (r x s) = (-(a x r) + (c x r)) / (r x s) = ((c x r)-(a x r)) / (r x s) = ((c - a) x r) / (r x s)
    
    // u = ((c - a) x r) / (r x s)
    // t = ((c - a) x s) / (r x s)

    Vec2 r = (b - a) * (1+deviation);
    Vec2 s = (d - c) * (1+deviation);
    Vec2 cma = c - a;
    float rms = r * s;
    float t = (cma * s) / rms;
    float u = (cma * r) / rms;

    Intersect res;
    res.status = t >= 0 && t <= 1 && u >= 0 && u <= 1;
    res.result = a + r*t;

    return res;
}

bool Physics::EntityIntersect(const Vec2 & a, const Vec2 & b, Entity e) {
    auto pos = e.getComponent<CTransform>().pos;
    auto size = e.getBoundingBox() / 2.0;
    auto e1 = pos-size;
    auto e2 = Vec2(pos.x+size.x, pos.y-size.y);
    auto e3 = pos+size;
    auto e4 = Vec2(pos.x-size.x, pos.y+size.y);

    auto r = LineIntersect(a, b, e1, e2);
    if (r.status) {
        return true;
    }

    r = LineIntersect(a, b, e2, e3);
    if (r.status) {
        return true;
    }

    r = LineIntersect(a, b, e3, e4);
    if (r.status) {
        return true;
    }

    r = LineIntersect(a, b, e4, e1);
    return r.status;
}
