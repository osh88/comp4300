#pragma once

#include "VertexArray.h"
#include <map>
#include <string>

const size_t DEFAULT_CAPACITY = 3000;

class VertexArrays {
    class Group {
    public:
        std::string                  name    = "";
        VertexArray                  array   = VertexArray(DEFAULT_CAPACITY);
        std::shared_ptr<sf::Texture> texture = nullptr;
        std::shared_ptr<sf::Shader>  shader  = nullptr;

        Group()
            : name(""), array(VertexArray(DEFAULT_CAPACITY)), texture(nullptr), shader(nullptr)
        {};

        Group(const std::string & name, const VertexArray & arr, std::shared_ptr<sf::Texture> texture = nullptr, std::shared_ptr<sf::Shader> shader = nullptr)
            : name(name), array(arr), texture(texture), shader(shader)
        {};
    };

    int                          m_defaultCapacity = DEFAULT_CAPACITY;
    std::map<std::string, Group> m_groups = std::map<std::string, Group>();
    std::vector<std::string>     m_groupOrder;

public:

    VertexArrays(int defaultArrayCapacity = DEFAULT_CAPACITY);
    const std::vector<std::string> getGroups() const;
    void deleteGroups();

    void clear(const std::string & groupName = "");
    void clearAll();

    void draw(const sf::Sprite & sprite, const std::string & groupName = "", std::shared_ptr<sf::Texture> texture = nullptr, std::shared_ptr<sf::Shader> shader = nullptr, int capacity = 0);
    void draw(sf::RenderWindow & window, const std::string & groupName = "", std::shared_ptr<sf::Texture> texture = nullptr, std::shared_ptr<sf::Shader> shader = nullptr);
    void drawAll(sf::RenderWindow & window);
};