#include "Scene_Play.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include "Action.h"
#include "Scene_Menu.h"

#include <iostream>
#include <fstream>
#include <math.h>

Scene_Play::Scene_Play(GameEngine * gameEngine, const std::string & levelPath)
    : Scene(gameEngine)
    , m_levelPath(levelPath)
    , m_gridText(m_game->assets().getFont("Mario"), "", 30)
    , m_playerState(m_game->assets().getFont("Mario"), "State: ", 15)
{
    init(m_levelPath);
}

void Scene_Play::init(const std::string & levelPath) {
    registerAction(sf::Keyboard::Key::P, "PAUSE");
    registerAction(sf::Keyboard::Key::Escape, "QUIT");
    registerAction(sf::Keyboard::Key::T, "TOGGLE_TEXTURE");   // Toggle drawing (T)extures
    registerAction(sf::Keyboard::Key::C, "TOGGLE_COLLISION"); // Toggle drawing (C)ollision
    registerAction(sf::Keyboard::Key::G, "TOGGLE_GRID");      // Toggle drawing (G)rid

    registerAction(sf::Keyboard::Key::Left, "MOVE_LEFT");
    registerAction(sf::Keyboard::Key::Right, "MOVE_RIGHT");
    registerAction(sf::Keyboard::Key::Up, "MOVE_UP");
    registerAction(sf::Keyboard::Key::Space, "SHOOT");

    // TODO: Register all other gameplay Actions

    m_gridText.setCharacterSize(12);
    m_gridText.setFont(m_game->assets().getFont("Tech"));
    m_playerState.setPosition({10, 10});

    loadLevel(levelPath);
}

Vec2 Scene_Play::gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity) {
    // TODO: This function takes in a grid (x, y) position and an Entity
    //       Return a Vec2 indicating where the CENTER position of the Entity should be
    //       You must use the Entity's Animation size to position it correctly
    //       The size of the grid width and height is stored in m_gridSize.x and m_gridSize.y
    //       The bottom-left corner of the Animation should align with the bottom left of the grid cell

    return Vec2(
        gridX * m_gridSize.x + entity->getComponent<CAnimation>().animation.getSize().x/2,
        m_game->window().getSize().y - gridY * m_gridSize.y - entity->getComponent<CAnimation>().animation.getSize().y/2
    );
}

void Scene_Play::loadLevel(const std::string & filename) {
    // reset the entity manager every time we load a level
    m_entityManager = EntityManager();

    std::string token;
    std::ifstream fin(filename);

    while (fin >> token) {
        if (token == "Tile") {
            //Tile N GX GY
            //  Animation Name  N   std::string (Animation asset name for this tile)
            //  GX Grid X Pos   GX  float
            //  GY Grid Y Pos   GY  float

            std::string animName;
            float gx, gy;
            fin >> animName >> gx >> gy;

            auto e = m_entityManager.addEntity("tile");
            e->addComponent<CAnimation>(m_game->assets().getAnimation(animName), true);
            e->addComponent<CBoundingBox>(m_game->assets().getAnimation(animName).getSize());
            e->addComponent<CTransform>(gridToMidPixel(gx, gy, e));
            e->addComponent<CDraggable>();
        } else if (token == "Dec") {
            //Dec N X Y
            //  Animation Name  N  std::string (Animation asset name for this tile)
            //  X Position      X  float
            //  Y Position      Y  float

            std::string animName;
            float gx, gy;
            fin >> animName >> gx >> gy;

            auto e = m_entityManager.addEntity("dec");
            e->addComponent<CAnimation>(m_game->assets().getAnimation(animName), true);
            e->addComponent<CTransform>(gridToMidPixel(gx, gy, e));
        } else if (token == "Player") {
            //Player GX GY CW CH SX SY SM GY B
            //  GX, GY Grid Pos   X, Y    float, float (starting position of player)
            //  BoundingBox W/H   CW, CH  float, float
            //  Left/Right Speed  SX      float
            //  Jump Speed        SY      float
            //  Max Speed         SM      float
            //  Gravity           GY      float
            //  Bullet Animation  B       std::string (Animation asset to use for bullets)

            fin >> m_playerConfig.X >> m_playerConfig.Y
                >> m_playerConfig.CW >> m_playerConfig.CH
                >> m_playerConfig.SPEED >> m_playerConfig.JUMP >> m_playerConfig.MAXSPEED
                >> m_playerConfig.GRAVITY >> m_playerConfig.WEAPON;

            spawnPlayer();
        }
    }
}

void Scene_Play::spawnPlayer() {
    m_player = m_entityManager.addEntity("player");
    m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Stand"), true);
    m_player->addComponent<CBoundingBox>(Vec2(m_playerConfig.CW, m_playerConfig.CH));
    m_player->addComponent<CTransform>(gridToMidPixel(m_playerConfig.X, m_playerConfig.Y, m_player));
    m_player->addComponent<CGravity>(m_playerConfig.GRAVITY);
    m_player->addComponent<CDraggable>();

    // TODO: be sure to add the remaining components to the player
}

void Scene_Play::spawnBullet() {
    // TODO: this should spawn a bullet at the given entity, going in the direction the entity is facing
    auto e = m_entityManager.addEntity("bullet");
    e->addComponent<CAnimation>(m_game->assets().getAnimation(m_playerConfig.WEAPON), true);
    e->addComponent<CBoundingBox>(m_game->assets().getAnimation(m_playerConfig.WEAPON).getSize());
    e->addComponent<CTransform>(m_player->getComponent<CTransform>().pos);
    e->addComponent<CLifespan>(180, m_currentFrame);

    auto m = m_player->getComponent<CTransform>().scale.x; // -1/1
    e->getComponent<CTransform>().velocity.x = m*m_playerConfig.MAXSPEED;
    e->getComponent<CTransform>().scale.x = m;
    e->getComponent<CTransform>().pos.x += m*(m_player->getComponent<CAnimation>().animation.getSize().x/4);
}

void Scene_Play::spawnCoin(std::shared_ptr<Entity> block) {
    auto e = m_entityManager.addEntity("coin");
    e->addComponent<CAnimation>(m_game->assets().getAnimation("Coin"), true);
    e->addComponent<CTransform>(block->getComponent<CTransform>().pos);
    e->getComponent<CTransform>().pos.y -= block->getComponent<CAnimation>().animation.getSize().y;
    e->addComponent<CLifespan>(30, m_currentFrame);
}

void Scene_Play::update() {
    // TODO: implement pause functionality
    if (!m_paused) {
        m_entityManager.update();

        sMovement();
        sLifespan();
        sCollision();
        sAnimation();
        sDragAndDrop();
    }

    sRender();

    if (!m_paused) {
        m_currentFrame++;
    }
}

void Scene_Play::sMovement() {
    auto& input = m_player->getComponent<CInput>();
    auto& trans = m_player->getComponent<CTransform>();

    if (input.shoot && input.canShoot) {
        spawnBullet();
        m_player->getComponent<CInput>().canShoot = false;
    }

    trans.velocity.x = 0;
    //m_player->getComponent<CState>().state = "stand";

    if (input.up && m_player->getComponent<CState>().state != "air" ) {
        trans.velocity.y = m_playerConfig.JUMP;
        m_player->getComponent<CState>().state = "air";
    }

    if (input.left) {
        trans.velocity.x = -m_playerConfig.SPEED;
        if (m_player->getComponent<CState>().state != "air") {
            m_player->getComponent<CState>().state = "run";
        }
        m_player->getComponent<CTransform>().scale.x = -1;
    }

    if (input.right) {
        trans.velocity.x = m_playerConfig.SPEED;
        if (m_player->getComponent<CState>().state != "air") {
            m_player->getComponent<CState>().state = "run";
        }
        m_player->getComponent<CTransform>().scale.x = 1;
    }

    for (auto e : m_entityManager.getEntities()) {
        if (e->hasComponent<CGravity>()) {
            e->getComponent<CTransform>().velocity.y += e->getComponent<CGravity>().gravity;

            if (e->getComponent<CTransform>().velocity.y > m_playerConfig.MAXSPEED) {
                e->getComponent<CTransform>().velocity.y = m_playerConfig.MAXSPEED;
            }
        }

        e->getComponent<CTransform>().prevPos = e->getComponent<CTransform>().pos;
        e->getComponent<CTransform>().pos += e->getComponent<CTransform>().velocity;
    }

    // Не позволяем игроку уходить за левый край экрана
    if (m_player->getComponent<CTransform>().pos.x < m_player->getComponent<CAnimation>().animation.getSize().x / 2) {
        m_player->getComponent<CTransform>().pos.x = m_player->getComponent<CAnimation>().animation.getSize().x / 2;
    }

    // Если игрок падает ниже экрана, перезагружаем уровень
    if (m_player->getComponent<CTransform>().pos.y > m_game->window().getSize().y + 100) {
        reload();
    }

    // TODO: Implement player movement / jumping based on its CInput component
    // TODO: Implement gravity's effect on the player
    // TODO: Implement the maximum player speed in both X and Y directions
    // NOTE: Setting an entity's scale.x to -1/1 will make it face to the left/right
}

void Scene_Play::sLifespan() {
    // TODO: Check lifespan of entities that have them, and destroy them if they go over
    for (auto e : m_entityManager.getEntities()) {
        if (!e->hasComponent<CLifespan>()) {
            continue;
        }

        auto c = e->getComponent<CLifespan>();
        if (m_currentFrame >= c.frameCreated + c.lifespan) {
            e->destroy();
        }
    }
}

void Scene_Play::sCollision() {
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

    auto pp = m_player->getComponent<CTransform>().prevPos;
    m_player->getComponent<CState>().state = "air";

    for (auto e : m_entityManager.getEntities("tile")) {
        auto o = Physics::GetOverlap(m_player, e);

        // Коллизии нет
        if (o.x <= 0 || o.y <= 0) {
            continue;
        }

        auto p = Physics::GetPreviousOverlap(m_player, e);
        auto ep = e->getComponent<CTransform>().prevPos;
        std::string direction = "";

        if (p.x > 0 && p.y > 0) {
            m_player->getComponent<CTransform>().pos -= o;
            m_player->getComponent<CTransform>().pos -= p;
            m_player->getComponent<CTransform>().velocity = Vec2(0, 0);
        } else if (p.x > 0) {
            if (pp.y < ep.y) {
                m_player->getComponent<CTransform>().pos.y -= o.y;
                direction = "down";
                if (m_player->getComponent<CInput>().left || m_player->getComponent<CInput>().right) {
                    m_player->getComponent<CState>().state = "run";
                } else {
                    m_player->getComponent<CState>().state = "stand";
                }
            } else {
                m_player->getComponent<CTransform>().pos.y += o.y;
                direction = "up";
            }
            m_player->getComponent<CTransform>().velocity.y = 0;
        } else if (p.y > 0) {
            if (pp.x < ep.x) {
                m_player->getComponent<CTransform>().pos.x -= o.x;
                direction = "right";
            } else {
                m_player->getComponent<CTransform>().pos.x += o.x;
                direction = "left";
            }
            m_player->getComponent<CTransform>().velocity.x = 0;
        }

        if (e->hasAnimation("Brick") && direction == "up") {
            e->addComponent<CAnimation>(m_game->assets().getAnimation("Explosion"), false);
            e->removeComponent<CBoundingBox>();
        }

        if (e->hasAnimation("Question") && direction == "up") {
            spawnCoin(e);
            e->addComponent<CAnimation>(m_game->assets().getAnimation("Question2"), true);
        }
    }

    for (auto b : m_entityManager.getEntities("bullet")) {
        for (auto e : m_entityManager.getEntities("tile")) {
            auto o = Physics::GetOverlap(b, e);

            // Коллизии нет
            if (o.x <= 0 || o.y <= 0) {
                continue;
            }

            b->destroy();

            if (e->hasAnimation("Brick")) {
                e->addComponent<CAnimation>(m_game->assets().getAnimation("Explosion"), false);
                e->removeComponent<CBoundingBox>();
            }
        }
    }
}

void Scene_Play::sDoAction(const Action &action) {
    if (action.type() == "START") {
             if (action.name() == "TOGGLE_TEXTURE")   { m_drawTextures = !m_drawTextures; }
        else if (action.name() == "TOGGLE_COLLISION") { m_drawCollision = !m_drawCollision; }
        else if (action.name() == "TOGGLE_GRID")      { m_drawGrid = !m_drawGrid; }
        else if (action.name() == "PAUSE")            { setPaused(!m_paused); }
        else if (action.name() == "QUIT")             { onEnd(); }
        else if (action.name() == "MOVE_LEFT")        { m_player->getComponent<CInput>().left = true; }
        else if (action.name() == "MOVE_RIGHT")       { m_player->getComponent<CInput>().right = true; }
        else if (action.name() == "MOVE_UP")          { m_player->getComponent<CInput>().up = true; }
        else if (action.name() == "SHOOT")            { m_player->getComponent<CInput>().shoot = true; }
        else if (action.name() == "LEFT_CLICK") {
            auto mp = action.pos();

            for (auto e : m_entityManager.getEntities()) {
                if (e->hasComponent<CDraggable>() && isInside(mp, e)) {
                    e->getComponent<CDraggable>().dragging = true;
                }
            }
        } else if (action.name() == "MOUSE_MOVE") {
            m_mousePos = action.pos();
        } else if (action.name() == "WINDOW_RESIZED") {
            auto delta = action.pos();

            for (auto e : m_entityManager.getEntities()) {
                e->getComponent<CTransform>().pos.y += delta.y;
            }
        }

    } else if (action.type() == "END") {
             if (action.name() == "MOVE_LEFT")        { m_player->getComponent<CInput>().left = false; }
        else if (action.name() == "MOVE_RIGHT")       { m_player->getComponent<CInput>().right = false; }
        else if (action.name() == "MOVE_UP")          { m_player->getComponent<CInput>().up = false; }
        else if (action.name() == "SHOOT")            {
            m_player->getComponent<CInput>().shoot = false;
            m_player->getComponent<CInput>().canShoot = true;
        } else if (action.name() == "LEFT_CLICK") {
            for (auto e : m_entityManager.getEntities()) {
                if (e->hasComponent<CDraggable>() && e->getComponent<CDraggable>().dragging) {
                    e->getComponent<CDraggable>().dragging = false;
                }
            }
        }
    }
}

void Scene_Play::sAnimation() {
    // TODO: Complete the Animation class code first
    // TODO: set the animation of the player based on its CState component
    // TODO: for each entity with an animation, call entity->getComponent<CAnimation>().animation.update()
    //       if the animation is not repeated, and it has ended, destroy the entity

    std::string oldAnimation = "";
    std::string newAnimation = "";

    if (m_player->hasComponent<CAnimation>()) {
        oldAnimation = m_player->getComponent<CAnimation>().animation.getName();
    }

    if (m_player->getComponent<CState>().state == "air") {
        newAnimation = "Air";
    }

    if (m_player->getComponent<CState>().state == "run") {
        newAnimation = "Run";
    }

    if (m_player->getComponent<CState>().state == "stand") {
        newAnimation = "Stand";
    }

    if (oldAnimation != newAnimation) {
        m_player->addComponent<CAnimation>(m_game->assets().getAnimation(newAnimation), true);
    }

    for (auto e : m_entityManager.getEntities()) {
        if (e->hasComponent<CAnimation>()) {
            auto& a = e->getComponent<CAnimation>();
            if (!a.repeat && a.animation.hasEnded()) {
                e->destroy();
            } else {
                a.animation.update();
            }
        }
    }
}

void Scene_Play::onEnd() {
    m_game->changeScene("MENU", std::make_shared<Scene_Menu>(m_game));
}

void Scene_Play::sRender() {
    // color the background darker so you know that the game is paused
    if (!m_paused) {
        m_game->window().clear(sf::Color(100, 100, 255));
    } else {
        m_game->window().clear(sf::Color(50, 50, 150));
    }

    // set the viewport of the window to be centered on the player if it's far enough right
    if (!m_player->hasComponent<CDraggable>() || !m_player->getComponent<CDraggable>().dragging) {
        auto &pPos = m_player->getComponent<CTransform>().pos;
        float windowCenterX = std::max(m_game->window().getSize().x / 2.0f, pPos.x);
        sf::View view = m_game->window().getView();
        view.setCenter({windowCenterX, m_game->window().getSize().y - view.getCenter().y});
        m_game->window().setView(view);
    }

    // draw all Entity textures / animations
    if (m_drawTextures) {
        for (auto e : m_entityManager.getEntities()) {
            auto & transform = e->getComponent<CTransform>();

            if (e->hasComponent<CAnimation>()) {
                auto & animation = e->getComponent<CAnimation>().animation;
                animation.getSprite().setRotation(sf::degrees(transform.angle));
                animation.getSprite().setPosition({transform.pos.x, transform.pos.y});
                animation.getSprite().setScale({transform.scale.x, transform.scale.y});
                m_game->window().draw(animation.getSprite());
            }
        }
    }

    // draw all Entity collision bounding boxes with a rectangle shape
    if (m_drawCollision) {
        for (auto e : m_entityManager.getEntities()) {
            if (e->hasComponent<CBoundingBox>()) {
                auto & box = e->getComponent<CBoundingBox>();
                auto & transform = e->getComponent<CTransform>();
                sf::RectangleShape rect;
                rect.setSize(sf::Vector2f(box.size.x-1, box.size.y-1));
                rect.setOrigin(sf::Vector2f(box.halfSize.x, box.halfSize.y));
                rect.setPosition({transform.pos.x, transform.pos.y});
                rect.setFillColor(sf::Color(0, 0, 0, 0));
                rect.setOutlineColor(sf::Color(255, 255, 255, 255));
                rect.setOutlineThickness(1);
                m_game->window().draw(rect);
            }
        }
    }

    // draw the grid so that students can easily debug
    if (m_drawGrid) {
        float leftX = m_game->window().getView().getCenter().x - width() / 2;
        float rightX = leftX + width() + m_gridSize.x;
        float nextGridX = leftX - ((int)leftX % (int)m_gridSize.x);

        for (float x = nextGridX; x < rightX; x += m_gridSize.x) {
            drawLine(Vec2(x, 0), Vec2(x, height()));
        }

        for (float y = 0; y < height(); y += m_gridSize.y) {
            drawLine(Vec2(leftX, height()-y), Vec2(rightX, height()-y));

            for (float x = nextGridX; x < rightX; x += m_gridSize.x) {
                std::string xCell = std::to_string((int)x / (int)m_gridSize.x);
                std::string yCell = std::to_string((int)y / (int)m_gridSize.y);
                m_gridText.setString("(" + xCell + "," + yCell + ")");
                m_gridText.setPosition({x + 3, height() - y - m_gridSize.y + 2});
                m_game->window().draw(m_gridText);
            }
        }
    }

    m_playerState.setPosition({10 + m_game->window().getView().getCenter().x - m_game->window().getSize().x / 2, 10});
    m_playerState.setString("STATE: " + m_player->getComponent<CState>().state);
    m_game->window().draw(m_playerState);
}

void Scene_Play::setPaused(bool v) {
    m_paused = v;
};

void Scene_Play::reload() {
    m_game->changeScene("PLAY", std::make_shared<Scene_Play>(m_game, m_levelPath), true);
}

void Scene_Play::sDragAndDrop() {
    for (auto e : m_entityManager.getEntities()) {
        if (e->hasComponent<CDraggable>() && e->getComponent<CDraggable>().dragging) {
            auto& t = e->getComponent<CTransform>();
            t.velocity = Vec2(0, 0);
            t.pos = m_mousePos;
        }
    }
}

bool Scene_Play::isInside(const Vec2 & pos, std::shared_ptr<Entity> e) const {
    auto ep = e->getComponent<CTransform>().pos;
    auto es = e->getComponent<CAnimation>().animation.getSize() / 2;
    auto dist = Vec2(std::fabs(pos.x-ep.x), std::fabs(pos.y-ep.y));
    return dist.x <= es.x && dist.y <= es.y;
}
