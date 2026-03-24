#pragma once

#include "Scene.h"
#include <map>
#include <memory>

#include "EntityManager.h"

typedef std::vector<std::vector<Entity>> TilesMatrix;

class Scene_Zelda : public Scene {
    struct PlayerConfig {
        float X, Y, BX, BY, SPEED, HEALTH;
    };

protected:

    Entity       m_player;
    std::string  m_levelPath;
    PlayerConfig m_playerConfig;
    bool         m_drawTextures = true;
    bool         m_drawDebug = false;
    bool         m_follow = false;
    sf::Text     m_frameTime;
    
    TilesMatrix  m_tiles;
    Vec2         m_shift = {0, 0};

    void init(const std::string & levelPath);
    void loadLevel(const std::string & filename);

    Vec2 getPosition(int rx, int ry, int tx, int ty) const;
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
    void onEnd();
    void sRender();
    void sWin();

    void setPaused(bool v); 
    void setAnimation(Entity e, const std::string & animName, bool repeat, float scale);
    Vec2 getRoomByPos(const Vec2 & pos);
    bool roomHasEntities(const Vec2 & room);
    bool intersect(Vec2 ap, Vec2 as, Vec2 bp, Vec2 bs);
    Vec2 getTileIndex(Vec2 pos);
    EntityVec getTilesAround(Vec2 tileIndex);
    EntityVec getViewTiles(Vec2 tl, Vec2 br);

public:

    Scene_Zelda(GameEngine * game, const std::string & levelPath);
    void update();
    void reload();
};
