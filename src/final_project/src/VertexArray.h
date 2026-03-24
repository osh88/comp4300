#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

#include "Assets.h"
#include "Entity.h"
#include "Vec2.h"

class VertexArray {
    sf::VertexArray  m_vertices;
    sf::RenderStates m_states;

public:

    VertexArray(int capacity = 3000);
    
    void clear();
    void draw(const sf::Sprite & sprite);
    void draw(sf::RenderWindow & window, std::shared_ptr<sf::Texture> texture = nullptr, std::shared_ptr<sf::Shader> shader = nullptr);
};