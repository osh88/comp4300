#include "Scene_Menu.h"
#include "Scene_Zelda.h"
#include "SFML/Graphics/Text.hpp"

void Scene_Menu::init() {
    registerAction(sf::Keyboard::Key::Up, "UP");
    registerAction(sf::Keyboard::Key::Down, "DOWN");
    registerAction(sf::Keyboard::Key::Enter, "PLAY");
    registerAction(sf::Keyboard::Key::Escape, "QUIT");

    int titleSize = 30;
    m_menuText.setFillColor(sf::Color::Black);
    m_menuText.setPosition({
        (float)(m_game->window().getSize().x / 2.0 - titleSize * (m_menuText.getString().getSize() + 1) / 2.0),
        (float)(titleSize * 3)
    });
    m_menuStrings.push_back("LEVEL 1");
    m_menuStrings.push_back("LEVEL 2");
    m_menuStrings.push_back("LEVEL 3");
    m_levelPaths.push_back("level1.txt");
    m_levelPaths.push_back("level2.txt");
    m_levelPaths.push_back("level3.txt");

    for (int i=0; i<m_menuStrings.size(); i++) {
        sf::Text text(m_game->assets().getFont("Mario"), m_menuStrings[i], 26);
        if (i != m_selectedMenuIndex) {
            text.setFillColor(sf::Color::Black);
        }
        text.setPosition({
            (float)(m_game->window().getSize().x / 2.0 - 26 * (m_menuStrings[i].length() + 1) / 2.0),
            (float)(m_menuText.getGlobalBounds().position.y + 10 + 30 * (i + 1))
        });
        m_menuItems.push_back(text);
    }

    m_menuHelp.setFillColor(sf::Color::Black);
    m_menuHelp.setPosition({
        (float)(m_game->window().getSize().x / 2.0 - 26 * (m_menuHelp.getString().getSize() + 1) / 2.0),
        (float)(m_game->window().getSize().y - 30 * 2)
    });
}

void Scene_Menu::update() {
    sRender();
    m_currentFrame++;
}

void Scene_Menu::onEnd() {
    m_game->quit();
}

void Scene_Menu::sDoAction(const Action& action) {
    if (action.type() == "START") {
        if (action.name() == "UP") {
            if (m_selectedMenuIndex > 0) {
                m_selectedMenuIndex--;
            }
            else {
                m_selectedMenuIndex = m_menuStrings.size() - 1;
            }
        } else if (action.name() == "DOWN") {
            m_selectedMenuIndex = (m_selectedMenuIndex + 1) % m_menuStrings.size();
        } else if (action.name() == "PLAY") {
            m_game->changeScene("PLAY", std::make_shared<Scene_Zelda>(m_game, m_levelPaths[m_selectedMenuIndex]));
        } else if (action.name() == "QUIT") {
            onEnd();
        } else if (action.name() == "WINDOW_RESIZED") {
            auto delta = action.pos();
            m_menuText.setPosition({ m_menuText.getPosition().x + delta.x / 2, m_menuText.getPosition().y });

            for (auto& item : m_menuItems) {
                item.setPosition({ item.getPosition().x + delta.x / 2, item.getPosition().y });
            }

            m_menuHelp.setPosition({ m_menuHelp.getPosition().x + delta.x / 2, (float)(m_game->window().getSize().y - 30 * 2) });
        }
    }
}

Scene_Menu::Scene_Menu(GameEngine* gameEngine)
    : Scene(gameEngine)
    , m_menuText(m_game->assets().getFont("Mario"), "Not Zelda", 30)
    , m_menuHelp(m_game->assets().getFont("Mario"), "UP/DOWN ENTER:PLAY ESC:BACK/QUIT", 26)
{
    init();
}

void Scene_Menu::sRender() {
    // set menu background
    m_game->window().clear(sf::Color(100, 100, 255));

    // draw title
    m_game->window().draw(m_menuText);

    // draw menu items
    for (int i=0; i<m_menuStrings.size(); i++) {
        m_menuItems[i].setFillColor(i == m_selectedMenuIndex ? sf::Color::White : sf::Color::Black);
        m_game->window().draw(m_menuItems[i]);
    }

    m_game->window().draw(m_menuHelp);
}

void Scene_Menu::reload() {
    m_game->changeScene("MENU", std::make_shared<Scene_Menu>(m_game), true);
}