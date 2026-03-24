#pragma once

#include "Scene.h"

class Scene_Menu : public Scene {
protected:

    std::vector<sf::String>  m_menuStrings;
    std::vector<sf::Text>    m_menuItems;
    sf::Text                 m_menuText;
    sf::Text                 m_menuHelp;
    size_t                   m_selectedMenuIndex = 0;
    sf::Texture              m_bgTexture;
    sf::Sprite               m_bg;
    bool                     m_debugMode = false;

    const sf::Color textColor = sf::Color(120, 120, 120, 255);

    void update();
    void onPause(const std::string & nextScene);
    void onResume(const std::string & prevScene);
    void sDoAction(const Action & action);

public:

    Scene_Menu(GameEngine * gameEngine = nullptr, bool debugMode = false);
    void init();
    void sRender();
    void reload();
};