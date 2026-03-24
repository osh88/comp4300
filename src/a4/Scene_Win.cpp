#include "Scene_Win.h"
#include "Scene_Menu.h"
#include "SFML/Graphics/Text.hpp"

void Scene_Win::init() {
    registerAction(sf::Keyboard::Key::Escape, "QUIT");

    int titleSize = 30;
    m_menuText.setFillColor(sf::Color::Red);
    m_menuText.setPosition({
        (float)(m_game->window().getSize().x / 2.0 - titleSize * (m_menuText.getString().getSize() + 1) / 2.0),
        (float)(m_game->window().getSize().y / 2.0 - titleSize)
    });

    m_menuHelp.setFillColor(sf::Color::Black);
    m_menuHelp.setPosition({
        (float)(m_game->window().getSize().x / 2.0 - 26 * (m_menuHelp.getString().getSize() + 1) / 2.0),
        (float)(m_game->window().getSize().y - 30 * 2)
    });
}

void Scene_Win::update() {
    sRender();
    m_currentFrame++;
}

void Scene_Win::onEnd() {
    m_game->changeScene("MENU", std::make_shared<Scene_Menu>(m_game));
}

void Scene_Win::sDoAction(const Action& action) {
    if (action.type() == "START") {
        if (action.name() == "QUIT") {
            onEnd();
        } else if (action.name() == "WINDOW_RESIZED") {
            auto delta = action.pos();
            m_menuText.setPosition({ m_menuText.getPosition().x + delta.x / 2, m_menuText.getPosition().y + delta.y / 2 });

            m_menuHelp.setPosition({ m_menuHelp.getPosition().x + delta.x / 2, (float)(m_game->window().getSize().y - 30 * 2) });
        }
    }
}

Scene_Win::Scene_Win(GameEngine* gameEngine)
    : Scene(gameEngine)
    , m_menuText(m_game->assets().getFont("Mario"), "You Win!", 30)
    , m_menuHelp(m_game->assets().getFont("Mario"), "ESC:BACK", 26)
{
    init();
}

void Scene_Win::sRender() {
    // set menu background
    m_game->window().clear(sf::Color::Cyan);
    m_game->window().draw(m_menuText);
    m_game->window().draw(m_menuHelp);
}

void Scene_Win::reload() {}