#pragma once

#include "Scene.h"
#include <map>
#include <memory>

#include "EntityManager.h"

class Scene_Play : public Scene {
    struct PlayerConfig {
        float X, Y, CW, CH, SPEED, MAXSPEED, JUMP, GRAVITY;
        std::string WEAPON;
    };

protected:

    std::shared_ptr<Entity> m_player;
    std::string             m_levelPath;
    PlayerConfig            m_playerConfig;
    bool                    m_drawTextures = true;
    bool                    m_drawCollision = false;
    bool                    m_drawGrid = false;
    const Vec2              m_gridSize = {64, 64};
    sf::Text                m_gridText;
    sf::Text                m_playerState;
    Vec2                    m_mousePos;

    void init(const std::string & levelPath);
    Vec2 gridToMidPixel(float, float, std::shared_ptr<Entity>);
    void loadLevel(const std::string&);
    void spawnPlayer();
    void spawnBullet();
    void spawnCoin(std::shared_ptr<Entity> block);
    void sMovement();
    void sLifespan();
    void sCollision();
    void sAnimation();
    void sRender();
    void sDoAction(const Action&);
    void onEnd();
    void setPaused(bool);
    void sDragAndDrop();
    bool isInside(const Vec2 & pos, std::shared_ptr<Entity> e) const;

    //void changePlayerStateTo(PlayerState s);
    void spawnCoinSpin(std::shared_ptr<Entity> tile);
    void spawnBrickDebris(std::shared_ptr<Entity> tile);

public:

    Scene_Play(GameEngine * gameEngine, const std::string & levelPath);
    void update();
    void reload();
};