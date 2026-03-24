#pragma once

#include "Scene.h"

class Scene_Win : public Scene {
protected:

    sf::Text                 m_menuText;
    sf::Text                 m_menuHelp;
    sf::Texture              m_bgTexture;
    sf::Sprite               m_bg;

    void update();
    void onEnd();
    void sDoAction(const Action & action);

public:

    Scene_Win(GameEngine * gameEngine = nullptr);
    void init();
    void sRender();
};