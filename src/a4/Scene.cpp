#include "Scene.h"
#include "SFML/Graphics/PrimitiveType.hpp"

Scene::Scene() {}

Scene::Scene(GameEngine* gameEngine)
        : m_game(gameEngine) {}

void Scene::doAction(const Action& action) {
    sDoAction(action);
}

void Scene::simulate(const size_t frames) {}

void Scene::registerAction(sf::Keyboard::Key inputKey, const std::string& actionName) {
    m_actionMap[(int)inputKey] = actionName;
}

size_t Scene::width() const {
    return m_game->window().getSize().x;
}

size_t Scene::height() const {
    return m_game->window().getSize().y;
}

size_t Scene::currentFrame() const {
    return m_currentFrame;
}

bool Scene::hasEnded() const {
    return m_hasEnded;
}

const ActionMap & Scene::getActionMap() const {
    return m_actionMap;
}

void Scene::drawLine(const Vec2& p1, const Vec2& p2, sf::Color color) {
    sf::Vertex line[] = {
        {{p1.x, p1.y}, color},
        {{p2.x, p2.y}, color}
    };

    m_game->window().draw(line, 2, sf::PrimitiveType::Lines);
}
