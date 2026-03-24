#pragma once

#include "Scene.h"
#include <map>
#include <memory>
#include <deque>

#include "EntityManager.h"

class Scene_Win : public Scene {
protected:

    sf::Text                 m_menuText;
    sf::Text                 m_menuHelp;

    void init();
    void update();
    void onEnd();
    void sDoAction(const Action & action);

public:

    Scene_Win(GameEngine * gameEngine = nullptr);
    void sRender();
    void reload();
};