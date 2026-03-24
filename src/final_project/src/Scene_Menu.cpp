#include "Scene_Menu.h"
#include "Scene_Play.h"
#include "Scene_World.h"
#include "Scene_Editor.h"
#include "Scene_Settings.h"
#include "SFML/Graphics/Text.hpp"
#include "Helpers.h"

#include <iostream>
using H = Helpers;

const sf::String SETTINGS_TITLE = H::S("Настройки");
const sf::String WORLD_TITLE    = H::S("Мир");
const sf::String EDITOR_TITLE   = H::S("Редактор");

Scene_Menu::Scene_Menu(GameEngine* gameEngine, bool debugMode)
    : Scene(gameEngine)
    , m_menuText(m_game->assets().getFont("Mario").font, L"Не Марио", 30)
    , m_menuHelp(m_game->assets().getFont("Mario").font, "UP/DOWN ENTER:PLAY ESC:BACK/QUIT", 26)
    , m_bg(m_bgTexture)
    , m_debugMode(debugMode)
{}

void Scene_Menu::init() {
    storeViewCenter(m_game->getWindowSize() / 2.0);

    m_game->assets().getSound("MusicTitle")->setLooping(true);
    m_game->assets().getSound("MusicTitle")->play();

    registerAction("UP",    sf::Keyboard::Key::Up);
    registerAction("DOWN",  sf::Keyboard::Key::Down);
    registerAction("ENTER", sf::Keyboard::Key::Enter);
    registerAction("QUIT",  sf::Keyboard::Key::Escape);
    registerAction("TOGGLE_DEBUG", sf::Keyboard::Key::D, true);

    if (!m_bgTexture.resize(m_game->window().getSize())) {
        std::cout << "can't resize bg texture" << std::endl;
        std::exit(-1);
    }
    m_bg.setPosition({0,0});
    m_bg.setTextureRect(sf::IntRect({0, 0}, Vec2(m_bgTexture.getSize())));

    m_menuText.setFillColor(textColor);
    auto bb = m_menuText.getGlobalBounds().size;
    m_menuText.setOrigin(sf::Vector2f(bb.x / 2.0, bb.y / 2.0));
    m_menuText.setPosition({ (float)m_game->window().getSize().x / 2, 90 });

    if (m_debugMode) {
        for (auto v : m_game->getSettings().levels) {
            m_menuStrings.push_back(v.path);
        }
    }

    m_menuStrings.push_back(WORLD_TITLE);
    m_menuStrings.push_back(SETTINGS_TITLE);
    m_menuStrings.push_back(EDITOR_TITLE);

    for (size_t i=0; i<m_menuStrings.size(); i++) {
        sf::Text text(m_game->assets().getFont("Mario").font, m_menuStrings[i], 26);

        if (i != m_selectedMenuIndex) {
            text.setFillColor(textColor);
        } else {
            text.setFillColor(sf::Color::White);
        }

        auto bb = text.getGlobalBounds().size;
        text.setOrigin(sf::Vector2f(bb.x / 2.0, bb.y / 2.0));
        text.setPosition({
            (float)m_game->window().getSize().x / 2,
            (float)(m_menuText.getGlobalBounds().position.y + m_menuText.getGlobalBounds().size.y + 10 + (bb.y + 10) * (i + 1))
        });

        m_menuItems.push_back(text);
    }

    m_menuHelp.setFillColor(textColor);
    bb = m_menuHelp.getGlobalBounds().size;
    m_menuHelp.setOrigin(sf::Vector2f(bb.x / 2.0, bb.y / 2.0));
    m_menuHelp.setPosition({
        (float)m_game->window().getSize().x / 2,
        (float)(m_game->window().getSize().y - bb.y * 2)
    });
}

void Scene_Menu::update() {
    sRender();
    m_currentFrame++;
}

void Scene_Menu::sDoAction(const Action& action) {
    if (action.type() == "START") {
        if (action.name() == "TOGGLE_DEBUG") {
            m_debugMode = !m_debugMode;
            reload();
        } else if (action.name() == "UP") {
            if (m_selectedMenuIndex > 0) {
                m_selectedMenuIndex--;
            }
            else {
                m_selectedMenuIndex = m_menuStrings.size() - 1;
            }
        } else if (action.name() == "DOWN") {
            m_selectedMenuIndex = (m_selectedMenuIndex + 1) % m_menuStrings.size();
        } else if (action.name() == "ENTER") {
            if (m_menuStrings[m_selectedMenuIndex] == WORLD_TITLE) {
                if (auto lvl = m_game->getSettings().getFirstWorldLevel(); lvl.has_value()) {
                    m_game->deleteScene("EDITOR");
                    m_game->changeScene("PLAY", std::make_shared<Scene_World>(m_game, lvl->path));
                    return;
                }
            }

            if (m_menuStrings[m_selectedMenuIndex] == SETTINGS_TITLE) {
                m_game->changeScene("SETTINGS", std::make_shared<Scene_Settings>(m_game));
                return;
            }

            if (m_menuStrings[m_selectedMenuIndex] == EDITOR_TITLE) {
                m_game->deleteScene("PLAY");
                m_game->changeScene("EDITOR", std::make_shared<Scene_Editor>(m_game));
                return;
            }

            auto& level = m_game->getSettings().levels[m_selectedMenuIndex];
            
            if (auto s = m_game->getScene("PLAY"); s != nullptr) {
                if (s->levelPath() != level.path) {
                    m_game->deleteScene("PLAY");
                }
            }

            m_game->deleteScene("EDITOR");

            if (level.world) {
                m_game->changeScene("PLAY", std::make_shared<Scene_World>(m_game, level.path));
            } else {
                m_game->changeScene("PLAY", std::make_shared<Scene_Play>(m_game, level));
            }
        } else if (action.name() == "QUIT") {
            m_game->quit();
        } else if (action.name() == "WINDOW_RESIZED") {
            storeViewCenter(m_game->getWindowSize() / 2.0);

            auto delta = action.pos() / 2;

            auto wh = m_game->getWindowSize();

            m_menuText.setPosition(Vec2(m_menuText.getPosition()) + Vec2(delta.x, 0));

            for (auto& item : m_menuItems) {
                item.setPosition(Vec2(item.getPosition()) + Vec2(delta.x, 0));
            }

            m_menuHelp.setPosition({ m_menuHelp.getPosition().x + delta.x, (float)(wh.y - m_menuHelp.getGlobalBounds().size.y * 2) });

            if (!m_bgTexture.resize(wh)) {
                std::cout << "can't resize bg texture" << std::endl;
                std::exit(-1);
            }
            m_bg.setTextureRect(sf::IntRect({0, 0}, Vec2(m_bgTexture.getSize())));
        }
    }
}

void Scene_Menu::sRender() {
    // set menu background
    m_game->window().clear(sf::Color(100, 100, 255));
    m_game->getVertexArrays().clearAll();

    auto shader = m_game->assets().getShader("Pices");
    shader->setUniform("u_resolution", sf::Vector2f(m_game->window().getSize().x, m_game->window().getSize().y));
    shader->setUniform("u_time", (float)m_currentFrame/60);
    m_game->getVertexArrays().draw(m_bg, "Pices", nullptr, shader);
    
    m_game->getVertexArrays().drawAll(m_game->window());

    // draw title
    m_game->window().draw(m_menuText);

    // draw menu items
    for (size_t i=0; i<m_menuStrings.size(); i++) {
        m_menuItems[i].setFillColor(i == m_selectedMenuIndex ? sf::Color::White : textColor);
        m_game->window().draw(m_menuItems[i]);
    }

    m_game->window().draw(m_menuHelp);
}

void Scene_Menu::onPause(const std::string & nextScene) {
    if (nextScene == "PLAY") {
        m_game->assets().getSound("MusicTitle")->pause();
    }
}

void Scene_Menu::onResume(const std::string & prevScene) {
    m_game->assets().getSound("MusicTitle")->play();
}

void Scene_Menu::reload() {
    m_game->changeScene("MENU", std::make_shared<Scene_Menu>(m_game, m_debugMode), true);
}