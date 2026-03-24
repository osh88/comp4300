#pragma once

#include "Assets.h"
#include "SFML/Graphics/RenderWindow.hpp"
#include "Settings.h"
#include "VertexArrays.h"
#include "Inventory.h"

#include <memory>

class Scene;

typedef std::map<std::string, std::shared_ptr<Scene>> SceneMap;

class GameEngine {
    struct changeScene {
        std::string            sceneName;
        std::shared_ptr<Scene> scene;
        bool                   endCurrentScene;
    };

protected:

    VertexArrays     m_vertexArrays;
    sf::RenderWindow m_window;
    Assets           m_assets;
    std::string      m_currentScene;
    SceneMap         m_sceneMap;
    size_t           m_simulationSpeed = 1;
    bool             m_running = true;
    Vec2             m_windowPrevSize;
    float            m_frameTime;
    int              m_fps = 60;
    changeScene      m_nextScene;
    sf::Clock        m_deltaClock;
    Inventory        m_inventory;

    std::string      m_settingsPath;
    Settings         m_settings;
    std::string      m_exeDir;

    void checkNextScene();
    void init(const std::string & settingsPath);
    void update();
    void sUserInput();

public:

    GameEngine(const std::string & path);

    VertexArrays & getVertexArrays();
    void changeScene(const std::string & sceneName, std::shared_ptr<Scene> scene, bool endCurrentScene = false);

    void quit();
    void run();

    std::shared_ptr<Scene> currentScene();
    sf::RenderWindow & window();
    Assets & assets();
    bool isRunning();
    const std::string& currentSceneName() const;
    const int getFrameTime() const;
    void setFPS(int v);
    void setFullScreen(bool init = false);
    void setSoundVolume();
    std::shared_ptr<Scene> getScene(const std::string& name);
    void deleteScene(const std::string& name);
    
    const std::string & getExeDir();
    Settings& getSettings();
    void loadSettings();
    void saveSettings();

    float getPlayerDamageCoeff();
    float getNpcDamageCoeff();
    Inventory & inventory();

    void moveView(const Vec2 & delta);
    Vec2 getViewCenter() const;
    void setViewCenter(const Vec2 & pos);
    Vec2 getViewSize() const;
    void setViewSize(const Vec2 & s);
    Vec2 getWindowSize() const;

    template <SameAsVec2 T>
    Vec2 p2c(const T & v) const {
        return m_window.mapPixelToCoords(Vec2(v.x, v.y));
    };

    template <SameAsVec2 T>
    Vec2 c2p(const T & v) const {
        return m_window.mapCoordsToPixel(Vec2(v.x, v.y));
    };
};