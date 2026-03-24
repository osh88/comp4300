#include "Scene.h"
#include "SFML/Graphics/PrimitiveType.hpp"
#include <iostream>

Scene::Scene() {}

Scene::Scene(GameEngine* gameEngine)
    : m_game(gameEngine)
{
    setBaseViewSize(m_game->getWindowSize());
    // m_viewSize = Vec2(m_game->window().getSize());
    // m_viewCenter = m_viewSize / 2.0f;
}

void Scene::doAction(const Action& action) {
    sDoAction(action);
}

void Scene::simulate(const size_t frames) {}

void Scene::registerAction(const std::string& actionName, sf::Keyboard::Key key, bool control, bool shift, bool alt, bool system) {
    // Затираем имеющуюся привязку
    unregisterAction(actionName);

    auto k = KeyAction(key, control, shift, alt, system);
    m_actionMap.emplace(k, actionName);
    //std::cout << "register action: " << actionName << " " << (int)k.key << " " << k.control << k.shift << k.alt << k.system << std::endl;
}

void Scene::unregisterAction(const std::string& actionName) {
    std::vector<KeyAction> toRemove;

    for (auto& [k, a] : m_actionMap) {
        if (a == actionName) {
            toRemove.push_back(k);
        }
    }

    for (auto k : toRemove) {
        m_actionMap.erase(k);
        //std::cout << "erase action: " << actionName << " " << (int)k.key << " " << k.control << k.shift << k.alt << k.system << std::endl;
    }
}

void Scene::clearActions() {
    m_actionMap.clear();
}

size_t Scene::currentFrame() const {
    return m_currentFrame;
}

std::optional<std::string> Scene::getAction(KeyAction k) const {
    if (auto r = m_actionMap.find(k); r != m_actionMap.end()) {
        //std::cout << "found action: " << r->second << " " << (int)k.key << " " << k.control << k.shift << k.alt << k.system << std::endl;
        return r->second;
    }

    return std::nullopt;
}

void Scene::drawLine(const Vec2& p1, const Vec2& p2, sf::Color color) {
    sf::Vertex line[] = {
        {{p1.x, p1.y}, color},
        {{p2.x, p2.y}, color}
    };

    m_game->window().draw(line, 2, sf::PrimitiveType::Lines);
}

void Scene::storeViewCenter(const Vec2 & pos) {
    m_storedViewCenter = pos;
}

const Vec2 & Scene::getStoredViewCenter() const {
    return m_storedViewCenter;
}

// void Scene::setViewSize(const Vec2 & s) {
//     m_viewSize = s;
// }

// const Vec2 & Scene::getViewSize() const {
//     return m_viewSize;
// }

Vec2 Scene::computedViewSize() {
    return m_baseViewSize + m_scaleStep * m_scale;
}

Vec2 Scene::getBaseViewSize() {
    return m_baseViewSize;
}

void Scene::setBaseViewSize(const Vec2 & s) {
    m_baseViewSize = s;
    m_scaleStep = m_baseViewSize * 0.01;
}
