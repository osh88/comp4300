#pragma once

#include "Scene.h"
#include <map>
#include <memory>

#include "EntityManager.h"

#include "json.hpp"
using json = nlohmann::json;

typedef std::vector<std::vector<Entity>> TilesMatrix;

class Scene_World : public Scene {
    struct PlayerConfig {
        Vec2  startPos    = {96, 97};
        Vec2  scale       = {1, 1};
        Vec2  boundingBox = {48, 48};
        float damage      = 1;
        float health      = 10;
        float gravity     = 0;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(PlayerConfig, startPos, scale, boundingBox, damage, health, gravity)
    };

    struct STeleport {
        enum State {increaseAlpha, delay};
        State                   state = increaseAlpha;
        int                     level = -1;
        int                     counter = 0;
        std::shared_ptr<Entity> curtain;
    };

protected:

    Entity       m_player;
    PlayerConfig m_playerConfig;
    bool         m_drawTextures = true;
    bool         m_drawDebug = false;
    bool         m_drawGrid = false;
    const Vec2   m_gridSize = {64, 64};
    std::string  m_levelPath = "";
    sf::Text     m_gridText;
    bool         m_follow = true;
    sf::Text     m_frameTime;
    STeleport    m_sTeleport;
    
    TilesMatrix  m_tiles;
    Vec2         m_shift = {0, 0};

    void init();

    void spawnPlayer();
    void spawnSword(Entity entity);
    void moveSword(Entity entity);
    void sMovement();
    void sDoAction(const Action & action);
    void sAI();
    void sStatus();
    void sCollision();
    void sAnimation();
    void sCamera();
    void sRender();
    void sWin();
    void sTeleport();

    void setPaused(bool v); 
    void setAnimation(Entity e, const std::string & animName, bool repeat, float scale);
    void setBoundingBox(Entity e, Vec2 bb, bool blockMove, bool blockVision);
    Vec2 getRoomByPos(const Vec2 & pos);
    // bool roomHasEntities(const Vec2 & room);
    bool intersect(Vec2 ap, Vec2 as, Vec2 bp, Vec2 bs);
    Vec2 getTileIndex(Vec2 pos);
    EntityVec getTilesAround(Vec2 tileIndex);
    EntityVec getViewTiles(Vec2 tl, Vec2 br);

    sf::Font f;
public:

    Scene_World();
    Scene_World(GameEngine * game, const std::string & levelPath);
    void update();
    void reload();
    void onEnd();
    void onPause(const std::string & nextScene);
    void onResume(const std::string & prevScene);

    void saveState();
    bool loadState();
    const std::string & levelPath() const;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Scene_World, m_player, m_playerConfig, m_follow, m_tiles, m_shift, m_entityManager)
};
