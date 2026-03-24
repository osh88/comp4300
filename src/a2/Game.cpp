#include "Game.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <math.h>

Game::Game(const std::string& config) {
    init(config);
}

void Game::init(const std::string& path) {
    std::string token;
    std::ifstream fin(path);

    while (fin >> token) {
        if (token == "Window") {
            int width, height, frameLimit, fullScreen;
            fin >> width >> height >> frameLimit >> fullScreen;

            auto state = (fullScreen >= 1) ? sf::State::Fullscreen : sf::State::Windowed;

            m_window.create(sf::VideoMode(sf::Vector2u(width, height)), "Assignment 2", sf::Style::Default, state);
            m_window.setFramerateLimit(frameLimit);
        } else if (token == "Font") {
            std::string path;
            int size, r, g, b;
            fin >> path >> size >> r >> g >> b;

            if (!m_font.openFromFile(path)) {
                std::cerr << "Could not load font!" << std::endl;
                exit(-1);
            }

            m_text = std::make_shared<sf::Text>(m_font, "", size);
            m_text->setFillColor(sf::Color(r, g, b));
            m_text->setPosition({10, 10});

            m_sp = std::make_shared<sf::Text>(m_font, "", size);
            m_sp->setFillColor(sf::Color(r, g, b));
            m_sp->setPosition({10, 40});

            m_fps = std::make_shared<sf::Text>(m_font, "", size);
            m_fps->setFillColor(sf::Color(r, g, b));
            m_fps->setPosition({10, 70});
        } else if (token == "Player") {
            fin >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S
            >> m_playerConfig.FR >> m_playerConfig.FG >> m_playerConfig.FB
            >> m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB
            >> m_playerConfig.OT >> m_playerConfig.V;
        } else if (token == "Enemy") {
            fin >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX
            >> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB
            >> m_enemyConfig.OT >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX
            >> m_enemyConfig.L >> m_enemyConfig.SI;
        } else if (token == "Bullet") {
            fin >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S
            >> m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB
            >> m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB
            >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;
        }
    }

    setScore(0);
    spawnPlayer();
}

void Game::run() {
    // TODO add pause functionality in here
    //      some systems should function while paused (rendering)
    //      some systems shouldn't (movement / input)
    while(m_running) {
        auto start = std::chrono::high_resolution_clock::now();

        m_entities.update();

        if (!m_paused) {
            sEnemySpawner();
            sMovement();
            sLifespan();
            sCollision();
            sSuperPower();
        }

        sUserInput();

        sRender();

        // increment the current frame
        // may need to be moved when pause implemented
        m_currentFrame++;

        // Record the end time
        auto end = std::chrono::high_resolution_clock::now();

        // Calculate the duration
        std::chrono::duration<double, std::milli> elapsed = end - start;

        m_fps->setString("FPS: " + std::to_string(elapsed.count()) + " ms");
    }
}

void Game::setPaused(bool paused) {
    m_paused = paused;
}

// respawn the player in the middle of the screen
//Player SR CR S FR FG FB OR OG OB OT V
//  Shape Radius      SR       int
//  Collision Radius  CR       int
//  Speed             S        float
//  Fill Color        FR,FG,FB int,int,int
//  Outline Color     OR,OG,OB int,int,int
//  Outline Thickness OT       int
//  Shape Vertices    V        int
void Game::spawnPlayer() {
    // TODO: Finish adding all properties of the player with the correct values from the config

    auto entity = m_entities.addEntity("player");

    float mx = m_window.getSize().x / 2.0f;
    float my = m_window.getSize().y / 2.0f;

    entity->cTransform = std::make_shared<CTransform>(Vec2(mx, my), Vec2(0.0f, 0.0f), 0.0f);

    entity->cShape = std::make_shared<CShape>(
        m_playerConfig.SR, m_playerConfig.V,
        sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB),
        sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB),
        m_playerConfig.OT
    );

    // Add an input component to the player so that we can use inputs
    entity->cInput = std::make_shared<CInput>();

    entity->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);

    entity->cSuperPower = std::make_shared<CSuperPower>(20);

    m_player = entity;

    m_sp->setString("SP: " + std::to_string(m_player->cSuperPower->remaining));
}

// spawn an enemy at a random position
//Enemy SR CR SMIN SMAX OR OG OB OT VMIN VMAX L SI
//  Shape Radius      SR        int
//  Collision Radius  CR        int
//  Min / Max Speed   SMIN,SMAX float, float
//  Outline Color     OR,OG,OB  int, int, int
//  Outline Thickness OT        int
//  Min/Max Vertices  VMIN,VMAX int, int
//  Small Lifespan    L         int
//  Spawn Interval    SP        int
void Game::spawnEnemy() {
    // TODO: make sure the enemy is spawned properly with the m_enemyConfig variables
    //       the enemy must be spawned completely within the bounds of the window

    auto entity = m_entities.addEntity("enemy");

    float ex = m_enemyConfig.SR + (rand() % (m_window.getSize().x-2*m_enemyConfig.SR));
    float ey = m_enemyConfig.SR + (rand() % (m_window.getSize().y-2*m_enemyConfig.SR));

    auto angle = rand() % 360;
    auto scalarSpeed = m_enemyConfig.SMIN;
    if ((int)m_enemyConfig.SMIN != (int)m_enemyConfig.SMAX) {
        scalarSpeed = m_enemyConfig.SMIN + (rand() % (int) (m_enemyConfig.SMAX - m_enemyConfig.SMIN));
    }
    auto speed = Vec2(sinf(angle), cosf(angle))*scalarSpeed;
    auto v = m_enemyConfig.VMIN + (rand() % (m_enemyConfig.VMAX-m_enemyConfig.VMIN));

    entity->cTransform = std::make_shared<CTransform>(Vec2(ex, ey), speed, angle);

    entity->cShape = std::make_shared<CShape>(
            m_enemyConfig.SR, v,
            sf::Color(rand() % 255, rand() % 255, rand() % 255),
            sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB),
            m_playerConfig.OT
    );

    entity->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR);

    entity->cScore = std::make_shared<CScore>(v*100);

    entity->cAge = std::make_shared<CAge>(3);

    // record when the most recent enemy was spawned
    m_lastEnemySpawnTime = m_currentFrame;
}

// spawns the small enemies when a big one (input entity e) explodes
void Game::spawnSmallEnemies(std::shared_ptr<Entity> e) {
    // TODO: spawn small enemies at the location of the input enemy e
    //       when we create the smaller enemy, we have to read the values of the original enemy
    //       - spawn a number of small enemies equal to the vertices of the original enemy
    //       - set each small enemy to the same color as th original, half the size
    //       - small enemies are worth double points of the original enemy

    auto enemyCount = e->cShape->circle.getPointCount();
    auto enemyPos = e->cTransform->pos;
    auto enemyRadius = e->cShape->circle.getRadius();
    auto angle = 0;
    auto angleStep = 360 / enemyCount;
    auto scalarSpeed = e->cTransform->velocity.length();

    for (size_t i=0; i<enemyCount; i++, angle += angleStep) {
        auto entity = m_entities.addEntity("enemy");

        auto radians = angle * (M_PI / 180.0);
        auto normSpeed = Vec2(sinf(radians), cosf(radians));
        auto speed = normSpeed * scalarSpeed;

        //auto pos = enemyPos + normSpeed*enemyRadius;
        entity->cTransform = std::make_shared<CTransform>(enemyPos, speed, angle);

        entity->cShape = std::make_shared<CShape>(
                enemyRadius/2, enemyCount,
                e->cShape->circle.getFillColor(),
                e->cShape->circle.getOutlineColor(),
                e->cShape->circle.getOutlineThickness()/2
        );

        entity->cCollision = std::make_shared<CCollision>(e->cCollision->radius/2);

        entity->cScore = std::make_shared<CScore>(e->cScore->score*2);

        entity->cLifespan = std::make_shared<CLifespan>(m_enemyConfig.L);

        entity->cAge = std::make_shared<CAge>(e->cAge->age-1);
    }
}

// spawns a bullet from a given entity to a target location
//Bullet SR CR S FR FG FB OR OG OB OT V L
//  Shape Radius      SR       int
//  Collision Radius  CR       int
//  Speed             S        float
//  Fill Color        FR,FG,FB int,int,int
//  Outline Color     OR,OG,OB int,int,int
//  Outline Thickness OT       int
//  Shape Vertices    V        int
//  Lifespan          L        int
void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2 &mousePos) {
    // TODO: implement the spawning of a bullet which travels toward target
    // - bullet speed is given as a scalar speed
    // you must set the velocity by using formula in notes

    auto entPos = Vec2(entity->cShape->circle.getPosition().x, entity->cShape->circle.getPosition().y);
    auto target = mousePos-entPos;
    auto norm   = target / target.length();
    auto speed  = norm * m_bulletConfig.S;

    auto bullet = m_entities.addEntity("bullet");

    auto pos = entPos + norm*entity->cShape->circle.getRadius();
    bullet->cTransform = std::make_shared<CTransform>(pos, speed, 0.0f);

    bullet->cShape = std::make_shared<CShape>(
            m_bulletConfig.SR, m_bulletConfig.V,
            sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB),
            sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB),
            m_bulletConfig.OT
    );

    bullet->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L);

    bullet->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> e) {
    if (e->cSuperPower->remaining == 0) {
        return;
    }

    auto bulletCount = e->cSuperPower->remaining;
    e->cSuperPower->remaining = 0;
    auto playerPos = e->cTransform->pos;
    auto playerRadius = e->cShape->circle.getRadius();
    auto angle = 0;
    auto angleStep = 360 / bulletCount;
    auto scalarSpeed = m_bulletConfig.S;

    for (size_t i=0; i<bulletCount; i++, angle += angleStep) {
        auto entity = m_entities.addEntity("bullet");

        auto radians = angle * (M_PI / 180.0);
        auto normSpeed = Vec2(sinf(radians), cosf(radians));
        auto speed = normSpeed * scalarSpeed;

        auto pos = playerPos + normSpeed*playerRadius;
        entity->cTransform = std::make_shared<CTransform>(pos, speed, angle);

        entity->cShape = std::make_shared<CShape>(
                playerRadius / 2, bulletCount,
                e->cShape->circle.getFillColor(),
                e->cShape->circle.getOutlineColor(),
                e->cShape->circle.getOutlineThickness() / 2
        );

        entity->cCollision = std::make_shared<CCollision>(e->cCollision->radius / 2);

        entity->cLifespan = std::make_shared<CLifespan>(m_enemyConfig.L);
    }
}

void Game::sMovement() {
    // TODO: implement all entity movement in this function
    //       you should read the m_player->cInput component to determine if the player is moving

    // PLAYER

    m_player->cTransform->velocity = {0 ,0};

    if (m_player->cInput->up) {
        m_player->cTransform->velocity.y = -m_playerConfig.S;
    }

    if (m_player->cInput->down) {
        m_player->cTransform->velocity.y = m_playerConfig.S;
    }

    if (m_player->cInput->left) {
        m_player->cTransform->velocity.x = -m_playerConfig.S;
    }

    if (m_player->cInput->right) {
        m_player->cTransform->velocity.x = m_playerConfig.S;
    }

    auto x = m_player->cTransform->pos.x + m_player->cTransform->velocity.x;
    if (x > m_playerConfig.SR && x < m_window.getSize().x-m_playerConfig.SR) {
        m_player->cTransform->pos.x += m_player->cTransform->velocity.x;
    }

    auto y = m_player->cTransform->pos.y + m_player->cTransform->velocity.y;
    if (y > m_playerConfig.SR && y < m_window.getSize().y-m_playerConfig.SR) {
        m_player->cTransform->pos.y += m_player->cTransform->velocity.y;
    }

    // ENEMY

    for (auto &e : m_entities.getEntities("enemy")) {
        if (e->cTransform->pos.x + e->cTransform->velocity.x < m_enemyConfig.SR) {
            e->cTransform->velocity.x = -e->cTransform->velocity.x;
        }

        if (e->cTransform->pos.x + e->cTransform->velocity.x > m_window.getSize().x-m_enemyConfig.SR) {
            e->cTransform->velocity.x = -e->cTransform->velocity.x;
        }

        if (e->cTransform->pos.y + e->cTransform->velocity.y < m_enemyConfig.SR) {
            e->cTransform->velocity.y = -e->cTransform->velocity.y;
        }

        if (e->cTransform->pos.y + e->cTransform->velocity.y > m_window.getSize().y-m_enemyConfig.SR) {
            e->cTransform->velocity.y = -e->cTransform->velocity.y;
        }

        e->cTransform->pos.x += e->cTransform->velocity.x;
        e->cTransform->pos.y += e->cTransform->velocity.y;
    }

    // BULLET

    for (auto &e : m_entities.getEntities("bullet")) {
        e->cTransform->pos.x += e->cTransform->velocity.x;
        e->cTransform->pos.y += e->cTransform->velocity.y;
    }
}

void Game::sLifespan() {
    // TODO: implement all lifespan functionality
    //
    // for all entities
    //   if entity has no lifespan component, skip it
    //   if entity has > 0 remaining lifespan, subtract 1
    //   if it has lifespan and is alive
    //     scale its alpha channel properly
    //   if it has lifespan and its time is up
    //     destroy the entity

    for (auto& e : m_entities.getEntities()) {
        if (!e->cLifespan) {
            continue;
        }

        if (e->cLifespan->remaining <= 0) {
            e->destroy();
            continue;
        }

        e->cLifespan->remaining--;

        auto fillColor = e->cShape->circle.getFillColor();
        fillColor.a = 255 * ((float)(e->cLifespan->remaining / (float)e->cLifespan->total));
        e->cShape->circle.setFillColor(fillColor);

        auto outlineColor = e->cShape->circle.getOutlineColor();
        outlineColor.a = 255 * ((float)(e->cLifespan->remaining / (float)e->cLifespan->total));
        e->cShape->circle.setOutlineColor(outlineColor);
    }
}

void Game::sCollision() {
    // TODO: implement all proper collisions between entities
    //       be sure to use the collision radius, NOT the shape radius
    for (auto e : m_entities.getEntities("enemy")) {
        if (m_player->collision(e)) {
            addScore(-e->cScore->score);
            m_player->destroy();
            e->destroy();
            spawnPlayer();
        }
    }

    for (auto b : m_entities.getEntities("bullet")) {
        for (auto e : m_entities.getEntities("enemy")) {
            if (b->collision(e)) {
                addScore(e->cScore->score);
                b->destroy();

                if (e->cAge->age > 0) {
                    spawnSmallEnemies(e);
                }

                e->destroy();
            }
        }
    }
}

void Game::sEnemySpawner() {
    // TODO: code which implements enemy spawning should go here
    //       use (m_currentFrame - m_lastEnemySpawnTime) to determine
    //       how long it has been since the last enemy spawned
    if (m_currentFrame-m_lastEnemySpawnTime > m_enemyConfig.SI) {
        spawnEnemy();
    }
}

void Game::sRender() {
    // TODO: change the code below to draw ALL of the entities
    //       sample drawing of the player Entity that we have created
    m_window.clear();

    m_window.draw(*m_text);
    m_window.draw(*m_sp);
    m_window.draw(*m_fps);

    for (auto &e : m_entities.getEntities()) {
        e->cShape->circle.setPosition(sf::Vector2(e->cTransform->pos.x, e->cTransform->pos.y));

        e->cTransform->angle += 1.0f;
        e->cShape->circle.setRotation(sf::degrees(e->cTransform->angle));

        m_window.draw(e->cShape->circle);
    }

    m_window.display();
}

void Game::sUserInput() {
    // TODO: handle user input here
    //       note that you should only be setting the player's input component variables here
    //       you should not implement the player's movement logic here
    //       the movement system will read the variables you set in this function

    while (const std::optional event = m_window.pollEvent())
    {
        if (event->is<sf::Event::Closed>() || (event->is<sf::Event::KeyPressed>() &&
           event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape)) {
            m_running = false;
        }

        if (event->is<sf::Event::Closed>() || (event->is<sf::Event::KeyPressed>() &&
           event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::P)) {
            setPaused(!m_paused);
        }

        if (m_paused) {
            continue;
        }

        if (event->is<sf::Event::KeyPressed>()) {
            switch (event->getIf<sf::Event::KeyPressed>()->code) {
                case sf::Keyboard::Key::W:
                    m_player->cInput->up = true;
                    break;

                case sf::Keyboard::Key::S:
                    m_player->cInput->down = true;
                    break;

                case sf::Keyboard::Key::A:
                    m_player->cInput->left = true;
                    break;

                case sf::Keyboard::Key::D:
                    m_player->cInput->right = true;
                    break;

                default:
                    break;
            }
        }

        if (event->is<sf::Event::KeyReleased>()) {
            switch (event->getIf<sf::Event::KeyReleased>()->code) {
                case sf::Keyboard::Key::W:
                    m_player->cInput->up = false;
                    break;

                case sf::Keyboard::Key::S:
                    m_player->cInput->down = false;
                    break;

                case sf::Keyboard::Key::A:
                    m_player->cInput->left = false;
                    break;

                case sf::Keyboard::Key::D:
                    m_player->cInput->right = false;
                    break;

                default:
                    break;
            }
        }

        if (event->is<sf::Event::MouseButtonPressed>()) {
            auto e = event->getIf<sf::Event::MouseButtonPressed>();

            if (e->button == sf::Mouse::Button::Left) {
                spawnBullet(m_player, Vec2(e->position.x, e->position.y));
            }

            if (e->button == sf::Mouse::Button::Right) {
                spawnSpecialWeapon(m_player);
            }
        }
    }
}

void Game::setScore(int val) {
    m_score = val;
    m_text->setString("SCORE: " + std::to_string(m_score));
}

void Game::addScore(int val) {
    setScore(m_score+val);
}

void Game::sSuperPower() {
    if (m_currentFrame % 20 == 0 && m_player->cSuperPower->remaining < m_player->cSuperPower->total) {
        m_player->cSuperPower->remaining++;
        m_sp->setString("SP: " + std::to_string(m_player->cSuperPower->remaining));
    }
}
