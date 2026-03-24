#pragma once

#include "Entity.h"
#include "EntityManager.h"

#include <SFML/Graphics.hpp>

struct PlayerConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V; float S; };
struct EnemyConfig  { int SR, CR, OR, OG, OB, OT, VMIN, VMAX, L, SI; float SMIN, SMAX; };
struct BulletConfig { int SR, CR, FR, FG, FB, OR, OG, OB, OT, V, L; float S; };

class Game {
    sf::RenderWindow m_window;
    EntityManager    m_entities;
    sf::Font         m_font;
    std::shared_ptr<sf::Text> m_text; // the score text to be drawn to the screen;
    std::shared_ptr<sf::Text> m_sp;   // super power;
    std::shared_ptr<sf::Text> m_fps;  // fps;
    PlayerConfig     m_playerConfig;
    EnemyConfig      m_enemyConfig;
    BulletConfig     m_bulletConfig;
    int              m_score = 0;
    int              m_currentFrame = 0;
    int              m_lastEnemySpawnTime = 0;
    bool             m_paused = false; // whether we update game logic
    bool             m_running = true; // whether the game is running

    std::shared_ptr<Entity> m_player;
    void init(const std::string& config); // initialize the GameState with a config file path
    void setPaused(bool paused);          // pause the game

    void sMovement();     // System: Entity position / movement update
    void sUserInput();    // System: User input
    void sLifespan();     // System: Lifespan
    void sRender();       // System: Render / Drawing
    void sEnemySpawner(); // System: Spawns enemies
    void sCollision();    // System: Collisions
    void sSuperPower();   // System: Super power

    void spawnPlayer();
    void spawnEnemy();
    void spawnSmallEnemies(std::shared_ptr<Entity> entity);
    void spawnBullet(std::shared_ptr<Entity> entity, const Vec2& mousePos);
    void spawnSpecialWeapon(std::shared_ptr<Entity> entity);
    void setScore(int val);
    void addScore(int val);

public:

    Game(const std::string& config);

    void run();
};