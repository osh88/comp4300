#pragma once

#include "Scene.h"

class Scene_Win final : public Scene {
protected:

    sf::Text                 m_menuText;
    sf::Text                 m_menuHelp;
    sf::Texture              m_bgTexture;
    sf::Sprite               m_bg;

    void update() override;
    void onEnd() override;
    void sDoAction(const Action & action) override;

public:

    Scene_Win(GameEngine * gameEngine = nullptr);
    virtual ~Scene_Win() = default;

    void init() override;
    void sRender() override;
};
