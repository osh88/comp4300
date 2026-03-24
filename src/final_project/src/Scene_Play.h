#pragma once

#include "Scene.h"
#include <map>
#include <memory>

#include "EntityManager.h"

typedef std::vector<std::vector<Entity>> TilesMatrix;

class Scene_Play : public Scene {
    struct SWin {
        enum State {increaseAlpha, hideShader, delay};
        State                   state = increaseAlpha;
        bool                    win = false;
        int                     counter = 0;
        std::shared_ptr<Entity> curtain;
    };

    struct SParallax {
        float startViewPosX;
        float startPlayerPosY;
    };

    struct STeleport {
        enum State {increaseAlpha, delay};
        State                   state = increaseAlpha;
        int                     level = -1;
        int                     counter = 0;
        std::shared_ptr<Entity> curtain;
    };

protected:

    std::string  m_weapon = "Arrow";
    Entity       m_player;
    bool         m_drawTextures = true;
    bool         m_drawGrid = false;
    const Vec2   m_gridSize = {64, 64};
    Settings::Level& m_level;
    sf::Text     m_frameTime;
    sf::Text     m_gridText;
    bool         m_drawDebug = false;
    sf::Text     m_playerState;
    Vec2         m_mousePos;
    bool         m_slow = false;
    
    Vec2         m_leftBottomOfLevel;

    SWin         m_sWin;
    SParallax    m_sParallax;
    STeleport    m_sTeleport;

    void spawnPlayer();
    void spawnWeapon();
    void moveSword(Entity entity);
    void spawnArtefact(Entity block, const std::string & artName);

    void sMovement();
    void sDoAction(const Action & action);
    void sAI();
    void sStatus();
    void sCollision();
    void sAnimation();
    void sCamera();
    void sRender();
    void sVelocity();
    void sDragAndDrop();
    void sParallax();
    void sWin();
    void sTeleport();
    void sInventory();

    void setPaused(bool);
    void setAnimation(Entity e, const std::string & animName, bool repeat, const Vec2 & scale);
    void setBoundingBox(Entity e, Vec2 bb, bool blockMove, bool blockVision);

    void setFPS();
    void onEnd();

public:

    Scene_Play(GameEngine * gameEngine, Settings::Level& level);
    void init();
    void update();
    void reload();
    const std::string & levelPath() const;
};