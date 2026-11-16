#include "Scene_Win.h"
#include <iostream>

Scene_Win::Scene_Win(GameEngine* gameEngine)
    : Scene(gameEngine)
    , m_menuText(m_game->assets().getFont("Mario").font, "You Win!", 30)
    , m_menuHelp(m_game->assets().getFont("Mario").font, "ESC:BACK", 26)
    , m_bg(m_bgTexture)
{}

void Scene_Win::init() {
    storeViewCenter(m_game->getWindowSize() / 2.0);

    if (!m_bgTexture.resize(m_game->getWindowSize())) {
        std::cout << "can't resize bg texture" << std::endl;
        std::exit(-1);
    }
    m_bg.setPosition({0,0});
    m_bg.setTextureRect(sf::IntRect({0, 0}, Vec2(m_bgTexture.getSize())));

    registerAction("QUIT", sf::Keyboard::Key::Escape);

    m_menuText.setFillColor(sf::Color::Red);
    auto bb = m_menuText.getGlobalBounds().size;
    m_menuText.setOrigin(sf::Vector2f(bb.x / 2.0, bb.y / 2.0));
    m_menuText.setPosition(sf::Vector2f(m_game->window().getSize().x / 2, m_game->window().getSize().y / 2));

    m_menuHelp.setFillColor(sf::Color(120, 120, 120, 255));
    bb = m_menuHelp.getGlobalBounds().size;
    m_menuHelp.setOrigin(sf::Vector2f(bb.x / 2.0, bb.y / 2.0));
    m_menuHelp.setPosition(sf::Vector2f(m_game->window().getSize().x / 2, m_game->window().getSize().y - bb.y * 2));

    m_game->assets().getSound("LinkDie")->play();
}

void Scene_Win::update() {
    sRender();
    m_currentFrame++;
}

void Scene_Win::onEnd() {
    m_game->changeScene("MENU", nullptr, true);
}

void Scene_Win::sDoAction(const Action& action) {
    if (action.type() == "START") {
        if (action.name() == "QUIT") {
            onEnd();
        } else if (action.name() == "WINDOW_RESIZED") {
            storeViewCenter(m_game->getWindowSize() / 2.0);

            auto delta = action.pos() / 2;

            m_menuText.setPosition({ m_menuText.getPosition().x + delta.x, m_menuText.getPosition().y + delta.y});
            m_menuHelp.setPosition({ m_menuHelp.getPosition().x + delta.x, (float)(m_game->window().getSize().y - m_menuHelp.getGlobalBounds().size.y * 2) });

            if (!m_bgTexture.resize(m_game->window().getSize())) {
                std::cout << "can't resize bg texture" << std::endl;
                std::exit(-1);
            }
            m_bg.setTextureRect(sf::IntRect({0, 0}, Vec2(m_bgTexture.getSize())));
        }
    }
}

void Scene_Win::sRender() {
    m_game->window().clear(sf::Color(60, 60, 60, 128));
    m_game->getVertexArrays().clearAll();

    auto shader = m_game->assets().getShader("Matrix");
    shader->setUniform("u_resolution", sf::Vector2f(m_game->window().getSize().x, m_game->window().getSize().y));
    shader->setUniform("u_time", (float)m_currentFrame/60);
    m_game->getVertexArrays().draw(m_bg, "Matrix", nullptr, shader);
    m_game->getVertexArrays().drawAll(m_game->window());

    m_game->window().draw(m_menuText);
    m_game->window().draw(m_menuHelp);
}
