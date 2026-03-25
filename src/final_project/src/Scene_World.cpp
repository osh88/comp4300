#include "Scene_World.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Graphics/Color.hpp"
#include "Scene_Menu.h"
#include "Profiler.h"
#include "Scene_Play.h"
#include "Level.h"

#include <math.h>
#include <fstream>
#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()
#include <iostream>

Scene_World::Scene_World()
    : m_gridText(f)
    , m_frameTime(f)
{};

Scene_World::Scene_World(GameEngine * game, const std::string & levelPath)
    : Scene(game)
    , m_levelPath(levelPath)
    , m_gridText(m_game->assets().getFont("Tech").font, "0", 12)
    , m_frameTime(m_game->assets().getFont("Mario").font, "0", 16)
{}

void Scene_World::init() {
    PROFILE_FUNCTION();

    m_bgColor = sf::Color(255, 192, 122);
    srand(time(0));

    if (!loadState()) {
        Level::loadFromFile(m_game->getExeDir() + m_levelPath, m_game, m_entityManager, m_bgColor);
    }

    // ======================================== ray castings
    // Удаляем сохраненные точки
    for (auto d : m_entityManager.getEntities("dot")) {
        d.destroy();
    }
    for (auto d : m_entityManager.getEntities("view")) {
        d.destroy();
    }
    // ======================================== ray castings

    auto it = m_entityManager.getEntities("player");
    if (it.size() == 1) {
        m_player = it.at(0);
        m_playerConfig.startPos = m_player.getComponent<CTransform>().pos;
        m_playerConfig.scale = m_player.getComponent<CTransform>().scale;
        m_playerConfig.boundingBox = m_player.getComponent<CBoundingBox>().size;
        m_playerConfig.damage = m_player.getComponent<CDamage>().damage;
        m_playerConfig.health = m_player.getComponent<CHealth>().max;
        m_playerConfig.gravity = m_player.getComponent<CGravity>().gravity;
    }

    m_entityManager.update();

    // for (auto [a, item] : m_game->getSettings().keys) {
    //     registerAction(item.action, item.key);spawnpla
    // }

    registerAction("QUIT",           sf::Keyboard::Key::Escape);
    registerAction("TOGGLE_FOLLOW",  sf::Keyboard::Key::Y, true);

    registerAction("MOVE_UP",    sf::Keyboard::Key::Up);
    registerAction("MOVE_DOWN",  sf::Keyboard::Key::Down);
    registerAction("MOVE_LEFT",  sf::Keyboard::Key::Left);
    registerAction("MOVE_RIGHT", sf::Keyboard::Key::Right);
    registerAction("SHOOT",      sf::Keyboard::Key::Space);

    registerAction("PAUSE",          sf::Keyboard::Key::P, true);
    registerAction("TOGGLE_TEXTURE", sf::Keyboard::Key::T, true);
    registerAction("TOGGLE_DEBUG",   sf::Keyboard::Key::D, true);
    registerAction("TOGGLE_GRID",    sf::Keyboard::Key::G, true);
    registerAction("RELOAD",         sf::Keyboard::Key::R, true);
    registerAction("TOGGLE_SLOW",    sf::Keyboard::Key::S, true);
}

Vec2 Scene_World::getTileIndex(Vec2 pos) {
    pos = pos / 64 + m_shift;
    pos.x = floor(pos.x);
    pos.y = floor(pos.y);
    return pos;
}

EntityVec Scene_World::getTilesAround(Vec2 tileIndex) {
    PROFILE_FUNCTION();

    // TODO !!!
    return m_entityManager.getEntities();

    EntityVec r;

    int width = m_tiles.size();
    int height = m_tiles.at(0).size();

    int x = tileIndex.x;
    int y = tileIndex.y;

    if (x-1 >= 0 && y-1 >= 0 && m_tiles[x-1][y-1].isActive())    r.push_back(m_tiles[x-1][y-1]);
    if (y-1 >= 0 && m_tiles[x][y-1].isActive())                  r.push_back(m_tiles[x][y-1]);
    if (x+1 < width && y-1 >= 0 && m_tiles[x+1][y-1].isActive()) r.push_back(m_tiles[x+1][y-1]);

    if (x-1 >= 0 && m_tiles[x-1][y].isActive())    r.push_back(m_tiles[x-1][y]);
    if (m_tiles[x][y].isActive())                  r.push_back(m_tiles[x][y]);
    if (x+1 < width && m_tiles[x+1][y].isActive()) r.push_back(m_tiles[x+1][y]);

    if (x-1 >= 0 && y+1 < height && m_tiles[x-1][y+1].isActive())    r.push_back(m_tiles[x-1][y+1]);
    if (y+1 < height && m_tiles[x][y+1].isActive())                  r.push_back(m_tiles[x][y+1]);
    if (x+1 < width && y+1 < height && m_tiles[x+1][y+1].isActive()) r.push_back(m_tiles[x+1][y+1]);

    return r;
}

EntityVec Scene_World::getViewTiles(Vec2 tl, Vec2 br) {
    PROFILE_FUNCTION();

    // TODO !!!
    return m_entityManager.getEntities();

    EntityVec r;

    int width = m_tiles.size();
    int height = m_tiles.at(0).size();

    if (tl.x < 0)       { tl.x = 0; }
    if (tl.x >= width)  { tl.x = width-1; }
    if (tl.y < 0)       { tl.y = 0; }
    if (tl.y >= height) { tl.y = height-1; }

    if (br.x < 0)       { br.x = 0; }
    if (br.x >= width)  { br.x = width-1; }
    if (br.y < 0)       { br.y = 0; }
    if (br.y >= height) { br.y = height-1; }

    for (int i=tl.x; i<=br.x; i++) {
        for (int j=tl.y; j<=br.y; j++) {
            if (m_tiles[i][j].isActive()) {
                r.push_back(m_tiles[i][j]);
            }
        }
    }

    return r;
}

void Scene_World::spawnPlayer() {
    PROFILE_FUNCTION();

    for (auto e : m_entityManager.getEntities("player")) {
        e.destroy();
    }
    m_entityManager.update();

    m_player = m_entityManager.addEntity("player");
    m_player.addComponent<CTransform>(m_playerConfig.startPos);
    m_player.addComponent<CAnimation>(m_game->assets().getAnimation("StandDown"), true);
    m_player.addComponent<CBoundingBox>(m_playerConfig.boundingBox, true, false);
    m_player.addComponent<CHealth>(m_playerConfig.health, m_playerConfig.health);
    m_player.addComponent<CGravity>(m_playerConfig.gravity);
    m_player.addComponent<CState>();
    m_player.addComponent<CDamage>(m_playerConfig.damage);
    m_player.addComponent<CInput>();


    for (auto e : m_entityManager.getEntities("npc")) {
        if (auto o = Physics::GetOverlap(m_player, e); o.x != 0 && o.y != 0) {
            if (e.hasComponent<CFollowPlayer>()) {
                e.getComponent<CTransform>().pos = e.getComponent<CFollowPlayer>().home;
            }
        }
    }

    // TODO: Implements this function so that it uses the parameters input from the level file
    //       Those parameters should be stored in the m_playerConfig variable
}

void Scene_World::spawnSword(Entity entity) {
    PROFILE_FUNCTION();
    // TODO: Implement the spawning of the sword, which:
    //       - should be given the appropriate lifespan
    //       - should spawn at the appropriate location based on player's facing direction
    //       - be given a damage value of 1
    //       - should play the "Slash" sound

    if (!entity.hasComponent<CState>()) {
        return;
    }

    auto e = m_entityManager.addEntity("sword");
    moveSword(entity);
    e.addComponent<CLifespan>(10, m_currentFrame);

    auto& stateAttack = entity.getComponent<CState>().shoot;
    m_game->assets().getSound("Slash")->play();

    e.onDestroy([&stateAttack]() {
        stateAttack = false;
    });
}

void Scene_World::moveSword(Entity entity) {
    PROFILE_FUNCTION();

    if (!entity.hasComponent<CState>()) {
        return;
    }

    auto s = entity.getComponent<CState>();
    auto p = entity.getComponent<CTransform>().pos;

    for (auto e : m_entityManager.getEntities("sword")) {
        switch (s.direction) {
            case Direction::UP:
            setAnimation(e, "SwordUp", true, 1);
            p.y -= e.getSize().y;
            setBoundingBox(e, Vec2(2, 2), true, false);
            break;

            case Direction::DOWN:
            setAnimation(e, "SwordDown", true, 1);
            p.y += entity.getSize().y;
            setBoundingBox(e, Vec2(2, 8), true, false);
            break;

            case Direction::LEFT:
            setAnimation(e, "SwordRight", true, -1);
            p.x -= e.getSize().x;
            setBoundingBox(e, Vec2(7, 3), true, false);
            break;

            case Direction::RIGHT:
            setAnimation(e, "SwordRight", true, 1);
            p.x += entity.getSize().x;
            setBoundingBox(e, Vec2(7, 3), true, false);
            break;
        }

        e.getComponent<CTransform>().pos = p;
    }
}

void Scene_World::update() {
    PROFILE_FUNCTION();

    if (!m_paused) {
        m_entityManager.update();

        sAI();
        sMovement();
        sCollision();
        sTeleport();
        sStatus();
        sAnimation();
        sCamera();
        sWin();
    }

    sRender();

    if (!m_paused) {
        m_currentFrame++;
    }
}

void Scene_World::sMovement() {
    PROFILE_FUNCTION();

    // TODO: Implement all player movement functionality here based on
    //       the player's input component variables
    auto& i = m_player.getComponent<CInput>();
    auto& t = m_player.getComponent<CTransform>();
    auto& s = m_player.getComponent<CState>();

    s.run = false;

    int c = i.up + i.down + i.left + i.right;
    if (c > 1) { return; }

    t.prevPos = t.pos;

    if (i.up)   {
        t.pos.y -= m_game->getSettings().playerConfig.speed;
        s.direction = Direction::UP;
        s.run = true;
    }

    if (i.down) {
        t.pos.y += m_game->getSettings().playerConfig.speed;
        s.direction = Direction::DOWN;
        s.run = true;
    }

    if (i.left) {
        t.pos.x -= m_game->getSettings().playerConfig.speed;
        s.direction = Direction::LEFT;
        s.run = true;
    }

    if (i.right) {
        t.pos.x += m_game->getSettings().playerConfig.speed;
        s.direction = Direction::RIGHT;
        s.run = true;
    }

    if (i.shoot && !s.shoot) {
        spawnSword(m_player);
        s.shoot = true;
    }

    // auto room = getRoomByPos(m_player.getComponent<CTransform>().pos);

    // if (!roomHasEntities(room)) {
    //     t.pos = t.prevPos;
    // }

    moveSword(m_player);
}

void Scene_World::sDoAction(const Action & action) {
    PROFILE_FUNCTION();

    // TODO: Implement all actions described for the game here
    //       Only the setting of the player's input component variables should be set here
    //       Do minimal logic in this function

    if (action.type() == "START") {
             if (action.name() == "PAUSE")          { setPaused(!m_paused); }
        else if (action.name() == "QUIT")           { m_game->changeScene("MENU", std::make_shared<Scene_Menu>(m_game)); }
        else if (action.name() == "TOGGLE_FOLLOW")  { m_follow = !m_follow; }
        else if (action.name() == "TOGGLE_TEXTURE") { m_drawTextures = !m_drawTextures; }
        else if (action.name() == "TOGGLE_DEBUG")   { m_drawDebug = !m_drawDebug; }
        else if (action.name() == "TOGGLE_GRID")    { m_drawGrid = !m_drawGrid; }
        else if (action.name() == "MOVE_UP")        { m_player.getComponent<CInput>().up = true; }
        else if (action.name() == "MOVE_DOWN")      { m_player.getComponent<CInput>().down = true; }
        else if (action.name() == "MOVE_LEFT")      { m_player.getComponent<CInput>().left = true; }
        else if (action.name() == "MOVE_RIGHT")     { m_player.getComponent<CInput>().right = true; }
        else if (action.name() == "SHOOT")          { m_player.getComponent<CInput>().shoot = true; }
        else if (action.name() == "RELOAD")         { reload(); }
        // ======================================== ray castings
        else if (action.name() == "MOUSE_MOVE") {
            //action.screenPos();
            //action.pos();

            Entity dot;
            if (auto items = m_entityManager.getEntities("dot"); items.size() != 0) {
                dot = items.at(0);

                if (items.size() > 1) {
                    for (auto e : items) {
                        if (e.id() != dot.id()) {
                            e.destroy();
                        }
                    }
                }
            } else {
                dot = m_entityManager.addEntity("dot");
                dot.addComponent<CTransform>(Vec2(0, 0), 0);
                dot.addComponent<CAnimation>(m_game->assets().getAnimation("Buster"), true);
            }

            dot.getComponent<CTransform>().pos = action.pos();
        }
        // ======================================== ray castings
    } else if (action.type() == "END") {
             if (action.name() == "MOVE_UP")    { m_player.getComponent<CInput>().up = false; }
        else if (action.name() == "MOVE_DOWN")  { m_player.getComponent<CInput>().down = false; }
        else if (action.name() == "MOVE_LEFT")  { m_player.getComponent<CInput>().left = false; }
        else if (action.name() == "MOVE_RIGHT") { m_player.getComponent<CInput>().right = false; }
        else if (action.name() == "SHOOT")      { m_player.getComponent<CInput>().shoot = false; }
    }
}

void Scene_World::sAI() {
    PROFILE_FUNCTION();

    // TODO: Implement Enemy AI
    //       Implement Follow behavior
    //       Implement Patrol behavior

    for (auto e : m_entityManager.getEntities("npc")) {
        if (e.hasComponent<CPatrol>()) {
            auto& t = e.getComponent<CTransform>();
            auto& p = e.getComponent<CPatrol>();
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

            t.prevPos = t.pos;
            t.pos += speed;
        }

        // TODO добавить проверку видимости цели
        if (e.hasComponent<CFollowPlayer>()) {
            auto& t = e.getComponent<CTransform>();
            auto p = e.getComponent<CFollowPlayer>();

            auto playerPos = m_player.getComponent<CTransform>().pos;
            auto newSpeed = (playerPos-t.pos);

            bool intersect = false;

            if (newSpeed.length() <= p.visionRadius) {
                auto pp = getTileIndex(playerPos);
                auto np = getTileIndex(t.pos);
                auto tl = Vec2(std::min(pp.x, np.x), std::min(pp.y, np.y));
                auto br = Vec2(std::max(pp.x, np.x), std::max(pp.y, np.y));
                auto tiles = getViewTiles(tl, br);
                for (auto x : m_entityManager.getEntities()) {
                    if (x.tag() != "tile" && x.tag() != "player" && x.id() != e.id() && x.getComponent<CBoundingBox>().blockVision) {
                        tiles.push_back(x);
                    }
                }

                for (auto x : tiles) {
                    if (x.getComponent<CBoundingBox>().blockVision && Physics::EntityIntersect(playerPos, t.pos, x)) {
                        intersect = true;
                        break;
                    }
                }
            }

            if (newSpeed.length() > p.visionRadius || intersect) {
                newSpeed = p.home-t.pos;
            }

            if (newSpeed.length() > 5) {
                newSpeed /= newSpeed.length();
                newSpeed *= p.speed;

                auto oldSpeed = t.velocity;
                auto delta = (newSpeed-oldSpeed) / 10;
                newSpeed += delta;

                t.prevPos = t.pos;
                t.pos += newSpeed;
                t.velocity = newSpeed;
            }
        }
    }
}

void Scene_World::sStatus() {
    PROFILE_FUNCTION();

    // TODO: Implement Lifespan here
    //       Implement Invincibility Frames here
    //...

    if (m_player.getComponent<CHealth>().current <= 0) {
        m_game->assets().getSound("LinkDie")->play();
        spawnPlayer();
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

void Scene_World::sCollision() {
    PROFILE_FUNCTION();

    // TODO: Implement entity - tile collisions
    //       Implement player - enemy collisions with appropriate damage calculations
    //       Implement sword - NPC collisions
    //       Implement black tile collisions / 'teleporting'
    //       Implement entity - heart collisions and life gain logic
    //
    //       You may want to use helper functions for these behaviors or this function will get long

    //auto tilesAroundPlayer = getTilesAround(getTileIndex(m_player.getComponent<CTransform>().pos));

    for (auto e : m_entityManager.getEntities("tile")) {
        auto o = Physics::GetOverlap(m_player, e);
        if (o.x == 0 || o.y == 0) {
            continue;
        }

        if (e.hasComponent<CBoundingBox>() && e.getComponent<CBoundingBox>().blockMove) {
            m_player.getComponent<CTransform>().pos = m_player.getComponent<CTransform>().prevPos;
        }

        // player x heart tile
        if (e.getComponent<CAnimation>().animation.getName() == "Heart") {
            auto& h = m_player.getComponent<CHealth>();
            if (h.current < h.max) {
                h.current = h.max;
                e.destroy();
                m_game->assets().getSound("GetItem")->play();
            }
        }

        if (e.hasComponent<CTeleport>() && abs(o.x) > 32 && abs(o.y) > 32) {
            auto lvl = e.getComponent<CTeleport>().level;
            auto levels = m_game->getSettings().levels;

            if (lvl >= 0 && lvl < (int)levels.size()) {
                m_sTeleport.level = lvl;
            }
        }
    }

    auto checkPlayer = (!m_player.hasComponent<CInvincibility>() || m_player.getComponent<CInvincibility>().iframes <= 0);
    auto swordIt = m_entityManager.getEntities("sword");

    for (auto n : m_entityManager.getEntities("npc")) {
        // npc x player
        if (checkPlayer) {
            auto o = Physics::GetOverlap(m_player, n);

            if (o.x != 0 && o.y != 0) {
                m_player.getComponent<CHealth>().current -= n.getComponent<CDamage>().damage * m_game->getNpcDamageCoeff();;
                m_player.addComponent<CInvincibility>(30);
                if (m_player.getComponent<CHealth>().current > 0) {
                    m_game->assets().getSound("LinkHit")->play();
                }

                if (n.hasComponent<CBoundingBox>() && n.getComponent<CBoundingBox>().blockMove) {
                    m_player.getComponent<CTransform>().pos = m_player.getComponent<CTransform>().prevPos;
                }
            }
        }

        // npc x sword
        if (swordIt.size() > 0 && (!n.hasComponent<CInvincibility>() || n.getComponent<CInvincibility>().iframes <= 0)) {
            auto sword = swordIt.at(0);

            auto o = Physics::GetOverlap(sword, n);
            if (o.x != 0 && o.y != 0) {
                n.getComponent<CHealth>().current -= m_player.getComponent<CDamage>().damage * m_game->getPlayerDamageCoeff();
                n.addComponent<CInvincibility>(30);
                if (n.getComponent<CHealth>().current > 0) {
                    m_game->assets().getSound("EnemyHit")->play();
                }
            }
        }

        // Пересечения двигающихся по маршруту NPC с тайлами не нужно рассчитывать.
        if (n.hasComponent<CPatrol>()) {
            continue;
        }

        // Если преследующий NPC стоит на месте, для него не нужно рассчитывать коллизии с тайлами.
        if (n.hasComponent<CFollowPlayer>() && n.getComponent<CTransform>().pos == n.getComponent<CTransform>().prevPos) {
            continue;
        }

        //auto tilesAroundNPC = getTilesAround(getTileIndex(n.getComponent<CTransform>().pos));
        for (auto t : m_entityManager.getEntities("tile")) {
            auto o = Physics::GetOverlap(n, t);
            if (o.x == 0 || o.y == 0) {
                continue;
            }

            if (t.hasComponent<CBoundingBox>() && t.getComponent<CBoundingBox>().blockMove) {
                // Вычитать и прибавлять 1 из предыдущей позиции нужно, чтобы NPC не "цеплялся" за тайлы при приследовании игрока.

                auto po = Physics::GetPreviousOverlap(n, t);
                auto& nt = n.getComponent<CTransform>();

                if (po.x != 0) {
                    if (nt.prevPos.y > nt.pos.y) {
                        nt.pos.y = nt.prevPos.y + 1;
                    } else {
                        nt.pos.y = nt.prevPos.y - 1;
                    }
                }

                if (po.y != 0) {
                    if (nt.prevPos.x > nt.pos.x) {
                        nt.pos.x = nt.prevPos.x + 1;
                    } else {
                        nt.pos.x = nt.prevPos.x - 1;
                    }
                }
            }

            if (t.getComponent<CAnimation>().animation.getName() == "Heart") {
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

void Scene_World::sAnimation() {
    PROFILE_FUNCTION();

    // TODO: Implement player facing direction animation
    //       Implement sword animation based on player facing
    //         The sword should move if the player changes direction mid swing
    //       Implement destruction of entities with non-repeating finished animations

    auto s = m_player.getComponent<CState>();
    switch (s.direction) {
        case Direction::UP:
        if (s.shoot) {
            setAnimation(m_player, "AtkUp", true, 1);
        } else if (s.run) { setAnimation(m_player, "RunUp", true, 1); }
        else       { setAnimation(m_player, "StandUp", true, 1); }
        break;

        case Direction::DOWN:
        if (s.shoot) {
            setAnimation(m_player, "AtkDown", true, 1);
        } else if (s.run) { setAnimation(m_player, "RunDown", true, 1); }
        else       { setAnimation(m_player, "StandDown", true, 1); }
        break;

        case Direction::LEFT:
        if (s.shoot) {
            setAnimation(m_player, "AtkRight", true, -1);
        } else if (s.run) { setAnimation(m_player, "RunRight", true, -1); }
        else       { setAnimation(m_player, "StandRight", true, -1); }
        break;

        case Direction::RIGHT:
        if (s.shoot) {
            setAnimation(m_player, "AtkRight", true, 1);
        } else if (s.run) { setAnimation(m_player, "RunRight", true, 1); }
        else       { setAnimation(m_player, "StandRight", true, 1); }
        break;
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

void Scene_World::sCamera() {
    PROFILE_FUNCTION();

    // TODO: Implement camera view logic

    // get the current view, which we will modify in the if-statement below
    sf::View view = m_game->window().getView();
    auto p = m_player.getComponent<CTransform>().pos;

    if (m_follow) {
        // calculate view for player follow camera
        view.setCenter({p.x, p.y});
    } else {
        // calculate view for room-based camera
        auto r = getRoomByPos(p);
        r.x *= 1280;
        r.y *= 768;
        r.x += 1280/2;
        r.y += 768/2;
        view.setCenter({r.x, r.y});
    }

    // then set the window view
    m_game->window().setView(view);
}

void Scene_World::sRender() {
    PROFILE_FUNCTION();

    m_game->window().clear(m_bgColor);
    m_game->getVertexArrays().clearAll();

    // draw all Entity textures / animations
    if (m_drawTextures) {
        PROFILE_SCOPE("drawTextures");
        // auto viewCenter = Vec2(m_game->window().getView().getCenter().x, m_game->window().getView().getCenter().y);
        // auto viewSize = Vec2(m_game->window().getView().getSize().x, m_game->window().getView().getSize().y)/2;

        // auto tl = getTileIndex(viewCenter-viewSize);
        // auto br = getTileIndex(viewCenter+viewSize);

        // auto viewTiles = getViewTiles(tl, br);

        // {PROFILE_SCOPE("copy_entities_to_viewTiles");
        // for (auto e : m_entityManager.getEntities()) {
        //     if (e.tag() != "tile" || e.hasAnimation("Curtain")) {
        //         viewTiles.push_back(e);
        //     }
        // }}

        {PROFILE_SCOPE("foreach_viewTiles");
        for (auto e : m_entityManager.getEntities()) {
            auto & transform = e.getComponent<CTransform>();

            if (e.hasComponent<CAnimation>()) {
                auto a = e.getComponent<CAnimation>().animation;
                auto& s = e.getComponent<CAnimation>().animation.getSprite();
                s.setRotation(sf::degrees(transform.angle));
                s.setPosition({ transform.pos.x, transform.pos.y });
                s.setScale({ transform.scale.x, transform.scale.y });
                auto c = s.getColor();
                c.a = (e.hasComponent<CInvincibility>()) ? a.getAlpha()/2 : a.getAlpha();
                s.setColor(c);

                if (auto shaderName = a.getShader(); shaderName != "") {
                    auto shader = m_game->assets().getShader(shaderName);
                    shader->setUniform("u_alpha", ((float)c.a)/255.0f);
                    shader->setUniform("u_mix", a.getMix());
                    shader->setUniform("u_resolution", sf::Vector2f(m_game->window().getSize().x, m_game->window().getSize().y));
                    shader->setUniform("u_time", (float)m_currentFrame/60);
                    shader->setUniform("texture", sf::Shader::CurrentTexture);
                    //m_game->getVertexArray().draw(animation.getSprite(), &(*shader));
                    m_game->getVertexArrays().draw(s);
                    m_game->getVertexArrays().draw(s, shaderName, m_game->assets().getLargeTexture(), shader);
                } else {
                    m_game->getVertexArrays().draw(s, "", m_game->assets().getLargeTexture());
                }
            }

            if (e.hasComponent<CHealth>()) {
                auto& h = e.getComponent<CHealth>();
                int health = (h.current / h.max) * 100;
                auto a = m_game->assets().getAnimation("healthBar" + std::to_string(health));
                auto pos = Vec2(transform.pos.x, transform.pos.y - e.getSize().y / 2.0 - a.getTexture().size.y - 10.0);
                a.getSprite().setPosition(pos);
                m_game->getVertexArrays().draw(a.getSprite(), "", m_game->assets().getLargeTexture());
            }
        }}
    }

    // draw entities
    m_game->getVertexArrays().drawAll(m_game->window());

    // draw the grid so that students can easily debug
    if (m_drawGrid) {
        PROFILE_SCOPE("drawGrid");
        auto wh = m_game->getWindowSize();
        float leftX = m_game->window().getView().getCenter().x - wh.x / 2.0 + 2.0;
        float rightX = leftX + wh.x - 4;

        float topY = m_game->window().getView().getCenter().y - wh.y / 2 + 2;
        float bottomY = topY + wh.y - 4;

        float nextGridX = leftX - ((int)leftX % (int)m_gridSize.x);
        float nextGridY = topY - ((int)topY % (int)m_gridSize.y);

        for (float x = nextGridX; x < rightX; x += m_gridSize.x) {
            drawLine(Vec2(x, topY), Vec2(x, bottomY));
        }

        for (float y = nextGridY; y < wh.y; y += m_gridSize.y) {
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

    if (m_drawDebug) {
        PROFILE_SCOPE("drawDebug");

        sf::RectangleShape rect;
        rect.setFillColor(sf::Color(0, 0, 0, 0));
        rect.setSize(sf::Vector2f(63, 63));
        rect.setOrigin(sf::Vector2f(32, 32));
        rect.setOutlineThickness(1);

        // draw all Entity collision bounding boxes with a rectangle shape
        for (auto e : m_entityManager.getEntities()) {
            if (e.hasComponent<CBoundingBox>()) {
                auto blockMove = e.getComponent<CBoundingBox>().blockMove;
                auto blockVision = e.getComponent<CBoundingBox>().blockVision;
                auto box = e.getBoundingBox();
                auto & transform = e.getComponent<CTransform>();

                rect.setSize(sf::Vector2f(box.x - 1, box.y - 1));
                rect.setOrigin(sf::Vector2f(box.x / 2.0, box.y / 2.0));
                rect.setPosition({transform.pos.x, transform.pos.y });

                if (blockMove && blockVision) { rect.setOutlineColor(sf::Color::Black); }
                if (blockMove && !blockVision) { rect.setOutlineColor(sf::Color::Blue); }
                if (!blockMove && blockVision) { rect.setOutlineColor(sf::Color::Red); }
                if (!blockMove && !blockVision) { rect.setOutlineColor(sf::Color::White); }

                m_game->window().draw(rect);
            }
        }

        // Draw room borders
        rect.setSize(sf::Vector2f(1280, 768));
        rect.setOrigin(sf::Vector2f(0, 0));
        rect.setOutlineColor(sf::Color::Black);
        rect.setFillColor(sf::Color(0, 0, 0, 0));
        rect.setOutlineThickness(1);
        for (int i=-10; i<=10; i++) {
            for (int j=-10; j<=10; j++) {
                rect.setPosition(sf::Vector2f(i*1280, j*768));
                m_game->window().draw(rect);
            }
        }

        // Draw patrol points and vision vectors
        sf::CircleShape c;
        c.setFillColor(sf::Color::Black);
        c.setRadius(4);
        c.setOrigin(sf::Vector2f(2, 2));
        for (auto e : m_entityManager.getEntities("npc")) {
            if (e.hasComponent<CPatrol>()) {
                for (auto p : e.getComponent<CPatrol>().positions) {
                    c.setPosition(sf::Vector2f(p.x, p.y));
                    m_game->window().draw(c);
                }
            }

            if (e.hasComponent<CFollowPlayer>()) {
                auto playerPos = m_player.getComponent<CTransform>().pos;
                auto entityPos = e.getComponent<CTransform>().pos;

                bool intersect = false;
                // 10ms!
                // for (auto x : m_entityManager.getEntities()) {
                //     if (x.tag() == "player" || x.id() == e.id() || !x.getComponent<CBoundingBox>().blockVision) {
                //         continue;
                //     }

                //     if (Physics::EntityIntersect(playerPos, entityPos, x)) {
                //         intersect = true;
                //         break;
                //     }
                // }

                if (intersect) {
                    drawLine(playerPos, entityPos, sf::Color::Black);
                } else {
                    drawLine(playerPos, entityPos, sf::Color::Red);
                }
            }
        }

        auto tilesAround = getTilesAround(getTileIndex(m_player.getComponent<CTransform>().pos));
        rect.setSize(sf::Vector2f(64, 64));
        rect.setOutlineColor(sf::Color::Red);
        rect.setOutlineThickness(1);

        for (auto e : tilesAround) {
            auto p = e.getComponent<CTransform>().pos;
            p -= 32;
            rect.setPosition(sf::Vector2f(p.x, p.y));
            m_game->window().draw(rect);
        }
    }

    if (m_currentFrame % 60 == 0) {
        m_frameTime.setString(std::to_string(m_game->getFrameTime()) + " ms/frame");
    }

    m_frameTime.setPosition(m_game->p2c(Vec2(10, 10)));
    m_game->window().draw(m_frameTime);

    // ======================================== ray castings

    // Entity view;
    // if (auto items = m_entityManager.getEntities("view"); items.size() != 0) {
    //     view = items.at(0);
    // } else {
    //     view = m_entityManager.addEntity("view");
    //     view.addComponent<CTransform>(Vec2(0 ,0), 1);
    //     view.addComponent<CBoundingBox>(Vec2(m_game->window().getView().getSize()), false, false);
    // }

    // view.getComponent<CTransform>().pos = Vec2(m_game->window().getView().getCenter());

    // std::vector<Entity> items;
    // std::vector<Vec2> rays;

    // // Собираем сущности, отображаемые на экране, и лучи, идущие в углы сущностей
    // for (auto e : m_entityManager.getEntities()) {
    //     if (e.tag() == "dot" || !e.hasComponent<CBoundingBox>()) {
    //         continue;
    //     }

    //     auto ov = Physics::GetOverlap(view, e);
    //     if (ov.x == 0 || ov.y == 0) {
    //         continue;
    //     }

    //     if (e.getComponent<CBoundingBox>().blockVision || e.tag() == "view") {
    //         items.push_back(e);
    //     }

    //     auto ec = e.getComponent<CTransform>().pos;
    //     auto es = e.getComponent<CBoundingBox>().halfSize;
    //     auto a = Vec2(ec.x - es.x, ec.y - es.y);
    //     auto b = Vec2(ec.x + es.x, ec.y - es.y);
    //     auto c = Vec2(ec.x + es.x, ec.y + es.y);
    //     auto d = Vec2(ec.x - es.x, ec.y + es.y);

    //     rays.push_back(a);
    //     rays.push_back(b);
    //     rays.push_back(c);
    //     rays.push_back(d);
    // }

    // auto pos = m_entityManager.getEntities("dot")[0].getComponent<CTransform>().pos;
    // auto dev = 0.001f;

    // auto ec = view.getComponent<CTransform>().pos;
    // auto es = view.getComponent<CBoundingBox>().halfSize;
    // auto p1 = Vec2(ec.x - es.x, ec.y - es.y);
    // auto p2 = Vec2(ec.x + es.x, ec.y - es.y);
    // auto p3 = Vec2(ec.x + es.x, ec.y + es.y);
    // auto p4 = Vec2(ec.x - es.x, ec.y + es.y);

    // struct Line { Vec2 p1, p2; };
    // std::vector<Line> lines1 = {{p1, p2}, {p2, p3}, {p3, p4}, {p4, p1}};

    // // Добавляем в список лучей лучи, идущие в точки пересечения сущностей с границей экрана
    // for (auto e : items) {
    //     auto ec = e.getComponent<CTransform>().pos;
    //     auto es = e.getComponent<CBoundingBox>().halfSize;
    //     auto a = Vec2(ec.x - es.x, ec.y - es.y);
    //     auto b = Vec2(ec.x + es.x, ec.y - es.y);
    //     auto c = Vec2(ec.x + es.x, ec.y + es.y);
    //     auto d = Vec2(ec.x - es.x, ec.y + es.y);

    //     std::vector<Line> lines2 = {{a, b}, {b, c}, {c, d}, {d, a}};

    //     for (auto line1 : lines1) {
    //         for (auto line2 : lines2) {
    //             if (auto res = Physics::LineIntersect(line1.p1, line1.p2, line2.p1, line2.p2, dev); res.status) {
    //                 rays.push_back(res.result);
    //             }
    //         }
    //     }
    // }

    // std::vector<Vec2> rays2;

    // for (auto& r : rays) {
    //     bool drop = false;

    //     // Удаляем лучи, которые находятся пересекаются с объектами
    //     for (auto e : items) {
    //         auto ec = e.getComponent<CTransform>().pos;
    //         auto es = e.getComponent<CBoundingBox>().halfSize;
    //         auto a = Vec2(ec.x - es.x, ec.y - es.y);
    //         auto b = Vec2(ec.x + es.x, ec.y - es.y);
    //         auto c = Vec2(ec.x + es.x, ec.y + es.y);
    //         auto d = Vec2(ec.x - es.x, ec.y + es.y);

    //         if (auto res = Physics::LineIntersect(pos, r, a, b, dev); res.status && r != a && r != b) {
    //             drop = true;
    //             break;
    //         }

    //         if (auto res = Physics::LineIntersect(pos, r, b, c, dev); res.status && r != b && r != c) {
    //             drop = true;
    //             break;
    //         }

    //         if (auto res = Physics::LineIntersect(pos, r, c, d, dev); res.status && r != c && r != d) {
    //             drop = true;
    //             break;
    //         }

    //         if (auto res = Physics::LineIntersect(pos, r, d, a, dev); res.status && r != d && r != a) {
    //             drop = true;
    //             break;
    //         }
    //     }

    //     if (!drop || true) {
    //         auto s = (r-pos); // Приводим к началу координат
    //         s /= s.length();  // Нормализуем

    //         // Строим перпендикуляр
    //         auto v = Vec2(abs(s.y), abs(s.x));
    //         if (s.x < 0) { v.x = -v.x; }
    //         if (s.y < 0) { v.y = -v.y; }

    //         v *= 2; // Задаем длину перпендикуляра

    //         // Строим два луча - слева и справа от основного
    //         static std::vector<Vec2> rs(2);
    //         rs[0] = r + v;
    //         rs[1] = r - v;

    //         // Удлиняем лучи до размера большего, чем экран
    //         for (auto& r : rs) {
    //             r = (r - pos) * 10000 + pos;
    //         }

    //         // Укорачиваем лучи до ближайшего препятствия
    //         for (auto r : rs) {
    //             for (auto e : items) {
    //                 auto ec = e.getComponent<CTransform>().pos;
    //                 auto es = e.getComponent<CBoundingBox>().halfSize;
    //                 auto a = Vec2(ec.x - es.x, ec.y - es.y);
    //                 auto b = Vec2(ec.x + es.x, ec.y - es.y);
    //                 auto c = Vec2(ec.x + es.x, ec.y + es.y);
    //                 auto d = Vec2(ec.x - es.x, ec.y + es.y);

    //                 if (auto res = Physics::LineIntersect(pos, r, a, b); res.status && pos.dist(res.result) < pos.dist(r)) {
    //                     r = res.result;
    //                 }

    //                 if (auto res = Physics::LineIntersect(pos, r, b, c); res.status && pos.dist(res.result) < pos.dist(r)) {
    //                     r = res.result;
    //                 }

    //                 if (auto res = Physics::LineIntersect(pos, r, c, d); res.status && pos.dist(res.result) < pos.dist(r)) {
    //                     r = res.result;
    //                 }

    //                 if (auto res = Physics::LineIntersect(pos, r, d, a); res.status && pos.dist(res.result) < pos.dist(r)) {
    //                     r = res.result;
    //                 }
    //             }

    //             rays2.push_back(r);
    //         }
    //     }
    // }

    // auto v0 = Vec2(pos.x, -10000) - pos;
    // auto v0len = v0.length();

    // // Сортируем лучи по углу
    // std::sort(rays2.begin(), rays2.end(), [v0, v0len, pos](const Vec2 & a, const Vec2 & b) -> bool {
    //     auto v1 = a-pos;
    //     auto v2 = b-pos;
    //     auto a1 = sf::radians(acosf((v0.x * v1.x + v0.y * v1.y) / (v0len * v1.length()))).asDegrees();
    //     auto a2 = sf::radians(acosf((v0.x * v2.x + v0.y * v2.y) / (v0len * v2.length()))).asDegrees();

    //     if (v1.x < v0.x) { a1 = 360-a1; }
    //     if (v2.x < v0.x) { a2 = 360-a2; }

    //     return a1 < a2;
    // });

    // auto c = sf::Color::Cyan;
    // c.a = 80;

    // if (rays2.size() > 0) { rays2.push_back(rays2[0]); }

    // for (size_t i = 0; i<rays2.size()-1; i++) {
    //     auto p0 = pos;
    //     auto p1 = rays2[i];
    //     auto p2 = rays2[i+1];
    //     // drawLine(p0, p1, c);
    //     // drawLine(p0, p2, c);
    //     // drawLine(p1, p2, c);

    //     sf::ConvexShape triangle;
    //     triangle.setPointCount(3);
    //     triangle.setPoint(0, p0);
    //     triangle.setPoint(1, p1);
    //     triangle.setPoint(2, p2);
    //     triangle.setFillColor(c);
    //     m_game->window().draw(triangle);
    // }

    // ======================================== ray castings
}

void Scene_World::setPaused(bool v) {
    m_game->assets().getSound(v ? "PauseIn" : "PauseOut")->play();
    m_paused = v;
};

void Scene_World::reload() {
    m_game->changeScene("PLAY", std::make_shared<Scene_World>(m_game, m_levelPath), true);
}

void Scene_World::setAnimation(Entity e, const std::string & animName, bool repeat, float scale) {
    if (e.getComponent<CTransform>().scale.x != scale) {
        e.getComponent<CTransform>().scale.x = scale;
    }

    if (!e.hasComponent<CAnimation>() || e.getComponent<CAnimation>().animation.getName() != animName) {
        e.addComponent<CAnimation>(m_game->assets().getAnimation(animName), repeat);
    }
}

void Scene_World::setBoundingBox(Entity e, Vec2 bb, bool blockMove, bool blockVision) {
    if (!e.hasComponent<CBoundingBox>() || e.getComponent<CBoundingBox>().size != bb || e.getComponent<CBoundingBox>().blockMove != blockMove || e.getComponent<CBoundingBox>().blockVision != blockVision) {
        e.addComponent<CBoundingBox>(Vec2(2, 8), true, false);
    }
}

Vec2 Scene_World::getRoomByPos(const Vec2 & pos) {
    auto r = Vec2((int)pos.x / 1280, (int)pos.y / 768);

    if (pos.x < 0) {
        r.x -= 1;
    }

    if (pos.y < 0) {
        r.y -= 1;
    }

    return r;
}

// bool Scene_World::roomHasEntities(const Vec2 & room) {
//     PROFILE_FUNCTION();

//     Vec2 roomSize = { 1280, 768 };
//     auto roomCenter = Vec2(room.x*roomSize.x, room.y*roomSize.y)+roomSize/2;

//     for (auto e : m_entityManager.getEntities()) {
//         if (e.tag() == "player" || e.tag() == "sword") {
//             continue;
//         }

//         auto ep = e.getComponent<CTransform>().pos;
//         auto es = e.getBoundingBox();

//         if (intersect(roomCenter, roomSize, ep, es)) {
//             return true;
//         }
//     }

//     return false;
// }

bool Scene_World::intersect(Vec2 ap, Vec2 as, Vec2 bp, Vec2 bs) {
    auto delta = Vec2(abs(bp.x-ap.x), abs(bp.y-ap.y));

    as /= 2;
    bs /= 2;
    auto o = Vec2(as.x + bs.x - delta.x, as.y + bs.y - delta.y);

    return o.x > 0 && o.y > 0;
}

void Scene_World::sWin() {
    // PROFILE_FUNCTION();
    // if (m_entityManager.getEntities("npc").size() == 0) {
    //     m_game->changeScene("WIN", std::make_shared<Scene_Win>(m_game, final), true); // final true when all levels completed
    // }
}

void Scene_World::saveState() {
    PROFILE_FUNCTION();

    Level::save(m_game->getExeDir() + m_levelPath + ".snapshot", m_entityManager, m_bgColor);
}

bool Scene_World::loadState() {
    PROFILE_FUNCTION();

    auto path = m_game->getExeDir() + m_levelPath + ".snapshot";
    if (!std::filesystem::exists(path)) {
        return false;
    }

    Level::loadFromFile(path, m_game, m_entityManager, m_bgColor);
    return true;
}

void Scene_World::onEnd() {
    PROFILE_FUNCTION();

    if (m_sTeleport.curtain != nullptr) {
        m_sTeleport.curtain->destroy();
        m_sTeleport.curtain = nullptr;
    }

    saveState();
    EntityMemoryPool::Instance().clear();
}

void Scene_World::onPause(const std::string & nextScene) {
    // TODO: Implement scene end
    //       - stop the music
    //       - play the menu music
    //       - change scene to menu
}

void Scene_World::onResume(const std::string & prevScene) {

}

void Scene_World::sTeleport() {
    PROFILE_FUNCTION();

    for (auto e : m_entityManager.getEntities()) {
        if (e.hasComponent<CTeleport>()) {
            auto prevLvl = e.getComponent<CTeleport>().level - 1;
            bool prevLevelPass = (prevLvl < 0) || m_game->getSettings().levels[prevLvl].pass;

            auto lvl = e.getComponent<CTeleport>().level;

            // Открываем доступ к порталу только если предыдущий уровень пройден а текщий нет
            auto pass = prevLevelPass && !m_game->getSettings().levels[lvl].pass;

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
        m_game->assets().getSound("Portal")->play();

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

                m_game->changeScene("PLAY", std::make_shared<Scene_Play>(m_game, m_game->getSettings().levels[m_sTeleport.level]), true);
                // m_sTeleport.curtain->destroy();
                // m_sTeleport.curtain = nullptr;
                m_sTeleport.level = -1;
                m_sTeleport.counter = 0;
                m_sTeleport.state = STeleport::State::increaseAlpha;
                return;
            }
            break;
    }

    m_sTeleport.counter += 2;
}

const std::string & Scene_World::levelPath() const {
    return m_levelPath;
};
