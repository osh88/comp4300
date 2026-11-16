#pragma once

#include "Scene.h"

class Scene_Menu final : public Scene {
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

    void update() override;
    void onPause(const std::string & nextScene) override;
    void onResume(const std::string & prevScene) override;
    void sDoAction(const Action & action) override;

public:

    Scene_Menu(GameEngine * gameEngine = nullptr, bool debugMode = false);
    virtual ~Scene_Menu() = default;

    void init() override;
    void sRender() override;
    void reload() override;
};
