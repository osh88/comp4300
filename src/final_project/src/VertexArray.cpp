#include "VertexArray.h"
#include "Profiler.h"

VertexArray::VertexArray(int capacity) {
    m_vertices = sf::VertexArray(sf::PrimitiveType::Triangles, capacity * 6);
}

void VertexArray::clear() {
    PROFILE_FUNCTION();
    m_vertices.clear();
}

void VertexArray::draw(const sf::Sprite & sprite) {
    // Получаем глобальные вершины с учётом transform
    sf::Transform transform = sprite.getTransform();
    sf::FloatRect bounds = sprite.getLocalBounds();

    // Исходные локальные координаты (без transform)
    sf::Vector2f localVertices[4] = {
        {bounds.position.x,                 bounds.position.y},
        {bounds.position.x + bounds.size.x, bounds.position.y},
        {bounds.position.x + bounds.size.x, bounds.position.y + bounds.size.y},
        {bounds.position.x,                 bounds.position.y + bounds.size.y}
    };

    // Применяем transform к каждой вершине
    sf::Vector2f worldVertices[4];
    for (int i = 0; i < 4; ++i) {
        worldVertices[i] = transform.transformPoint(localVertices[i]);
    }

    // Текстурные координаты (с учётом textureRect)
    sf::IntRect textureRect = sprite.getTextureRect();
    float texLeft   = static_cast<float>(textureRect.position.x);
    float texTop    = static_cast<float>(textureRect.position.y);
    float texRight  = texLeft + textureRect.size.x;
    float texBottom = texTop + textureRect.size.y;

    // Заполняем вершины в массиве

    m_vertices.append({ worldVertices[0], sprite.getColor(), {texLeft,  texTop}    });
    m_vertices.append({ worldVertices[1], sprite.getColor(), {texRight, texTop}    });
    m_vertices.append({ worldVertices[3], sprite.getColor(), {texLeft,  texBottom} });
    m_vertices.append({ worldVertices[3], sprite.getColor(), {texLeft,  texBottom} });
    m_vertices.append({ worldVertices[1], sprite.getColor(), {texRight, texTop}    });
    m_vertices.append({ worldVertices[2], sprite.getColor(), {texRight, texBottom} });
}

void VertexArray::draw(sf::RenderWindow & window, std::shared_ptr<sf::Texture> texture, std::shared_ptr<sf::Shader> shader) {
    PROFILE_FUNCTION();

    if (texture != nullptr) {
        m_states.texture = texture.get();
    } else {
        m_states.texture = nullptr;
    }

    if (shader != nullptr) {
        m_states.shader = shader.get();
    } else {
        m_states.shader = nullptr;
    }

    window.draw(m_vertices, m_states);
}
