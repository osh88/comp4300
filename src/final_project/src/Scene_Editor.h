#pragma once

#include "Scene.h"
#include "imgui.h"

class Scene_Editor : public Scene {
    struct PlayerConfig {
        float X, Y, CW, CH, SPEED, MAXSPEED, JUMP, GRAVITY, HEALTH;
        std::string WEAPON;
    };

    // Drag-and-Drop System
    struct SDnD {
        Vec2                  startEntPos = {0, 0};
        Vec2                  mousePos = {0,0};
        std::optional<Entity> entity;

        Vec2                  startMousePos = {0,0};
        Vec2                  startViewPos = {0,0};
        bool                  moveCanvas = false;
    };

protected:
    PlayerConfig          m_playerConfig;
    bool                  m_drawGrid = true;
    bool                  m_drawBoundingBox = false;
    bool                  m_gridAbove = true;
    Entity                m_player;
    Vec2                  m_gridSize = {64, 64};
    sf::Text              m_gridText;
    Vec2                  m_leftBottomOfLevel = {0, 0};
    std::string           m_levelPath = "";
    int                   m_pathIndex = -1;
    std::optional<Entity> m_selectedEntity;
    std::optional<Entity> m_entityForCopy;
    SDnD                  m_sDnD;
    std::vector<const char*> m_paths = std::vector<const char*>(100);
    std::vector<Animation> m_animations;

    void update();
    void onResume(const std::string & prevScene);
    void onPause(const std::string & nextScene);
    void sDoAction(const Action & action);
    void sDragAndDrop();
    void sGUI();
    
    Vec2 newSpritePos();
    void spawnPlayer();
    void spawnDec();
    void spawnTile();
    void spawnMTile();
    void spawnNPC(const std::string & ai);
    void saveLevel();
    void loadLevel(const std::string & path);
    std::optional<std::string> selectFile();
    std::optional<Animation> selectAnimation(const Animation& animation);
    bool CollapsingHeaderWithColor(const std::string & header, ImVec4 color);
    int selectTeleportLevel(int level);
    bool InputFloat2(const std::string & name, Vec2& p, float step = 0.0f, float fastStep = 0.0f, const std::string & format = "%.0f", ImGuiInputTextFlags flags = 0);
    void afterSpawn(const Entity & e, const Vec2 & pos);
    void drawGrid();
    Entity copyEntity(const Entity & o);
    std::optional<std::string> selectArtefact(std::string & name);
    std::vector<std::string> selectArtefacts(std::vector<std::string> names);

public:

    Scene_Editor(GameEngine * gameEngine = nullptr);
    void init();
    void sRender();
};
