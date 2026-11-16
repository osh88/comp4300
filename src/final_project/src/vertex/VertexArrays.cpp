#include "VertexArrays.h"

VertexArrays::VertexArrays(int defaultArrayCapacity)
    : m_defaultCapacity(defaultArrayCapacity)
{};

const std::vector<std::string> VertexArrays::getGroups() const {
    return m_groupOrder;
};

void VertexArrays::clear(const std::string & groupName) {
    if (auto g = m_groups.find(groupName); g != m_groups.end()) {
        g->second.array.clear();
        g->second.texture = nullptr;
        g->second.shader = nullptr;
    }
}

void VertexArrays::clearAll() {
    for (auto& g : m_groups) {
        g.second.array.clear();
        g.second.texture = nullptr;
        g.second.shader = nullptr;
    }
}

void VertexArrays::deleteGroups() {
    m_groups.clear();
    m_groupOrder.clear();
}

void VertexArrays::draw(const sf::Sprite & sprite, const std::string & groupName, std::shared_ptr<sf::Texture> texture, std::shared_ptr<sf::Shader> shader, int capacity) {
    if (auto g = m_groups.find(groupName); g == m_groups.end()) {
        m_groupOrder.push_back(groupName);
        auto va = VertexArray(capacity > 0 ? capacity : m_defaultCapacity);
        m_groups[groupName] = Group(groupName, va, texture, shader);
        va.draw(sprite);
    } else {
        g->second.array.draw(sprite);
        if (texture != nullptr) {
            g->second.texture = texture;
        }
        if (shader != nullptr) {
            g->second.shader = shader;
        }
    }
}

void VertexArrays::draw(sf::RenderWindow & window, const std::string & groupName, std::shared_ptr<sf::Texture> texture, std::shared_ptr<sf::Shader> shader) {
    if (auto g = m_groups.find(groupName); g != m_groups.end()) {
        g->second.array.draw(
            window,
            texture != nullptr ? texture : g->second.texture,
            shader != nullptr ? shader : g->second.shader
        );
    }
}

void VertexArrays::drawAll(sf::RenderWindow & window) {
    for (auto groupName : m_groupOrder) {
        if (auto g = m_groups.find(groupName); g != m_groups.end()) {
            g->second.array.draw(window, g->second.texture, g->second.shader);
        }
    }
}