#pragma once

#include "Assets.h"
#include "Vec2.h"
#include <vector>
#include <string>

#include "json.hpp"
using json = nlohmann::json;

class Component {
public:
    bool has = false;
};

class CTransform : public Component {
public:
    Vec2 pos = { 0.0, 0.0 };
    Vec2 prevPos = { 0.0, 0.0 };
    Vec2 velocity = { 0.0, 0.0 };
    Vec2 move = { 0.0, 0.0 };
    Vec2 scale = { 1.0, 1.0 };
    float angle = 0;
    float z = 0;

    CTransform() {}
    CTransform(const Vec2 & p, float z = 0)
        : pos(p), prevPos(p), z(z) {}
    CTransform(const Vec2 & p, const Vec2 & sp, const Vec2 & sc, float a)
        : pos(p), prevPos(p), velocity(sp), move({0, 0}), scale(sc), angle(a) {}

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CTransform, has, pos, prevPos, scale, velocity, move, angle, z)
};

class CLifespan : public Component {
public:
    int lifespan = 0;
    int frameCreated = 0;
    CLifespan() {}
    CLifespan(int duration, int frame)
        : lifespan(duration), frameCreated(frame) {}

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CLifespan, has, lifespan, frameCreated)
};

class CDamage : public Component {
public:
    float damage = 1;
    CDamage() {}
    CDamage(float d)
        : damage(d) {}

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CDamage, has, damage)
};

class CInvincibility : public Component {
public:
    int iframes = 0;
    CInvincibility() {}
    CInvincibility(int f)
        : iframes(f) {}

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CInvincibility, has, iframes)
};

class CHealth : public Component {
public:
    float max = 1;
    float current = 1;
    CHealth() {}
    CHealth(float m, float c)
        : max(m), current(c) {}

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CHealth, has, max, current)
};

class CInput : public Component {
public:
    bool up       = false;
    bool down     = false;
    bool left     = false;
    bool right    = false;
    bool shoot    = false;
    
    int  num        = -1;
    bool numPressed = false;

    CInput() {}

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CInput, has)
};

class CBoundingBox : public Component {
public:
    Vec2 size;
    Vec2 halfSize;
    bool blockMove = false;
    bool blockVision = false;

    CBoundingBox() {}
    CBoundingBox(const Vec2 & s)
        : size(s), halfSize(s/2) {}
    CBoundingBox(const Vec2 & s, bool m, bool v)
        : size(s), halfSize(s/2), blockMove(m), blockVision(v) {}

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CBoundingBox, has, size, halfSize, blockMove, blockVision)
};

class CAnimation : public Component {
public:
    Animation animation;
    bool repeat = false;

    CAnimation() {}
    CAnimation(const Animation & a, bool r)
        : animation(a), repeat(r) {}

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CAnimation, has, animation, repeat)
};

class CGravity : public Component {
public:
    float gravity = 0;

    float antiGravity = 0;
    int antiGravLifetime = 0;
    float getCalcGravity() {
        if (antiGravLifetime <= 0) {
            return gravity;
        }

        antiGravLifetime--;
        return antiGravity;
    }

    CGravity() {}
    CGravity(float g)
        : gravity(g)
    {}

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CGravity, has, gravity)
};

enum Direction { LEFT, RIGHT, UP, DOWN };

class CState : public Component {
public:
    Direction direction = Direction::RIGHT;
    bool run        = false;
    bool onAir      = false;
    bool shoot      = false;
    int  shootDelay = 0;

    CState() {}
    CState(Direction d, bool r, bool a, bool s)
        : direction(d), run(r), onAir(a), shoot(s), shootDelay(0) {}

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CState, has, direction, run, onAir, shoot)
};

class CFollowPlayer : public Component {
public:
    Vec2 home = {0, 0};
    float speed = 0;
    float visionRadius = 500;

    CFollowPlayer() {}
    CFollowPlayer(Vec2 p, float s, float vr = 500)
        : home(p), speed(s), visionRadius(vr) {}

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CFollowPlayer, has, home, speed, visionRadius)
};

class CPatrol : public Component {
public:
    std::vector<Vec2> positions;
    size_t currentPosition = 0;
    float speed = 0;

    CPatrol() {}
    CPatrol(std::vector<Vec2>& pos, float s)
        : positions(pos), speed(s) {}

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CPatrol, has, positions, currentPosition, speed)
};

class CDraggable : public Component {
public:
    bool dragging = false;
    CDraggable() {}

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CDraggable, has, dragging)
};

class CParallax : public Component {
public:
    Vec2 startPos = {0, 0};
    int repeat = 1;

    CParallax() {}
    CParallax(const Vec2 & p, int repeat)
        : startPos(p), repeat(repeat) {}

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CParallax, has, startPos, repeat)
};

class CTeleport : public Component {
public:
    int level;

    CTeleport() {}
    CTeleport(int level)
        : level(level) {}

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CTeleport, has, level)
};

class CArtefact : public Component {
public:
    std::string artefact;

    CArtefact() {}
    CArtefact(const std::string & artefact)
        : artefact(artefact) {}

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CArtefact, has, artefact)
};

class CArtefactSpawner : public Component {
public:
    std::vector<std::string> artefacts;

    CArtefactSpawner() {}
    CArtefactSpawner(std::vector<std::string> artefacts)
        : artefacts(artefacts) {}

    const std::string getArtefact() {
        if (artefacts.size() > 0) {
            return artefacts[rand() % artefacts.size()];
        }

        return "";
    }

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CArtefactSpawner, has, artefacts)
};
