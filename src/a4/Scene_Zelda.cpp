#include "Scene_Zelda.h"
#include "Scene_Win.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include "SFML/Window/Keyboard.hpp"
#include "SFML/Graphics/Color.hpp"
#include "Scene_Menu.h"
#include "Profiler.h"

#include <math.h>
#include <fstream>
#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()
#include <iostream>

Scene_Zelda::Scene_Zelda(GameEngine * game, const std::string & levelPath)
    : Scene(game)
    , m_levelPath(levelPath)
    , m_frameTime(m_game->assets().getFont("Mario"), "0", 16)
{
    init(m_levelPath);
}

void Scene_Zelda::init(const std::string & levelPath) {
    srand(time(0));
    loadLevel(levelPath);

    registerAction(sf::Keyboard::Key::Escape, "QUIT");
    registerAction(sf::Keyboard::Key::P, "PAUSE");
    registerAction(sf::Keyboard::Key::Y, "TOGGLE_FOLLOW");
    registerAction(sf::Keyboard::Key::T, "TOGGLE_TEXTURE");   // Toggle drawing (T)extures
    registerAction(sf::Keyboard::Key::D, "TOGGLE_DEBUG");

    registerAction(sf::Keyboard::Key::Up, "MOVE_UP");
    registerAction(sf::Keyboard::Key::Down, "MOVE_DOWN");
    registerAction(sf::Keyboard::Key::Left, "MOVE_LEFT");
    registerAction(sf::Keyboard::Key::Right, "MOVE_RIGHT");
    registerAction(sf::Keyboard::Key::Space, "ATTACK");
}

void Scene_Zelda::loadLevel(const std::string & filename) {
    PROFILE_FUNCTION();

    m_entityManager = EntityManager();

    // TODO: Load the level file and put all entities in the manager
    //       Use the getPosition() function below to convert room-tile coords to game world coords

    // reset the entity manager every time we load a level
    m_entityManager = EntityManager();

    std::string token;
    std::ifstream fin(filename);
    int minRoomX = 0, maxRoomX = 0, minRoomY = 0, maxRoomY = 0;
    std::map<int, std::map<int, Entity>> buf;

    while (fin >> token) {
        if (token == "Tile") {
            // Tile N RX RY TX TY BM BV
            //   Animation Name   N      string
            //   Room Coordinate  RX RY  int, int
            //   Tile Position    TX TY  int, int
            //   Blocks Movement  BM     int (1 = true, 0 = false)
            //   Blocks Vision    BV     int (1 = true, 0 = false)

            std::string animName;
            float rx, ry, tx, ty;
            int bm, bv;
            fin >> animName >> rx >> ry >> tx >> ty >> bm >> bv;

            auto e = m_entityManager.addEntity("tile");
            e.addComponent<CAnimation>(m_game->assets().getAnimation(animName), true);
            e.addComponent<CBoundingBox>(m_game->assets().getAnimation(animName).getSize(), bm, bv);
            e.addComponent<CTransform>(getPosition(rx, ry, tx, ty));

            if (rx < minRoomX) { minRoomX = rx; }
            if (rx > maxRoomX) { maxRoomX = rx; }
            if (ry < minRoomY) { minRoomY = ry; }
            if (ry > maxRoomY) { maxRoomY = ry; }
            buf[rx*20+tx][ry*12+ty] = e;
        } else if (token == "NPC") {
            // NPC N RX RY TX TY BM BV H D AI ...
            //   Animation Name    N      string
            //   Room Coordinate   RX RY  int, int
            //   Tile Position     TX TY  int, int
            //   Blocks Movement   BM     int (1 = true, 0 = false)
            //   Blocks Vision     BV     int (1 = true, 0 = false)
            //   Max Health        H      int > 0
            //   Damage            D      int > 0
            //   AI Behavior Name  AI     string
            //   AI Parameters     ...    (see below)

            // AI = Follow
            //   ... = S
            //   Follow Speed  S  float (speed to follow player)

            // AI = Patrol
            //   ... = S N X1 Y1 X2 Y2 ... XN YN
            //   Patrol Speed      S      float
            //   Patrol Positions  N      int (number of patrol positions)
            //   Position 1-N      Xi Yi  int, int (Tile Position of Patrol Position i)

            // For Example:
            // NPC Tektite 0 0 15 10 0 0 2 1 Patrol 2 4 15 10 15 7 17 7 17 10
            // - Spawn an NPC with animation name Tektie in room (0,0) with tile pos (15,10)
            // - This NPC has max health of 2 and damage of 1
            // - This NPC does not block movement or vision
            // - This NPC has a Patrol AI with speed 2 and 4 positions, each in room (0,0)
            //   Positions: (15, 10) (15, 7) (17, 7) (17, 10)

            std::string animName;
            int rx, ry, tx, ty, bm, bv, h, d;

            std::string ai;
            float s;
            int n, x, y;

            fin >> animName >> rx >> ry >> tx >> ty >> bm >> bv >> h >> d >> ai;

            auto e = m_entityManager.addEntity("npc");
            e.addComponent<CAnimation>(m_game->assets().getAnimation(animName), true);
            // Уменьшение на 2 пикселя нужно, чтобы NPC могли проходить между рядом стоящими тайлами и не "проваливались" в тайл
            e.addComponent<CBoundingBox>(m_game->assets().getAnimation(animName).getSize()-4, bm, bv); 
            auto pos = getPosition(rx, ry, tx, ty);
            e.addComponent<CTransform>(pos);
            e.addComponent<CHealth>(h, h);
            e.addComponent<CDamage>(d);

            if (ai == "Follow") {
                fin >> s;
                e.addComponent<CFollowPlayer>(pos, s);
            } else if (ai == "Patrol") {
                fin >> s >> n;

                std::vector<Vec2> v;
                for (int i=0; i<n; i++) {
                    fin >> x >> y;
                    v.push_back(getPosition(rx, ry, x, y));
                }

                e.addComponent<CPatrol>(v, s);
            }
        } else if (token == "Player") {
            // Player X Y BX BY S H
            //   Spawn Position     X Y    int, int
            //   Bounding Box Size  BX BY  int, int
            //   Speed              S      float
            //   Max Health         H      int

            fin >> m_playerConfig.X >> m_playerConfig.Y
                >> m_playerConfig.BX >> m_playerConfig.BY
                >> m_playerConfig.SPEED >> m_playerConfig.HEALTH;

            spawnPlayer();
        }
    }

    int width = (maxRoomX-minRoomX+1)*20;
    int height = (maxRoomY-minRoomY+1)*12;
    
    if (minRoomX < 0) m_shift.x = (-minRoomX)*20;
    if (minRoomY < 0) m_shift.y = (-minRoomY)*12;

    m_tiles = std::vector<std::vector<Entity>>(width);
    for (int i=0; i<width; i++) {
        m_tiles[i] = std::vector<Entity>(height);
    }

    for (const auto& [x, m] : buf) {
        for (const auto& [y, e] : m) {
            m_tiles[x+m_shift.x][y+m_shift.y] = e;
        }
    }
}

Vec2 Scene_Zelda::getTileIndex(Vec2 pos) {
    pos = pos / 64 + m_shift;
    pos.x = floor(pos.x);
    pos.y = floor(pos.y);
    return pos;
}

EntityVec Scene_Zelda::getTilesAround(Vec2 tileIndex) {
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

EntityVec Scene_Zelda::getViewTiles(Vec2 tl, Vec2 br) {
    PROFILE_FUNCTION();
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

Vec2 Scene_Zelda::getPosition(int rx, int ry, int tx, int ty) const {
    // TODO: Implement this function, which takes in the room (rx, ry) coordinate
    //       as well as the tile (tx, ty) coordinate, and returns the Vec2 game world
    //       position of the center of the entity.

    return Vec2(
        1280*rx + 64*tx + 32,
        768*ry + 64*ty + 32
    );
}

void Scene_Zelda::spawnPlayer() {
    PROFILE_FUNCTION();
    m_player = m_entityManager.addEntity("player");
    m_player.addComponent<CTransform>(Vec2(m_playerConfig.X, m_playerConfig.Y));
    m_player.addComponent<CAnimation>(m_game->assets().getAnimation("StandDown"), true);
    m_player.addComponent<CBoundingBox>(Vec2(m_playerConfig.BX, m_playerConfig.BY), true, false);
    m_player.addComponent<CHealth>(m_playerConfig.HEALTH, m_playerConfig.HEALTH);
    m_player.addComponent<CState>();

    // TODO: Implements this function so that it uses the parameters input from the level file
    //       Those parameters should be stored in the m_playerConfig variable
}

void Scene_Zelda::spawnSword(Entity entity) {
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

    auto& stateAttack = entity.getComponent<CState>().attack;
    m_game->assets().getSound("Slash")->play();

    e.onDestroy([&stateAttack]() {
        stateAttack = false;
    });
}

void Scene_Zelda::moveSword(Entity entity) {
    if (!entity.hasComponent<CState>()) {
        return;
    }

    auto s = entity.getComponent<CState>();
    auto p = entity.getComponent<CTransform>().pos;

    for (auto e : m_entityManager.getEntities("sword")) {
        switch (s.direction) {
            case Direction::UP:
            setAnimation(e, "SwordUp", true, 1);
            p.y -= e.getComponent<CAnimation>().animation.getSize().y;
            e.addComponent<CBoundingBox>(Vec2(5, 32), true, false);
            break;

            case Direction::DOWN:
            setAnimation(e, "SwordDown", true, 1);
            p.y += entity.getComponent<CAnimation>().animation.getSize().y;
            e.addComponent<CBoundingBox>(Vec2(5, 32), true, false);
            break;

            case Direction::LEFT:
            setAnimation(e, "SwordRight", true, -1);
            p.x -= e.getComponent<CAnimation>().animation.getSize().x;
            e.addComponent<CBoundingBox>(Vec2(32, 5), true, false);
            break;

            case Direction::RIGHT:
            setAnimation(e, "SwordRight", true, 1);
            p.x += entity.getComponent<CAnimation>().animation.getSize().x;
            e.addComponent<CBoundingBox>(Vec2(32, 5), true, false);
            break;
        }

        e.getComponent<CTransform>().pos = p;
    }
}

void Scene_Zelda::update() {
    PROFILE_FUNCTION();

    if (!m_paused) {
        m_entityManager.update();

        sAI();
        sMovement();
        sCollision();
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

void Scene_Zelda::sMovement() {
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
        t.pos.y -= m_playerConfig.SPEED;
        s.direction = Direction::UP;
        s.run = true;
    }

    if (i.down) {
        t.pos.y += m_playerConfig.SPEED;
        s.direction = Direction::DOWN;
        s.run = true;
    }

    if (i.left) {
        t.pos.x -= m_playerConfig.SPEED;
        s.direction = Direction::LEFT;
        s.run = true;
    }

    if (i.right) {
        t.pos.x += m_playerConfig.SPEED;
        s.direction = Direction::RIGHT;
        s.run = true;
    }

    if (i.attack && !s.attack) {
        spawnSword(m_player);
        s.attack = true;
    }

    auto room = getRoomByPos(m_player.getComponent<CTransform>().pos);

    if (!roomHasEntities(room)) {
        t.pos = t.prevPos;
    }

    moveSword(m_player);
}

void Scene_Zelda::sDoAction(const Action & action) {
    PROFILE_FUNCTION();
    // TODO: Implement all actions described for the game here
    //       Only the setting of the player's input component variables should be set here
    //       Do minimal logic in this function

    if (action.type() == "START") {
             if (action.name() == "PAUSE")          { setPaused(!m_paused); }
        else if (action.name() == "QUIT")           { onEnd(); }
        else if (action.name() == "TOGGLE_FOLLOW")  { m_follow = !m_follow; }
        else if (action.name() == "TOGGLE_TEXTURE") { m_drawTextures = !m_drawTextures; }
        else if (action.name() == "TOGGLE_DEBUG")   { m_drawDebug = !m_drawDebug; }
        else if (action.name() == "MOVE_UP")        { m_player.getComponent<CInput>().up = true; }
        else if (action.name() == "MOVE_DOWN")      { m_player.getComponent<CInput>().down = true; }
        else if (action.name() == "MOVE_LEFT")      { m_player.getComponent<CInput>().left = true; }
        else if (action.name() == "MOVE_RIGHT")     { m_player.getComponent<CInput>().right = true; }
        else if (action.name() == "ATTACK")         { m_player.getComponent<CInput>().attack = true; }        
    } else if (action.type() == "END") {
             if (action.name() == "MOVE_UP")    { m_player.getComponent<CInput>().up = false; }
        else if (action.name() == "MOVE_DOWN")  { m_player.getComponent<CInput>().down = false; }
        else if (action.name() == "MOVE_LEFT")  { m_player.getComponent<CInput>().left = false; }
        else if (action.name() == "MOVE_RIGHT") { m_player.getComponent<CInput>().right = false; }
        else if (action.name() == "ATTACK")     { m_player.getComponent<CInput>().attack = false; }
    }
}

void Scene_Zelda::sAI() {
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

            if (newSpeed.length() <= 500) {
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
                    if (Physics::EntityIntersect(playerPos, t.pos, x)) {
                        intersect = true;
                        break;
                    }
                }
            }

            if (newSpeed.length() > 500 || intersect) {
                newSpeed = p.home-t.pos;
            }

            if (newSpeed.length() > 5) {
                newSpeed /= newSpeed.length();
                newSpeed *= p.speed;

                auto oldSpeed = t.velocity;
                auto delta = (newSpeed-oldSpeed) / 120;
                newSpeed += delta;

                t.prevPos = t.pos;
                t.pos += newSpeed;
                t.velocity = newSpeed;
            }
        }
    }
}

void Scene_Zelda::sStatus() {
    PROFILE_FUNCTION();
    // TODO: Implement Lifespan here
    //       Implement Invincibility Frames here
    //...

    if (m_player.getComponent<CHealth>().current <= 0) {
        m_game->assets().getSound("LinkDie")->play();
        reload();
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

void Scene_Zelda::sCollision() {
    PROFILE_FUNCTION();
    // TODO: Implement entity - tile collisions
    //       Implement player - enemy collisions with appropriate damage calculations
    //       Implement sword - NPC collisions
    //       Implement black tile collisions / 'teleporting'
    //       Implement entity - heart collisions and life gain logic
    //
    //       You may want to use helper functions for these behaviors or this function will get long


    auto tilesAroundPlayer = getTilesAround(getTileIndex(m_player.getComponent<CTransform>().pos));

    // teleport
    std::vector<Entity> teleports;

    for (auto e : tilesAroundPlayer) {
        auto o = Physics::GetOverlap(m_player, e);
        if (o.x <= 0 || o.y <= 0) {
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

        if (e.getComponent<CAnimation>().animation.getName() == "Black" && o.x > 32 && o.y > 32) {
            for (auto t : m_entityManager.getEntities("tile")) {
                if (t.getComponent<CAnimation>().animation.getName() == "Black" && t.getComponent<CTransform>().pos != e.getComponent<CTransform>().pos) {
                    teleports.push_back(t);
                }
            }
        }
    }

    if (teleports.size() > 0) {
        auto e = teleports.at(rand() % teleports.size());
        auto pos = e.getComponent<CTransform>().pos;
        pos.y += 64;
        m_player.getComponent<CTransform>().prevPos = m_player.getComponent<CTransform>().pos;
        m_player.getComponent<CTransform>().pos = pos;
        m_player.getComponent<CState>().direction = Direction::DOWN;
    }

    auto checkPlayer = (!m_player.hasComponent<CInvincibility>() || m_player.getComponent<CInvincibility>().iframes <= 0);
    auto swordIt = m_entityManager.getEntities("sword");

    for (auto n : m_entityManager.getEntities("npc")) {
        // npc x player
        if (checkPlayer) {
            auto o = Physics::GetOverlap(m_player, n);

            if (o.x > 0 && o.y > 0) {
                auto d = n.getComponent<CDamage>().damage;
                m_player.getComponent<CHealth>().current -= d;
                m_player.addComponent<CInvincibility>(30);
                if (m_player.getComponent<CHealth>().current > 0) {
                    m_game->assets().getSound("LinkHit")->play();
                }
            }
        }

        // npc x sword
        if (swordIt.size() > 0 && (!n.hasComponent<CInvincibility>() || n.getComponent<CInvincibility>().iframes <= 0)) {
            auto sword = swordIt.at(0);
            
            auto o = Physics::GetOverlap(sword, n);
            if (o.x > 0 && o.y > 0) {
                n.getComponent<CHealth>().current -= 1;
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

        auto tilesAroundNPC = getTilesAround(getTileIndex(n.getComponent<CTransform>().pos));
        for (auto t : tilesAroundNPC) {
            auto o = Physics::GetOverlap(n, t);
            if (o.x <= 0 || o.y <= 0) {
                continue;
            }

            if (t.hasComponent<CBoundingBox>() && t.getComponent<CBoundingBox>().blockMove) {
                // Вычитать и прибавлять 1 из предыдущей позиции нужно, чтобы NPC не "цеплялся" за тайлы при приследовании игрока.

                auto po = Physics::GetPreviousOverlap(n, t);
                auto& nt = n.getComponent<CTransform>();

                if (po.x > 0) {
                    if (nt.prevPos.y > nt.pos.y) {
                        nt.pos.y = nt.prevPos.y + 1;
                    } else {
                        nt.pos.y = nt.prevPos.y - 1;
                    }
                }

                if (po.y > 0) {
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

void Scene_Zelda::sAnimation() {
    PROFILE_FUNCTION();
    // TODO: Implement player facing direction animation
    //       Implement sword animation based on player facing
    //         The sword should move if the player changes direction mid swing
    //       Implement destruction of entities with non-repeating finished animations

    auto s = m_player.getComponent<CState>();
    switch (s.direction) {
        case Direction::UP:
        if (s.attack) {
            setAnimation(m_player, "AtkUp", true, 1);
        } else if (s.run) { setAnimation(m_player, "RunUp", true, 1); }
        else       { setAnimation(m_player, "StandUp", true, 1); }
        break;

        case Direction::DOWN:
        if (s.attack) {
            setAnimation(m_player, "AtkDown", true, 1);
        } else if (s.run) { setAnimation(m_player, "RunDown", true, 1); }
        else       { setAnimation(m_player, "StandDown", true, 1); }
        break;

        case Direction::LEFT:
        if (s.attack) {
            setAnimation(m_player, "AtkRight", true, -1);
        } else if (s.run) { setAnimation(m_player, "RunRight", true, -1); }
        else       { setAnimation(m_player, "StandRight", true, -1); }
        break;

        case Direction::RIGHT:
        if (s.attack) {
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

void Scene_Zelda::sCamera() {
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

void Scene_Zelda::onEnd() {
    // TODO: Implement scene end
    //       - stop the music
    //       - play the menu music
    //       - change scene to menu
    m_game->changeScene("MENU", std::make_shared<Scene_Menu>(m_game));
}

void Scene_Zelda::sRender() {
    PROFILE_FUNCTION();
    m_game->window().clear(sf::Color(255, 192, 122));

    sf::RectangleShape tick({ 1.0f, 6.0f });
    tick.setFillColor(sf::Color::Black);
    Vec2 healthSize(64, 6);

    sf::RectangleShape healthRect({ healthSize.x, healthSize.y });
    healthRect.setFillColor(sf::Color(96, 96, 96));
    healthRect.setOutlineColor(sf::Color::Black);
    healthRect.setOutlineThickness(2);

    sf::RectangleShape healthTick({ healthSize.x, healthSize.y });
    healthTick.setFillColor(sf::Color(255, 0, 0));
    healthTick.setOutlineThickness(0);

    // draw all Entity textures / animations
    if (m_drawTextures) {
        PROFILE_SCOPE("drawTextures");
        auto viewCenter = Vec2(m_game->window().getView().getCenter().x, m_game->window().getView().getCenter().y);
        auto viewSize = Vec2(m_game->window().getView().getSize().x, m_game->window().getView().getSize().y)/2;

        auto tl = getTileIndex(viewCenter-viewSize);
        auto br = getTileIndex(viewCenter+viewSize);

        auto viewTiles = getViewTiles(tl, br);

        {PROFILE_SCOPE("copy_entities_to_viewTiles");
        for (auto e : m_entityManager.getEntities()) {
            if (e.tag() != "tile") {
                viewTiles.push_back(e);
            }
        }}

        {PROFILE_SCOPE("foreach_viewTiles");
        for (auto e : viewTiles) {
            auto & transform = e.getComponent<CTransform>();
            sf::Color c = sf::Color::White;
            if (e.hasComponent<CInvincibility>()) {
                c = sf::Color(255, 255, 255, 128);
            }

            if (e.hasComponent<CAnimation>()) {
                auto& animation = e.getComponent<CAnimation>().animation;
                animation.getSprite().setRotation(sf::degrees(transform.angle));
                animation.getSprite().setPosition({ transform.pos.x, transform.pos.y });
                animation.getSprite().setScale({ transform.scale.x, transform.scale.y });
                animation.getSprite().setColor(c);

                if (e.tag() == "player") {
                    auto shader = m_game->assets().getShader("Player");
                    shader->setUniform("myvar", 5.f);
                    shader->setUniform("texture", sf::Shader::CurrentTexture);
                    m_game->window().draw(animation.getSprite(), &(*shader));
                } else {
                    m_game->window().draw(animation.getSprite());
                }
            }

            if (e.hasComponent<CHealth>()) {
                auto& h = e.getComponent<CHealth>();
                healthRect.setPosition({ transform.pos.x - 32, transform.pos.y - 48 });
                m_game->window().draw(healthRect);

                float ratio = (float)(h.current) / h.max;
                healthTick.setPosition({ transform.pos.x - 32, transform.pos.y - 48 });
                healthTick.setSize({ healthSize.x*ratio, healthSize.y });
                m_game->window().draw(healthTick);

                for (int i = 0; i < h.max; i++) {
                    tick.setPosition(healthTick.getPosition() + sf::Vector2f(i * 64 * 1 / h.max, 0));
                    m_game->window().draw(tick);
                }
            }
        }}
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
                auto & box = e.getComponent<CBoundingBox>();
                auto & transform = e.getComponent<CTransform>();
                
                // rect.setSize(sf::Vector2f(box.size.x - 1, box.size.y - 1));
                // rect.setOrigin(sf::Vector2f(box.halfSize.x, box.halfSize.y));
                rect.setPosition({transform.pos.x, transform.pos.y });

                if (box.blockMove && box.blockVision) { rect.setOutlineColor(sf::Color::Black); }
                if (box.blockMove && !box.blockVision) { rect.setOutlineColor(sf::Color::Blue); }
                if (!box.blockMove && box.blockVision) { rect.setOutlineColor(sf::Color::Red); }
                if (!box.blockMove && !box.blockVision) { rect.setOutlineColor(sf::Color::White); }
                
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

    m_frameTime.setPosition(m_game->window().mapPixelToCoords(sf::Vector2i(10, 10)));
    m_game->window().draw(m_frameTime);
}

void Scene_Zelda::setPaused(bool v) {
    m_paused = v;
};

void Scene_Zelda::reload() {
    m_game->changeScene("PLAY", std::make_shared<Scene_Zelda>(m_game, m_levelPath), true);
}

void Scene_Zelda::setAnimation(Entity e, const std::string & animName, bool repeat, float scale) {
    if (e.getComponent<CTransform>().scale.x != scale) {
        e.getComponent<CTransform>().scale.x = scale;
    }

    if (e.hasComponent<CAnimation>()) {
        if (e.getComponent<CAnimation>().animation.getName() == animName) {
            return;
        }
    }

    e.addComponent<CAnimation>(m_game->assets().getAnimation(animName), repeat);
}

Vec2 Scene_Zelda::getRoomByPos(const Vec2 & pos) {
    auto r = Vec2((int)pos.x / 1280, (int)pos.y / 768);

    if (pos.x < 0) {
        r.x -= 1;
    }

    if (pos.y < 0) {
        r.y -= 1;
    }

    return r;
}

bool Scene_Zelda::roomHasEntities(const Vec2 & room) {
    Vec2 roomSize = { 1280, 768 };
    auto roomCenter = Vec2(room.x*roomSize.x, room.y*roomSize.y)+roomSize/2;

    for (auto e : m_entityManager.getEntities()) {
        if (e.tag() == "player" || e.tag() == "sword") {
            continue;
        }

        auto ep = e.getComponent<CTransform>().pos;
        auto es = e.getComponent<CBoundingBox>().size;
        
        if (intersect(roomCenter, roomSize, ep, es)) {
            return true;
        }
    }

    return false;
}

bool Scene_Zelda::intersect(Vec2 ap, Vec2 as, Vec2 bp, Vec2 bs) {
    auto delta = Vec2(abs(bp.x-ap.x), abs(bp.y-ap.y));

    as /= 2;
    bs /= 2;
    auto o = Vec2(as.x + bs.x - delta.x, as.y + bs.y - delta.y);

    return o.x > 0 && o.y > 0;
}

void Scene_Zelda::sWin() {
    PROFILE_FUNCTION();
    if (m_entityManager.getEntities("npc").size() == 0) {
        m_game->changeScene("WIN", std::make_shared<Scene_Win>(m_game), true);
    }
}