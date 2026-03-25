#include "Scene_Play.h"
#include "Scene_World.h"
#include "Scene_Win.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Graphics/Color.hpp"
#include "Scene_Menu.h"
#include "Profiler.h"
#include "imgui.h"
#include "Level.h"

#include <math.h>
#include <fstream>
#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()
#include <iostream>

Scene_Play::Scene_Play(GameEngine * gameEngine, Settings::Level & level)
    : Scene(gameEngine)
    , m_level(level)
    , m_frameTime(m_game->assets().getFont("Mario").font, "0", 16)
    , m_gridText(m_game->assets().getFont("Tech").font, "", 12)
    , m_playerState(m_game->assets().getFont("Mario").font, "State: ", 15)
{}

void Scene_Play::init() {
    PROFILE_FUNCTION();

    srand(time(0));

    Level::loadFromFile(m_game->getExeDir() + m_level.path, m_game, m_entityManager, m_bgColor);
    auto it = m_entityManager.getEntities("player");
    if (it.size() == 1) {
        m_player = it.at(0);
    }

    m_leftBottomOfLevel = {100000, -100000};
    for (auto e : m_entityManager.getEntities()) {
        auto pos = e.getComponent<CTransform>().pos;
        auto size = e.getSize() / 2.0;
        pos.x -= size.x;
        pos.y += size.y;

        if (pos.x < m_leftBottomOfLevel.x) m_leftBottomOfLevel.x = pos.x;
        if (pos.y > m_leftBottomOfLevel.y) m_leftBottomOfLevel.y = pos.y;
    }

    if (m_leftBottomOfLevel != Vec2(100000, -100000)) {
        auto wh = m_game->getWindowSize() / 2.0;
        auto vc = Vec2(m_leftBottomOfLevel.x + wh.x, m_leftBottomOfLevel.y - wh.y);

        storeViewCenter(vc);
    } else {
        m_leftBottomOfLevel = Vec2(0, m_game->getWindowSize().y);
        storeViewCenter(m_game->getWindowSize() / 2.0);
    }

    // ======================================== ray castings
    for (auto d : m_entityManager.getEntities("view")) {
        d.destroy();
    }
    // ======================================== ray castings

    m_sParallax.startViewPosX = getStoredViewCenter().x;
    m_sParallax.startPlayerPosY = m_player.getComponent<CTransform>().pos.y;

    m_entityManager.sortEntitiesByZ();

    for (auto [a, item] : m_game->getSettings().keys) {
        registerAction(item.action, item.key);
    }

    registerAction("QUIT",             sf::Keyboard::Key::Escape);
    registerAction("PAUSE",            sf::Keyboard::Key::P, true);
    registerAction("TOGGLE_TEXTURE",   sf::Keyboard::Key::T, true);
    registerAction("TOGGLE_DEBUG",     sf::Keyboard::Key::D, true);
    registerAction("TOGGLE_GRID",      sf::Keyboard::Key::G, true);
    registerAction("RELOAD",           sf::Keyboard::Key::R, true);
    registerAction("TOGGLE_SLOW",      sf::Keyboard::Key::S, true);
    registerAction("TOGGLE_INVENTORY", sf::Keyboard::Key::I);

    for (auto i = sf::Keyboard::Key::Num0; i <= sf::Keyboard::Key::Num9; i = (sf::Keyboard::Key) ((int)i + 1)) {
        registerAction("INV_NUM" + std::to_string((int)i - (int)sf::Keyboard::Key::Num0), i);
    }

    m_playerState.setPosition({10, 10});
    setFPS();

    m_game->inventory().reset();
}

void Scene_Play::setFPS() {
    m_game->setFPS(m_slow ? 10 : 60);
}

void Scene_Play::spawnWeapon() {
    PROFILE_FUNCTION();

    auto art = m_game->inventory().getWeapon();
    if (!art.has_value()) {
        return;
    }

    if (art->name == "sword") {
        if (!m_player.hasComponent<CState>()) {
            return;
        }

        auto e = m_entityManager.addEntity("weapon");
        moveSword(m_player);
        e.addComponent<CLifespan>(10, m_currentFrame);

        auto& stateAttack = m_player.getComponent<CState>().shoot;
        m_game->assets().getSound("Slash")->play();

        e.onDestroy([&stateAttack]() {
            stateAttack = false;
        });

        return;
    }

    if (art->name == "arrow" || art->name == "light_arrow") {
        auto e = m_entityManager.addEntity("weapon");
        e.addComponent<CAnimation>(m_game->assets().getAnimation(art->invAnimation), true);
        e.getComponent<CAnimation>().animation.setColor(sf::Color::White);
        e.addComponent<CBoundingBox>(m_game->assets().getAnimation(art->invAnimation).frameSize());
        e.addComponent<CTransform>(m_player.getComponent<CTransform>().pos, 1);
        e.addComponent<CLifespan>(30, m_currentFrame);
        e.addComponent<CDamage>(art->value);

        auto m = m_player.getComponent<CTransform>().scale.x; // -1/1
        e.getComponent<CTransform>().velocity.x = m * m_game->getSettings().playerConfig.maxSpeed;
        e.getComponent<CTransform>().scale.x = m;
        e.getComponent<CTransform>().pos.x += m*(m_player.getSize().x/4);

        m_game->assets().getSound("Arrow")->play();

        m_game->inventory().delArtefact(art->name);
        return;
    }
}

void Scene_Play::moveSword(Entity entity) {
    PROFILE_FUNCTION();

    if (auto art = m_game->inventory().getWeapon(); !art.has_value() || art->name != "sword") {
        return;
    }

    if (!entity.hasComponent<CState>()) {
        return;
    }

    auto s = entity.getComponent<CState>();
    auto p = entity.getComponent<CTransform>().pos;

    for (auto e : m_entityManager.getEntities("weapon")) {
        switch (s.direction) {
            case Direction::UP:
            setAnimation(e, "SwordUp", true, Vec2(1, 1));
            p.y -= e.getSize().y;
            setBoundingBox(e, Vec2(2, 2), true, false);
            break;

            case Direction::DOWN:
            setAnimation(e, "SwordDown", true, Vec2(1, 1));
            p.y += entity.getSize().y;
            setBoundingBox(e, Vec2(2, 8), true, false);
            break;

            case Direction::LEFT:
            setAnimation(e, "SwordRight", true, Vec2(-1, 1));
            p.x -= e.getSize().x;
            setBoundingBox(e, Vec2(7, 3), true, false);
            break;

            case Direction::RIGHT:
            setAnimation(e, "SwordRight", true, Vec2(1, 1));
            p.x += entity.getSize().x;
            setBoundingBox(e, Vec2(7, 3), true, false);
            break;
        }

        e.getComponent<CTransform>().pos = p;
    }
}

void Scene_Play::spawnArtefact(Entity block, const std::string & artName) {
    if (auto art = Artefact::make(artName); art.has_value()) {
        auto e = m_entityManager.addEntity("dec");
        e.addComponent<CAnimation>(m_game->assets().getAnimation(art->invAnimation), true);
        e.addComponent<CTransform>(block.getComponent<CTransform>().pos);
        e.getComponent<CTransform>().pos.y -= block.getSize().y;
        e.addComponent<CLifespan>(60, m_currentFrame);
    }
}

void Scene_Play::update() {
    PROFILE_FUNCTION();

    if (!m_paused) {
        m_entityManager.update();

        sInventory();
        sAI();
        sVelocity();
        sCollision();
        sTeleport();
        sDragAndDrop();
        sMovement();
        sStatus();
        sAnimation();
        sCamera();
        sParallax();
        sWin();
    }

    sRender();

    if (!m_paused) {
        m_currentFrame++;
    }
}

void Scene_Play::sVelocity() {
    PROFILE_FUNCTION();

    auto& input = m_player.getComponent<CInput>();
    auto& t     = m_player.getComponent<CTransform>();
    auto& s     = m_player.getComponent<CState>();

    if (s.shootDelay > 0) {
        s.shootDelay--;
    }

    if (input.shoot && s.shootDelay == 0) {
        spawnWeapon();
        switch (m_game->getSettings().difficulty) {
            case Difficulty::EASY:
                s.shootDelay = 20; break;
            case Difficulty::MEDIUM:
                s.shootDelay = 40; break;
            case Difficulty::HARD:
                s.shootDelay = 60; break;
        }
    }

    t.velocity.x = 0;

    if (input.up && !s.onAir) {
        //m_game->assets().getSound("Jump")->play();
        t.velocity.y = m_game->getSettings().playerConfig.jumpSpeed;
        s.onAir = true;
    }

    if (input.left) {
        t.velocity.x = -m_game->getSettings().playerConfig.speed;
        if (!s.onAir) {
            s.run = true;
        }
        s.direction = Direction::LEFT;
    }

    if (input.right) {
        t.velocity.x = m_game->getSettings().playerConfig.speed;
        if (!s.onAir) {
            s.run = true;
        }
        s.direction = Direction::RIGHT;
    }

    for (auto e : m_entityManager.getEntities()) {
        auto& t = e.getComponent<CTransform>();

        // Для игрока добавляем ускорение свободного падения только когда он в воздухе
        if (e.id() == m_player.id()) {
            if (m_player.getComponent<CState>().onAir) {
                t.velocity.y += e.getComponent<CGravity>().getCalcGravity();
            } else {
                t.velocity.y = e.getComponent<CGravity>().getCalcGravity()*4; // Увеличиваем, чтобы прижимать к двигающейся вниз платформе
            }
            continue;
        }

        if (e.hasComponent<CGravity>()) {
            t.velocity.y += e.getComponent<CGravity>().getCalcGravity();
        }

        if (t.velocity.y > m_game->getSettings().playerConfig.maxSpeed) {
            t.velocity.y = m_game->getSettings().playerConfig.maxSpeed;
        }
    }

    // TODO: Implement player movement / jumping based on its CInput component
    // TODO: Implement gravity's effect on the player
    // TODO: Implement the maximum player speed in both X and Y directions
    // NOTE: Setting an entity's scale.x to -1/1 will make it face to the left/right
}

void Scene_Play::sMovement() {
    PROFILE_FUNCTION();

    for (auto e : m_entityManager.getEntities()) {
        auto& t = e.getComponent<CTransform>();

        t.prevPos = t.pos;

        t.pos += t.move;
        t.move = {0, 0};

        auto maxSpeed = m_game->getSettings().playerConfig.maxSpeed;

        if (abs(t.velocity.x) > maxSpeed) {
            t.velocity.x = (t.velocity.x < 0) ? -maxSpeed : maxSpeed;
        }

        if (abs(t.velocity.y) > maxSpeed) {
            t.velocity.y = (t.velocity.y < 0) ? -maxSpeed : maxSpeed;
        }

        t.pos += t.velocity;
    }

    // Не позволяем игроку уходить за левый край экрана
    auto& t = m_player.getComponent<CTransform>();
    auto animHalfSizeX = m_player.getSize().x / 2;
    if (t.pos.x - animHalfSizeX < m_leftBottomOfLevel.x) {
        t.pos.x = m_leftBottomOfLevel.x + animHalfSizeX;
    }

    moveSword(m_player);
}

void Scene_Play::sAI() {
    PROFILE_FUNCTION();

    // TODO: Implement Enemy AI
    //       Implement Follow behavior
    //       Implement Patrol behavior

    for (auto npc : m_entityManager.getEntities("npc")) {
        if (npc.hasComponent<CPatrol>()) {
            auto& t = npc.getComponent<CTransform>();
            auto& p = npc.getComponent<CPatrol>();
            auto currentPoint = p.positions.at(p.currentPosition);

            if (t.pos.dist(currentPoint) < 5) {
                p.currentPosition++;
                if (p.currentPosition >= p.positions.size()) {
                    p.currentPosition = 0;
                }
            }
            currentPoint = p.positions.at(p.currentPosition);

            auto speed = (currentPoint-t.pos);
            speed /= speed.length();
            speed *= p.speed;

            if (npc.hasComponent<CGravity>()) {
                speed.y = t.velocity.y;
            }

            t.velocity = speed;
        }

        // TODO добавить проверку видимости цели
        if (npc.hasComponent<CFollowPlayer>()) {
            auto& npcT = npc.getComponent<CTransform>();
            auto p = npc.getComponent<CFollowPlayer>();

            auto playerPos = m_player.getComponent<CTransform>().pos;
            auto newSpeed = (playerPos-npcT.pos);

            bool intersect = false;

            if (newSpeed.length() <= p.visionRadius) {
                for (auto obstacle : m_entityManager.getEntities()) {
                    if (obstacle.id() == m_player.id() || obstacle.id() == npc.id() || !obstacle.getComponent<CBoundingBox>().blockVision) {
                        continue;
                    }

                    if (Physics::EntityIntersect(playerPos, npcT.pos, obstacle)) {
                        intersect = true;
                        break;
                    }
                }
            }

            if (newSpeed.length() > p.visionRadius || intersect) {
                newSpeed = p.home-npcT.pos;
            }

            if (newSpeed.length() > 5) {
                newSpeed /= newSpeed.length();
                newSpeed *= p.speed;

                auto oldSpeed = npcT.velocity;
                auto delta = (newSpeed-oldSpeed) / 10;
                newSpeed = oldSpeed + delta;

                if (npc.hasComponent<CGravity>() && npc.getComponent<CGravity>().gravity != 0) {
                    newSpeed.y += npcT.velocity.y * npc.getComponent<CGravity>().gravity;
                }

                npcT.velocity = newSpeed;
            } else {
                npcT.velocity = Vec2(0, 0);
            }
        }
    }

    for (auto tile : m_entityManager.getEntities("tile")) {
        if (!tile.hasComponent<CPatrol>()) {
            continue;
        }

        auto& t = tile.getComponent<CTransform>();
        auto& p = tile.getComponent<CPatrol>();
        auto currentPoint = p.positions.at(p.currentPosition);

        if (t.pos.dist(currentPoint) < 5) {
            p.currentPosition++;
            if (p.currentPosition >= p.positions.size()) {
                p.currentPosition = 0;
            }
        }
        currentPoint = p.positions.at(p.currentPosition);

        auto speed = (currentPoint-t.pos);
        speed /= speed.length();
        speed *= p.speed;

        t.velocity = speed;
    }
}

void Scene_Play::sStatus() {
    PROFILE_FUNCTION();

    if (m_player.getComponent<CState>().shootDelay > 0) {
        m_player.getComponent<CState>().shootDelay -= 1;
    }

    // TODO: Implement Lifespan here
    //       Implement Invincibility Frames here
    //...

    if (m_player.getComponent<CHealth>().current <= 0) {
        m_game->assets().getSound("LinkDie")->play();
        reload();
        return;
    }

    // Если игрок падает ниже экрана, перезагружаем уровень
    auto pos = m_game->p2c(m_game->window().getSize());
    if (m_player.getComponent<CTransform>().pos.y > pos.y + 100) {
        m_game->assets().getSound("LinkDie")->play();
        reload();
        return;
    }

    for (auto e : m_entityManager.getEntities()) {
        if (e.hasComponent<CInvincibility>()) {
            e.getComponent<CInvincibility>().iframes -= 1;

            if (e.getComponent<CInvincibility>().iframes <= 0) {
                e.removeComponent<CInvincibility>();
            }
        }

        if (e.hasComponent<CLifespan>()) {
            auto c = e.getComponent<CLifespan>();

            if (m_currentFrame >= c.frameCreated + c.lifespan) {
                e.destroy();
            }
        }

        if (e.hasComponent<CHealth>()) {
            if (e.getComponent<CHealth>().current <= 0) {
                e.destroy();
                m_game->assets().getSound("EnemyDie")->play();
            }
        }
    }
}

void Scene_Play::sCollision() {
    PROFILE_FUNCTION();

    // REMEMBER: SFML's (0,0) position is on the TOP-LEFT corner
    //           This means jumping will have a negative y-component
    //           and gravity will have a positive y-component
    //           Also, something BELOW something else will have a y value GREATER than it
    //           Also, something ABOVE something else will have a y value LESS than it

    // TODO: Implement Physics::GetOverlap() function, use it inside this function

    // TODO: Implement bullet / tile collisions
    //       Destroy the tile if it has a Brick animation
    // TODO: Implement player / tile collisions and resolutions
    //       Update the CState component of the player to store whether
    //       it is currently on the ground or in the air. This will be
    //       used by the Animation system
    // TODO: Check to see if the player has fallen down a hole (y > height())
    // TODO: Don't let the player walk off the left side of the map

    // TODO: Implement entity - tile collisions
    //       Implement player - enemy collisions with appropriate damage calculations
    //       Implement sword - NPC collisions
    //       Implement black tile collisions / 'teleporting'
    //       Implement entity - heart collisions and life gain logic

    auto pi = m_player.getComponent<CInput>();
    auto& pt = m_player.getComponent<CTransform>();
    auto& ps = m_player.getComponent<CState>();
    ps.onAir = true;

    for (auto e : m_entityManager.getEntities("tile")) {
        auto o = Physics::GetOverlap(m_player, e);

        // Коллизии нет
        if (o.x == 0 || o.y == 0) {
            continue;
        }

        // player x heart tile
        if (e.hasComponent<CArtefact>()) {
            m_game->inventory().addArtefact(e.getComponent<CArtefact>().artefact);
            e.destroy();
            m_game->assets().getSound("GetItem")->play();
        }

        // player x Pole tile (finish)
        if (e.hasAnimation("Pole")) {
            m_sWin.win = true;
            clearActions();
            m_player.addComponent<CInput>(); // затираем ввод
        }

        if (e.hasComponent<CTeleport>() && abs(o.x) > 32 && abs(o.y) > 32) {
            auto lvl = e.getComponent<CTeleport>().level;
            auto levels = m_game->getSettings().levels;

            if (lvl >= 0 && lvl < (int)levels.size() && !e.getComponent<CBoundingBox>().blockMove) {
                m_sTeleport.level = lvl;
            }
        }

        if (!e.getComponent<CBoundingBox>().blockMove) {
            continue;
        }

        auto p = Physics::GetPreviousOverlap(m_player, e);
        std::string direction = "";

        if (p.y != 0) {
            direction = pt.velocity.x > 0 ? "right" : "left";
            pt.move.x = pt.velocity.x + o.x;
            pt.velocity.x = e.getComponent<CTransform>().velocity.x;
        }

        if (p.x != 0) {
            direction = pt.velocity.y > 0 ? "down" : "up";
            pt.move.y = pt.velocity.y + o.y - e.getComponent<CTransform>().velocity.y;
            pt.velocity.y = e.getComponent<CTransform>().velocity.y;
            if (!pi.left && !pi.right) {
                pt.velocity.x = e.getComponent<CTransform>().velocity.x;
            }
        }

        if (direction == "down") {
            ps.onAir = false;
            ps.run = (pi.left || pi.right);
        }

        if (e.hasAnimation("Brick") && direction == "up") {
            e.addComponent<CAnimation>(m_game->assets().getAnimation("Explosion"), false);
            e.removeComponent<CBoundingBox>();
            m_game->assets().getSound("BreakBrick")->play();
        }

        if (e.hasComponent<CArtefactSpawner>() && direction == "up") {
            auto artName = e.getComponent<CArtefactSpawner>().getArtefact();
            spawnArtefact(e, artName);
            m_game->inventory().addArtefact(artName);
            e.addComponent<CAnimation>(m_game->assets().getAnimation("Question2"), true);
            e.removeComponent<CArtefactSpawner>();
            m_game->assets().getSound("GetItem")->play();
        }
    }

    for (auto b : m_entityManager.getEntities("weapon")) {
        // weapon x tile
        for (auto e : m_entityManager.getEntities("tile")) {
            if (!e.getComponent<CBoundingBox>().blockMove) {
                continue;
            }

            auto o = Physics::GetOverlap(b, e);

            // Коллизии нет
            if (o.x == 0 || o.y == 0) {
                continue;
            }

            b.destroy();

            if (e.hasAnimation("Brick")) {
                e.addComponent<CAnimation>(m_game->assets().getAnimation("Explosion"), false);
                e.removeComponent<CBoundingBox>();
                m_game->assets().getSound("BreakBrick")->play();
            }

            break;
        }

        if (!b.isActive()) {
            continue;
        }

        // weapon x npc
        for (auto n : m_entityManager.getEntities("npc")) {
            auto o = Physics::GetOverlap(b, n);
            if (o.x != 0 && o.y != 0) {
                b.destroy();

                if (n.hasComponent<CInvincibility>() && n.getComponent<CInvincibility>().iframes > 0) {
                    continue;
                }

                n.getComponent<CHealth>().current -= m_player.getComponent<CDamage>().damage * m_game->getPlayerDamageCoeff();
                n.addComponent<CInvincibility>(30);
                if (n.getComponent<CHealth>().current > 0) {
                    m_game->assets().getSound("EnemyHit")->play();
                }

                break;
            }
        }
    }

    auto checkPlayer = (!m_player.hasComponent<CInvincibility>() || m_player.getComponent<CInvincibility>().iframes <= 0);

    for (auto n : m_entityManager.getEntities("npc")) {
        // npc x player
        if (checkPlayer) {
            auto o = Physics::GetOverlap(m_player, n);

            if (o.x != 0 && o.y != 0) {
                m_player.getComponent<CHealth>().current -= n.getComponent<CDamage>().damage * m_game->getNpcDamageCoeff();
                m_player.addComponent<CInvincibility>(30);
                if (m_player.getComponent<CHealth>().current > 0) {
                    m_game->assets().getSound("LinkHit")->play();
                }

                if (n.getComponent<CBoundingBox>().blockMove) {
                    auto p = Physics::GetPreviousOverlap(m_player, n);
                    std::string direction = "";

                    if (p.y != 0) {
                        direction = pt.velocity.x > 0 ? "right" : "left";
                        pt.move.x = pt.velocity.x + o.x;
                        pt.velocity.x = n.getComponent<CTransform>().velocity.x;
                    }

                    if (p.x != 0) {
                        direction = pt.velocity.y > 0 ? "down" : "up";
                        pt.move.y = pt.velocity.y + o.y - n.getComponent<CTransform>().velocity.y;
                        pt.velocity.y = n.getComponent<CTransform>().velocity.y;
                        if (!pi.left && !pi.right) {
                            pt.velocity.x = n.getComponent<CTransform>().velocity.x;
                        }
                    }

                    if (direction == "down") {
                        ps.onAir = false;
                        ps.run = (pi.left || pi.right);
                    }
                }
            }
        }

        for (auto t : m_entityManager.getEntities("tile")) {
            auto o = Physics::GetOverlap(n, t);
            if (o.x == 0 || o.y == 0) {
                continue;
            }

            if (t.hasComponent<CBoundingBox>() && t.getComponent<CBoundingBox>().blockMove) {
                auto po = Physics::GetPreviousOverlap(n, t);
                auto& nt = n.getComponent<CTransform>();

                if (po.x != 0) { nt.velocity.y += o.y; }
                if (po.y != 0) { nt.velocity.x += o.x; }
            }

            if (t.hasAnimation("Heart")) {
                auto& h = n.getComponent<CHealth>();
                if (h.current < h.max) {
                    h.current = h.max;
                    t.destroy();
                    m_game->assets().getSound("GetItem")->play();
                }
            }
        }
    }
}

void Scene_Play::sDoAction(const Action &action) {
    PROFILE_FUNCTION();

    if (action.type() == "START") {
             if (action.name() == "TOGGLE_TEXTURE")   { m_drawTextures = !m_drawTextures; }
        else if (action.name() == "TOGGLE_DEBUG")     { m_drawDebug = !m_drawDebug; }
        else if (action.name() == "TOGGLE_GRID")      { m_drawGrid = !m_drawGrid; }
        else if (action.name() == "TOGGLE_INVENTORY") {
            m_game->inventory().drawInventory(!m_game->inventory().drawInventory());
            if (m_game->inventory().drawInventory()) {
                m_game->assets().getSound("Inventory")->play();
            }
        } else if (action.name() == "RELOAD")           { reload(); }
        else if (action.name() == "PAUSE")            { setPaused(!m_paused); }
        else if (action.name() == "QUIT")             { m_game->changeScene("MENU", std::make_shared<Scene_Menu>(m_game)); }
        else if (action.name() == "MOVE_LEFT")        { m_player.getComponent<CInput>().left = true; }
        else if (action.name() == "MOVE_RIGHT")       { m_player.getComponent<CInput>().right = true; }
        else if (action.name() == "MOVE_UP")          { m_player.getComponent<CInput>().up = true; }
        else if (action.name() == "SHOOT")            { m_player.getComponent<CInput>().shoot = true; }

        else if (action.name() == "INV_NUM0")         { if (!m_player.getComponent<CInput>().numPressed) { m_player.getComponent<CInput>().num = 0; m_player.getComponent<CInput>().numPressed = true; }}
        else if (action.name() == "INV_NUM1")         { if (!m_player.getComponent<CInput>().numPressed) { m_player.getComponent<CInput>().num = 1; m_player.getComponent<CInput>().numPressed = true; }}
        else if (action.name() == "INV_NUM2")         { if (!m_player.getComponent<CInput>().numPressed) { m_player.getComponent<CInput>().num = 2; m_player.getComponent<CInput>().numPressed = true; }}
        else if (action.name() == "INV_NUM3")         { if (!m_player.getComponent<CInput>().numPressed) { m_player.getComponent<CInput>().num = 3; m_player.getComponent<CInput>().numPressed = true; }}
        else if (action.name() == "INV_NUM4")         { if (!m_player.getComponent<CInput>().numPressed) { m_player.getComponent<CInput>().num = 4; m_player.getComponent<CInput>().numPressed = true; }}
        else if (action.name() == "INV_NUM5")         { if (!m_player.getComponent<CInput>().numPressed) { m_player.getComponent<CInput>().num = 5; m_player.getComponent<CInput>().numPressed = true; }}
        else if (action.name() == "INV_NUM6")         { if (!m_player.getComponent<CInput>().numPressed) { m_player.getComponent<CInput>().num = 6; m_player.getComponent<CInput>().numPressed = true; }}
        else if (action.name() == "INV_NUM7")         { if (!m_player.getComponent<CInput>().numPressed) { m_player.getComponent<CInput>().num = 7; m_player.getComponent<CInput>().numPressed = true; }}
        else if (action.name() == "INV_NUM8")         { if (!m_player.getComponent<CInput>().numPressed) { m_player.getComponent<CInput>().num = 8; m_player.getComponent<CInput>().numPressed = true; }}
        else if (action.name() == "INV_NUM9")         { if (!m_player.getComponent<CInput>().numPressed) { m_player.getComponent<CInput>().num = 9; m_player.getComponent<CInput>().numPressed = true; }}

        else if (action.name() == "LEFT_CLICK") {
            auto mp = action.pos();

            for (auto e : m_entityManager.getEntities()) {
                if (e.hasComponent<CDraggable>() && Physics::IsInside(mp, e)) {
                    e.getComponent<CDraggable>().dragging = true;
                }
            }
        } else if (action.name() == "MOUSE_MOVE") {
            m_mousePos = action.pos();
        } else if (action.name() == "WINDOW_RESIZED") {
            auto delta = action.pos() / 2;

            auto playerX = m_player.getComponent<CTransform>().pos.x;
            auto windowX = m_game->getWindowSize().x / 2.0;
            auto x = std::max(playerX, (float)windowX);

            storeViewCenter(Vec2(x, getStoredViewCenter().y - delta.y));

            m_sParallax.startViewPosX += delta.x;
            m_sParallax.startPlayerPosY -= delta.y;
        } else if (action.name() == "TOGGLE_SLOW") {
            m_slow = !m_slow;
            setFPS();
        }
    } else if (action.type() == "END") {
             if (action.name() == "MOVE_LEFT")        { m_player.getComponent<CInput>().left = false; }
        else if (action.name() == "MOVE_RIGHT")       { m_player.getComponent<CInput>().right = false; }
        else if (action.name() == "MOVE_UP")          { m_player.getComponent<CInput>().up = false; }
        else if (action.name() == "SHOOT")            { m_player.getComponent<CInput>().shoot = false; }

        else if (action.name() == "INV_NUM0")         { m_player.getComponent<CInput>().num = -1; m_player.getComponent<CInput>().numPressed = false; }
        else if (action.name() == "INV_NUM1")         { m_player.getComponent<CInput>().num = -1; m_player.getComponent<CInput>().numPressed = false; }
        else if (action.name() == "INV_NUM2")         { m_player.getComponent<CInput>().num = -1; m_player.getComponent<CInput>().numPressed = false; }
        else if (action.name() == "INV_NUM3")         { m_player.getComponent<CInput>().num = -1; m_player.getComponent<CInput>().numPressed = false; }
        else if (action.name() == "INV_NUM4")         { m_player.getComponent<CInput>().num = -1; m_player.getComponent<CInput>().numPressed = false; }
        else if (action.name() == "INV_NUM5")         { m_player.getComponent<CInput>().num = -1; m_player.getComponent<CInput>().numPressed = false; }
        else if (action.name() == "INV_NUM6")         { m_player.getComponent<CInput>().num = -1; m_player.getComponent<CInput>().numPressed = false; }
        else if (action.name() == "INV_NUM7")         { m_player.getComponent<CInput>().num = -1; m_player.getComponent<CInput>().numPressed = false; }
        else if (action.name() == "INV_NUM8")         { m_player.getComponent<CInput>().num = -1; m_player.getComponent<CInput>().numPressed = false; }
        else if (action.name() == "INV_NUM9")         { m_player.getComponent<CInput>().num = -1; m_player.getComponent<CInput>().numPressed = false; }

        else if (action.name() == "LEFT_CLICK") {
            for (auto e : m_entityManager.getEntities()) {
                if (e.hasComponent<CDraggable>() && e.getComponent<CDraggable>().dragging) {
                    e.getComponent<CDraggable>().dragging = false;
                    e.getComponent<CTransform>().velocity = {0, 0};
                }
            }
        }
    }
}

void Scene_Play::sAnimation() {
    PROFILE_FUNCTION();

    // TODO: Complete the Animation class code first
    // TODO: set the animation of the player based on its CState component
    // TODO: for each entity with an animation, call entity->getComponent<CAnimation>().animation.update()
    //       if the animation is not repeated, and it has ended, destroy the entity

    auto s = m_player.getComponent<CState>();
    auto scale = m_player.getComponent<CTransform>().scale;
    scale.x = abs(scale.x);
    scale.y = abs(scale.y);

    if (s.direction == Direction::LEFT) {
        scale.x = -scale.x;
    }

    if (s.onAir) {
        setAnimation(m_player, "RunRight", true, scale);
    } else if (s.run) {
        setAnimation(m_player, "RunRight", true, scale);
    } else {
        setAnimation(m_player, "StandRight", true, scale);
    }

    for (auto e : m_entityManager.getEntities()) {
        if (e.hasComponent<CAnimation>()) {
            auto& a = e.getComponent<CAnimation>();
            if (!a.repeat && a.animation.hasEnded()) {
                e.destroy();
            } else {
                a.animation.update();
            }
        }
    }
}

void Scene_Play::sRender() {
    PROFILE_FUNCTION();

    m_game->window().clear(m_bgColor);
    m_game->getVertexArrays().clearAll();

    // draw all Entity textures / animations
    if (m_drawTextures) {
        PROFILE_SCOPE("drawTextures");

        for (auto e : m_entityManager.getEntities()) {
            auto & transform = e.getComponent<CTransform>();

            if (e.hasComponent<CAnimation>()) {
                auto& animation = e.getComponent<CAnimation>().animation;
                animation.getSprite().setRotation(sf::degrees(transform.angle));
                animation.getSprite().setScale({ transform.scale.x, transform.scale.y });
                auto c = animation.getSprite().getColor();
                c.a = (e.hasComponent<CInvincibility>()) ? animation.getAlpha()/2 : animation.getAlpha();
                animation.getSprite().setColor(c);

                int repeat = (e.hasComponent<CParallax>()) ? e.getComponent<CParallax>().repeat : 1;

                for (int i=0; i<repeat; i++) {
                    animation.getSprite().setPosition({ transform.pos.x + i * e.getSize().x, transform.pos.y });

                    if (auto shaderName = animation.getShader(); shaderName != "") {
                        auto shader = m_game->assets().getShader(shaderName);
                        shader->setUniform("u_alpha", ((float)c.a)/255.0f);
                        shader->setUniform("u_mix", animation.getMix());
                        shader->setUniform("u_resolution", sf::Vector2f(m_game->window().getSize().x, m_game->window().getSize().y));
                        shader->setUniform("u_time", (float)m_currentFrame/60);
                        shader->setUniform("texture", sf::Shader::CurrentTexture);
                        m_game->getVertexArrays().draw(animation.getSprite(), shaderName, m_game->assets().getLargeTexture(), shader);
                    } else {
                        m_game->getVertexArrays().draw(animation.getSprite(), "", m_game->assets().getLargeTexture());
                    }
                }
            }

            if (e.hasComponent<CHealth>()) {
                auto& h = e.getComponent<CHealth>();
                int health = (h.current / h.max) * 100;
                if (health < 0) { health = 0; }
                auto a = m_game->assets().getAnimation("healthBar" + std::to_string(health));
                auto pos = Vec2(transform.pos.x, transform.pos.y - e.getSize().y / 2.0 - a.getTexture().size.y - 10.0);
                a.getSprite().setPosition(pos);
                m_game->getVertexArrays().draw(a.getSprite(), "", m_game->assets().getLargeTexture());
            }
        }

        m_game->getVertexArrays().drawAll(m_game->window());
    }

    // draw the grid so that students can easily debug
    if (m_drawGrid) {
        PROFILE_SCOPE("drawGrid");

        auto wh = m_game->getWindowSize();
        float leftX = m_game->getViewCenter().x - wh.x / 2.0;
        float rightX = leftX + wh.x + m_gridSize.x;
        float nextGridX = leftX - ((int)leftX % (int)m_gridSize.x);

        for (float x = nextGridX; x < rightX; x += m_gridSize.x) {
            drawLine(Vec2(x, 0), Vec2(x, wh.y));
        }

        for (float y = 0; y < wh.y; y += m_gridSize.y) {
            drawLine(Vec2(leftX, wh.y-y), Vec2(rightX, wh.y-y));

            for (float x = nextGridX; x < rightX; x += m_gridSize.x) {
                std::string xCell = std::to_string((int)x / (int)m_gridSize.x);
                std::string yCell = std::to_string((int)y / (int)m_gridSize.y);
                m_gridText.setString("(" + xCell + "," + yCell + ")");
                m_gridText.setPosition({x + 3, wh.y - y - m_gridSize.y + 2});
                m_game->window().draw(m_gridText);
            }
        }
    }

    // draw all Entity collision bounding boxes with a rectangle shape
    if (m_drawDebug) {
        PROFILE_SCOPE("drawDebug");

        sf::RectangleShape rect;
        rect.setFillColor(sf::Color(0, 0, 0, 0));
        rect.setOutlineThickness(1);

        sf::CircleShape c;
        c.setRadius(4);
        c.setOrigin(sf::Vector2f(2, 2));

        { PROFILE_SCOPE("drawCollision");
        for (auto e : m_entityManager.getEntities()) {
            if (e.hasComponent<CBoundingBox>()) {
                auto blockMove = e.getComponent<CBoundingBox>().blockMove;
                auto blockVision = e.getComponent<CBoundingBox>().blockVision;
                auto box = e.getBoundingBox();
                auto & transform = e.getComponent<CTransform>();

                rect.setSize(sf::Vector2f(box.x-1, box.y-1));
                rect.setOrigin(sf::Vector2f(box.x / 2.0, box.y / 2.0));
                rect.setPosition({transform.pos.x, transform.pos.y});

                if (blockMove && blockVision) { rect.setOutlineColor(sf::Color(0,255,255)); }
                if (blockMove && !blockVision) { rect.setOutlineColor(sf::Color(0,255,0)); }
                if (!blockMove && blockVision) { rect.setOutlineColor(sf::Color(0,0,255)); }
                if (!blockMove && !blockVision) { rect.setOutlineColor(sf::Color(0,0,0)); }

                m_game->window().draw(rect);
            }

            if (e.hasComponent<CPatrol>()) {
                for (auto p : e.getComponent<CPatrol>().positions) {
                    c.setPosition(sf::Vector2f(p.x, p.y));
                    c.setFillColor(sf::Color::Black);
                    m_game->window().draw(c);
                }
            }

            if (e.hasComponent<CFollowPlayer>()) {
                auto playerPos = m_player.getComponent<CTransform>().pos;
                auto entityPos = e.getComponent<CTransform>().pos;

                auto p = e.getComponent<CFollowPlayer>().home;
                c.setPosition(sf::Vector2f(p.x, p.y));
                c.setFillColor(sf::Color::Red);
                m_game->window().draw(c);

                bool intersect = false;
                // 10ms!
                for (auto x : m_entityManager.getEntities()) {
                    if (x.tag() == "player" || x.id() == e.id() || !x.getComponent<CBoundingBox>().blockVision) {
                        continue;
                    }

                    if (Physics::EntityIntersect(playerPos, entityPos, x)) {
                        intersect = true;
                        break;
                    }
                }

                if (intersect) {
                    drawLine(playerPos, entityPos, sf::Color::Black);
                } else {
                    drawLine(playerPos, entityPos, sf::Color::Red);
                }
            }
        }}
    }

    m_playerState.setPosition(m_game->p2c(Vec2(10, 10)));
    auto s = m_player.getComponent<CState>();
    if (s.onAir) {
        m_playerState.setString("STATE: air");
    } else if (s.run) {
        m_playerState.setString("STATE: run");
    } else {
        m_playerState.setString("STATE: stand");
    }
    m_game->window().draw(m_playerState);

    if (m_currentFrame % 60 == 0) {
        m_frameTime.setString(std::to_string(m_game->getFrameTime()) + " ms/frame");
    }
    m_frameTime.setPosition(m_game->p2c(Vec2(250, 10)));
    m_game->window().draw(m_frameTime);

    // ======================================== ray castings

    Entity view;
    if (auto items = m_entityManager.getEntities("view"); items.size() != 0) {
        view = items.at(0);
    } else {
        view = m_entityManager.addEntity("view");
        view.addComponent<CTransform>(Vec2(0 ,0), 1);
        view.addComponent<CBoundingBox>(Vec2(m_game->window().getView().getSize()), false, false);
    }

    view.getComponent<CTransform>().pos = Vec2(m_game->window().getView().getCenter());

    std::vector<Entity> items;
    std::vector<Vec2> rays;

    // Собираем сущности, отображаемые на экране, и лучи, идущие в углы сущностей
    for (auto e : m_entityManager.getEntities()) {
        if (e.tag() == "weapon" || !e.hasComponent<CBoundingBox>()) {
            continue;
        }

        auto ov = Physics::GetOverlap(view, e);
        if (ov.x == 0 || ov.y == 0) {
            continue;
        }

        if (e.getComponent<CBoundingBox>().blockVision || e.tag() == "view") {
            items.push_back(e);
        }

        auto ec = e.getComponent<CTransform>().pos;
        auto es = e.getComponent<CBoundingBox>().halfSize;
        auto a = Vec2(ec.x - es.x, ec.y - es.y);
        auto b = Vec2(ec.x + es.x, ec.y - es.y);
        auto c = Vec2(ec.x + es.x, ec.y + es.y);
        auto d = Vec2(ec.x - es.x, ec.y + es.y);

        rays.push_back(a);
        rays.push_back(b);
        rays.push_back(c);
        rays.push_back(d);
    }

    auto dev = 0.001f;

    auto ec = view.getComponent<CTransform>().pos;
    auto es = view.getComponent<CBoundingBox>().halfSize;
    auto p1 = Vec2(ec.x - es.x, ec.y - es.y);
    auto p2 = Vec2(ec.x + es.x, ec.y - es.y);
    auto p3 = Vec2(ec.x + es.x, ec.y + es.y);
    auto p4 = Vec2(ec.x - es.x, ec.y + es.y);

    struct Line { Vec2 p1, p2; };
    std::vector<Line> lines1 = {{p1, p2}, {p2, p3}, {p3, p4}, {p4, p1}};

    // Добавляем в список лучей лучи, идущие в точки пересечения сущностей с границей экрана
    for (auto e : items) {
        auto ec = e.getComponent<CTransform>().pos;
        auto es = e.getComponent<CBoundingBox>().halfSize;
        auto a = Vec2(ec.x - es.x, ec.y - es.y);
        auto b = Vec2(ec.x + es.x, ec.y - es.y);
        auto c = Vec2(ec.x + es.x, ec.y + es.y);
        auto d = Vec2(ec.x - es.x, ec.y + es.y);

        std::vector<Line> lines2 = {{a, b}, {b, c}, {c, d}, {d, a}};

        for (auto line1 : lines1) {
            for (auto line2 : lines2) {
                if (auto res = Physics::LineIntersect(line1.p1, line1.p2, line2.p1, line2.p2, dev); res.status) {
                    rays.push_back(res.result);
                }
            }
        }
    }

    for (auto weapon : m_entityManager.getEntities("weapon")) {
        if (auto art = m_game->inventory().getWeapon(); !art.has_value() || art->name != "light_arrow") {
            break;
        }

        auto pos = weapon.getComponent<CTransform>().pos;

        std::vector<Vec2> rays2;

        for (auto& r : rays) {
            bool drop = false;

            // Удаляем лучи, которые находятся пересекаются с объектами
            for (auto e : items) {
                auto ec = e.getComponent<CTransform>().pos;
                auto es = e.getComponent<CBoundingBox>().halfSize;
                auto a = Vec2(ec.x - es.x, ec.y - es.y);
                auto b = Vec2(ec.x + es.x, ec.y - es.y);
                auto c = Vec2(ec.x + es.x, ec.y + es.y);
                auto d = Vec2(ec.x - es.x, ec.y + es.y);

                if (auto res = Physics::LineIntersect(pos, r, a, b, dev); res.status && r != a && r != b) {
                    drop = true;
                    break;
                }

                if (auto res = Physics::LineIntersect(pos, r, b, c, dev); res.status && r != b && r != c) {
                    drop = true;
                    break;
                }

                if (auto res = Physics::LineIntersect(pos, r, c, d, dev); res.status && r != c && r != d) {
                    drop = true;
                    break;
                }

                if (auto res = Physics::LineIntersect(pos, r, d, a, dev); res.status && r != d && r != a) {
                    drop = true;
                    break;
                }
            }

            if (!drop || true) {
                auto s = (r-pos); // Приводим к началу координат
                s /= s.length();  // Нормализуем

                // Строим перпендикуляр
                auto v = Vec2(abs(s.y), abs(s.x));
                if (s.x < 0) { v.x = -v.x; }
                if (s.y < 0) { v.y = -v.y; }

                v *= 2; // Задаем длину перпендикуляра

                // Строим два луча - слева и справа от основного
                static std::vector<Vec2> rs(2);
                rs[0] = r + v;
                rs[1] = r - v;

                // Удлиняем лучи до размера большего, чем экран
                for (auto& r : rs) {
                    r = (r - pos) * 10000 + pos;
                }

                // Укорачиваем лучи до ближайшего препятствия
                for (auto r : rs) {
                    for (auto e : items) {
                        auto ec = e.getComponent<CTransform>().pos;
                        auto es = e.getComponent<CBoundingBox>().halfSize;
                        auto a = Vec2(ec.x - es.x, ec.y - es.y);
                        auto b = Vec2(ec.x + es.x, ec.y - es.y);
                        auto c = Vec2(ec.x + es.x, ec.y + es.y);
                        auto d = Vec2(ec.x - es.x, ec.y + es.y);

                        if (auto res = Physics::LineIntersect(pos, r, a, b); res.status && pos.dist(res.result) < pos.dist(r)) {
                            r = res.result;
                        }

                        if (auto res = Physics::LineIntersect(pos, r, b, c); res.status && pos.dist(res.result) < pos.dist(r)) {
                            r = res.result;
                        }

                        if (auto res = Physics::LineIntersect(pos, r, c, d); res.status && pos.dist(res.result) < pos.dist(r)) {
                            r = res.result;
                        }

                        if (auto res = Physics::LineIntersect(pos, r, d, a); res.status && pos.dist(res.result) < pos.dist(r)) {
                            r = res.result;
                        }
                    }

                    rays2.push_back(r);
                }
            }
        }

        auto v0 = Vec2(pos.x, -10000) - pos;
        auto v0len = v0.length();

        // Сортируем лучи по углу
        std::sort(rays2.begin(), rays2.end(), [v0, v0len, pos](const Vec2 & a, const Vec2 & b) -> bool {
            auto v1 = a-pos;
            auto v2 = b-pos;
            auto a1 = sf::radians(acosf((v0.x * v1.x + v0.y * v1.y) / (v0len * v1.length()))).asDegrees();
            auto a2 = sf::radians(acosf((v0.x * v2.x + v0.y * v2.y) / (v0len * v2.length()))).asDegrees();

            if (v1.x < v0.x) { a1 = 360-a1; }
            if (v2.x < v0.x) { a2 = 360-a2; }

            return a1 < a2;
        });

        auto c = sf::Color::Cyan;
        c.a = 30;

        if (rays2.size() > 0) { rays2.push_back(rays2[0]); }

        for (size_t i = 0; i<rays2.size()-1; i++) {
            auto p0 = pos;
            auto p1 = rays2[i];
            auto p2 = rays2[i+1];
            // drawLine(p0, p1, c);
            // drawLine(p0, p2, c);
            // drawLine(p1, p2, c);

            sf::ConvexShape triangle;
            triangle.setPointCount(3);
            triangle.setPoint(0, p0);
            triangle.setPoint(1, p1);
            triangle.setPoint(2, p2);
            triangle.setFillColor(c);
            m_game->window().draw(triangle);
        }
    }

    // ======================================== ray castings

    m_game->inventory().draw(m_game->window(), m_game->assets());
}

void Scene_Play::setPaused(bool v) {
    m_game->assets().getSound(v ? "PauseIn" : "PauseOut")->play();
    m_paused = v;
};

void Scene_Play::reload() {
    PROFILE_FUNCTION();

    m_game->changeScene("PLAY", std::make_shared<Scene_Play>(m_game, m_level), true);
}

void Scene_Play::sDragAndDrop() {
    PROFILE_FUNCTION();

    for (auto e : m_entityManager.getEntities()) {
        if (e.hasComponent<CDraggable>() && e.getComponent<CDraggable>().dragging) {
            auto& t = e.getComponent<CTransform>();
            t.velocity = Vec2(0, 0); // m_mousePos-t.pos;
            t.pos = m_mousePos;
        }
    }
}

void Scene_Play::sCamera() {
    PROFILE_FUNCTION();

    // set the viewport of the window to be centered on the player if it's far enough right
    if (!m_player.hasComponent<CDraggable>() || !m_player.getComponent<CDraggable>().dragging) {
        auto vc = getStoredViewCenter();

        vc.x = std::max(
            m_leftBottomOfLevel.x + m_game->getWindowSize().x / 2.0f,
            m_player.getComponent<CTransform>().pos.x
        );
        storeViewCenter(vc);

        m_game->setViewCenter(vc);
    }
}

void Scene_Play::setAnimation(Entity e, const std::string & animName, bool repeat, const Vec2 & scale) {
    if (e.getComponent<CTransform>().scale.x != scale.x) {
        e.getComponent<CTransform>().scale.x = scale.x;
    }

    if (e.getComponent<CTransform>().scale.y != scale.y) {
        e.getComponent<CTransform>().scale.y = scale.y;
    }

    if (e.hasComponent<CAnimation>()) {
        if (e.getComponent<CAnimation>().animation.getName() == animName) {
            return;
        }
    }

    e.addComponent<CAnimation>(m_game->assets().getAnimation(animName), repeat);
}

void Scene_Play::setBoundingBox(Entity e, Vec2 bb, bool blockMove, bool blockVision) {
    if (!e.hasComponent<CBoundingBox>() || e.getComponent<CBoundingBox>().size != bb || e.getComponent<CBoundingBox>().blockMove != blockMove || e.getComponent<CBoundingBox>().blockVision != blockVision) {
        e.addComponent<CBoundingBox>(Vec2(2, 8), true, false);
    }
}

void Scene_Play::sParallax() {
    PROFILE_FUNCTION();

    auto deltaX = m_game->getViewCenter().x - m_sParallax.startViewPosX;
    auto deltaY = m_player.getComponent<CTransform>().pos.y - m_sParallax.startPlayerPosY;

    const float DISTANCE = -5;

    for (auto e : m_entityManager.getEntities()) {
        if (!e.hasComponent<CParallax>()) { continue; }

        auto& p = e.getComponent<CParallax>();
        auto& t = e.getComponent<CTransform>();
        auto& a = e.getComponent<CAnimation>();

        // shift
        t.pos.x = p.startPos.x + (t.z / DISTANCE) * deltaX;
        t.pos.y = p.startPos.y + (t.z / DISTANCE) * deltaY * 0.0025;

        // alpha
        a.animation.setAlpha(200 + (1 - t.z / DISTANCE) * 55);
    }
}

void Scene_Play::sWin() {
    if (!m_sWin.win) {
        return;
    }

    PROFILE_FUNCTION();

    // TODO remove it
    // m_game->changeScene("PLAY", std::make_shared<Scene_World>(m_game), true);
    // return;

    if (m_sWin.curtain == nullptr) {
        m_game->assets().getSound("LevelWin")->play();
        auto e = m_entityManager.addEntity("tile");
        e.addComponent<CAnimation>(m_game->assets().getAnimation("Curtain"), true);
        e.getComponent<CAnimation>().animation.setAlpha(0);
        e.getComponent<CAnimation>().animation.setMix(0.0);
        
        auto vc = Vec2(m_game->window().getView().getCenter().x, m_game->window().getView().getCenter().y);
        //auto vs = Vec2(m_game->window().getView().getSize().x, m_game->window().getView().getSize().y);
        e.addComponent<CTransform>(vc-5);

        auto& t = e.getComponent<CTransform>();
        t.scale = Vec2(500, 500);
        t.z = 999;

        m_sWin.curtain = std::make_shared<Entity>(e);
    }

    switch (m_sWin.state) {
        case SWin::State::increaseAlpha:
            if (m_sWin.counter == 20 && m_level.shader != "") {
                m_sWin.curtain->getComponent<CAnimation>().animation.setShader(m_level.shader);
            }

            if (m_sWin.counter < 255) {
                m_sWin.curtain->getComponent<CAnimation>().animation.setAlpha(m_sWin.counter);
            }

            if (m_sWin.counter >= 300) {
                m_sWin.state = SWin::State::hideShader;
                m_sWin.counter = 0;
            }
            break;

        case SWin::State::hideShader: {
            auto m = m_sWin.curtain->getComponent<CAnimation>().animation.getMix() + 0.002;
            if (m <= 1.0) {
                m_sWin.curtain->getComponent<CAnimation>().animation.setMix(m);
            } else {
                m_sWin.state = SWin::State::delay;
                m_sWin.counter = 0;
            }}
            break;

        case SWin::State::delay:
            if (m_sWin.counter >= 120) {
                m_level.pass = true;
                m_game->saveSettings();

                bool finish = true;
                for (auto lvl : m_game->getSettings().levels) {
                    if (!lvl.pass && !lvl.world) {
                        finish = false;
                        break;
                    }
                }

                if (finish) {
                    m_game->changeScene("WIN", std::make_shared<Scene_Win>(m_game), true);
                } else {
                    if (auto lvl = m_game->getSettings().getFirstWorldLevel(); lvl.has_value()) {
                        m_game->changeScene("PLAY", std::make_shared<Scene_World>(m_game, lvl->path), true);
                    }
                }

                return;
            }
            break;
    }

    m_sWin.counter += 2;
}

void Scene_Play::onEnd() {
    PROFILE_FUNCTION();
    EntityMemoryPool::Instance().clear();
}

const std::string & Scene_Play::levelPath() const {
    return m_level.path;
}

void Scene_Play::sTeleport() {
    PROFILE_FUNCTION();

    auto& levels = m_game->getSettings().levels;

    for (auto e : m_entityManager.getEntities()) {
        if (e.hasComponent<CTeleport>() && e.getComponent<CTeleport>().level >= 0 && e.getComponent<CTeleport>().level < (int)levels.size()) {
            auto lvl = e.getComponent<CTeleport>().level;
            auto prevLvl = lvl - 1;
            bool prevLevelPass = (prevLvl < 0) || levels[prevLvl].pass;

            // Открываем доступ к порталу только если предыдущий уровень пройден а текщий нет
            auto pass = prevLevelPass && !levels[lvl].pass;

            e.getComponent<CAnimation>().animation.setAlpha(pass ? 255 : 64);
            e.getComponent<CBoundingBox>().blockMove = !pass;
        }
    }

    if (m_sTeleport.level < 0) {
        return;
    }

    // TODO remove it
    // m_player.getComponent<CTransform>().pos.y += 70;
    // m_player.getComponent<CState>().direction = Direction::DOWN;
    // m_game->changeScene("PLAY", std::make_shared<Scene_Play>(m_game, m_game->getSettings().levels[m_sTeleport.level]), true);
    // return;

    if (m_sTeleport.curtain == nullptr) {
        auto e = m_entityManager.addEntity("tile");
        e.addComponent<CAnimation>(m_game->assets().getAnimation("Curtain"), true);
        e.getComponent<CAnimation>().animation.setAlpha(0);

        auto vc = Vec2(m_game->window().getView().getCenter().x, m_game->window().getView().getCenter().y);
        //auto vs = Vec2(m_game->window().getView().getSize().x, m_game->window().getView().getSize().y);
        e.addComponent<CTransform>(vc-5);

        auto& t = e.getComponent<CTransform>();
        t.scale = Vec2(500, 500);
        t.z = 999;

        m_sTeleport.curtain = std::make_shared<Entity>(e);
    }

    switch (m_sTeleport.state) {
        case STeleport::State::increaseAlpha:
            if (m_sTeleport.counter < 255) {
                m_sTeleport.curtain->getComponent<CAnimation>().animation.setAlpha(m_sTeleport.counter);
            }

            if (m_sTeleport.counter >= 300) {
                m_sTeleport.state = STeleport::State::delay;
                m_sTeleport.counter = 0;
            }
            break;

        case STeleport::State::delay:
            if (m_sTeleport.counter >= 60) {
                m_player.getComponent<CTransform>().pos.y += 70;
                m_player.getComponent<CState>().direction = Direction::DOWN;

                m_game->changeScene("PLAY", std::make_shared<Scene_Play>(m_game, levels[m_sTeleport.level]), true);
                return;
            }
            break;
    }

    m_sTeleport.counter += 2;
}

void Scene_Play::sInventory() {
    auto& num = m_player.getComponent<CInput>().num;
    if (num < 0) { return; }

    if (num == 0) {
        m_game->inventory().clearSelections();
        num = -1;
        return;
    }

    if (m_game->inventory().drawInventory()) {
        m_game->inventory().selectArtefact(num-1);
    } else {
        m_game->inventory().useArtefact(num-1, m_player);
    }

    num = -1;
}
