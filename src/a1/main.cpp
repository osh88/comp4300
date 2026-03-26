// Darwin:
// clang++ -std=c++20 -I ../sfml-darwin/include -L ../sfml-darwin/lib -lsfml-window -lsfml-audio -lsfml-system -lsfml-graphics -rpath "\$ORIGIN/../../sfml-darwin/lib" *.cpp && ./a.out
//
// Linux:
// clang++ -std=c++20 -I ../../sfml-linux/include -L ../../sfml-linux/lib -lsfml-window -lsfml-audio -lsfml-system -lsfml-graphics -rpath "\$ORIGIN/../../sfml-linux/lib" *.cpp && ./a.out

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <iostream>
#include <fstream>
#include <math.h>
#include "game_engine.h"

class Vec2 {
public:
    float x = 0;
    float y = 0;

    Vec2() {};

    Vec2(float xin, float yin)
        : x (xin)
        , y (yin)
    {}

    Vec2 operator + (const Vec2& rhs) const {
        return Vec2(x + rhs.x, y + rhs.y);
    }

    void operator += (const Vec2& rhs) {
        x += rhs.x;
        y += rhs.y;
    }

    Vec2& add(const Vec2& v) {
        x += v.x;
        y += v.y;
        return *this;
    }

    Vec2& scale(float s) {
        x *= s;
        y *= s;
        return *this;
    }

    float dist(const Vec2& v) const {
        return sqrtf((v.x-x)*(v.x-x) + (v.y-y)*(v.y-y));
    }
};

sf::Vector2<float> v2(float x, float y) {
    return sf::Vector2<float>{x, y};
}

class Color {
public:

    int R, G, B;

    Color() {};

    Color(int r, int g, int b)
        : R(r)
        , G(g)
        , B(b)
    {}

    void print() const {
        std::cout << "(" << R << "," << G << "," << B << ")";
    }
};

class Shape {
protected:
    std::string m_name;
    float m_x, m_y, m_sx, m_sy;
    Color m_color;

    std::shared_ptr<sf::Font> m_font;
    std::unique_ptr<sf::Text> m_text;

    float m_text_width, m_text_height;
    float m_textLeft = 0;
    float m_textTop = 0;

public:

    Shape() {};

    Shape(std::string& name, float x, float y, float sx, float sy, Color color)
        : m_name  (name)
        , m_x     (x)
        , m_y     (y)
        , m_sx    (sx)
        , m_sy    (sy)
        , m_color (color)
    {}

    virtual ~Shape() = default;

    virtual void init(std::shared_ptr<sf::Font> font) {
        m_font = font;
        m_text = std::make_unique<sf::Text>(*font, m_name, 12);
        m_text_width = m_text->getLocalBounds().size.x;
        m_text_height = m_text->getLocalBounds().size.y;
    }

    virtual void Draw(sf::RenderWindow& window) {};

    virtual void print() const {
        std::cout << "name=" << m_name << " x=" << m_x << " y=" << m_y;
        std::cout << " sx=" << m_sx << " sy=" << m_sy << " color=";
        m_color.print();
    };
};

class Circle : public Shape {
    float m_radius;
    sf::CircleShape m_sfShape;

public:

    Circle(std::string& name, float x, float y, float sx, float sy, Color color, float radius)
        : Shape(name, x, y, sx, sy, color)
        , m_radius  (radius)
        , m_sfShape (sf::CircleShape(radius))
    {
        m_sfShape.setFillColor(sf::Color(m_color.R, m_color.G, m_color.B));
        m_sfShape.setPosition(v2(m_x, m_y));
    }

    void init(std::shared_ptr<sf::Font> font) override {
        Shape::init(font);
        m_textLeft = (m_radius*2 - m_text_width) / 2;
        m_textTop = (m_radius*2 - m_text_height) / 2;
    }

    void Draw(sf::RenderWindow& window) override {
        m_x += m_sx;
        m_y += m_sy;

        if (m_x <=0 || m_x+m_sfShape.getRadius()*2 >= window.getSize().x) {
            m_sx = -m_sx;
        }
        if (m_y <=0 || m_y+m_sfShape.getRadius()*2 >= window.getSize().y) {
            m_sy = -m_sy;
        }

        m_sfShape.setPosition(v2(m_x, m_y));

        m_text->setOrigin(m_text->getGlobalBounds().size / 2.f + m_text->getLocalBounds().position);
        m_text->setPosition(m_sfShape.getPosition() + v2(m_sfShape.getRadius(), m_sfShape.getRadius()));

        window.draw(m_sfShape);
        window.draw(*m_text);
    };

    void print() const override {
        std::cout << "Circle: ";
        Shape::print();
        std::cout << " radius=" << m_radius << std::endl;
    }
};

class Rectangle : public Shape {
    float m_width, m_height;
    sf::RectangleShape m_sfShape;

public:

    Rectangle(std::string& name, float x, float y, float sx, float sy, Color color, float width, float height)
        : Shape(name, x, y, sx, sy, color)
        , m_width  (width)
        , m_height (height)
        , m_sfShape (sf::RectangleShape(v2(width, height)))
    {
        m_sfShape.setFillColor(sf::Color(m_color.R, m_color.G, m_color.B));
        m_sfShape.setPosition(v2(m_x, m_y));
    }

    void init(std::shared_ptr<sf::Font> font) override {
        Shape::init(font);
        m_textLeft = (m_width - m_text_width) / 2;
        m_textTop = (m_height - m_text_height) / 2;
    }

    void Draw(sf::RenderWindow& window) override {
        m_x += m_sx;
        m_y += m_sy;

        if (m_x <=0 || m_x+m_sfShape.getSize().x >= window.getSize().x) {
            m_sx = -m_sx;
        }
        if (m_y <=0 || m_y+m_sfShape.getSize().y >= window.getSize().y) {
            m_sy = -m_sy;
        }

        m_sfShape.setPosition(v2(m_x, m_y));

        m_text->setOrigin(m_text->getGlobalBounds().size / 2.f + m_text->getLocalBounds().position);
        m_text->setPosition(m_sfShape.getPosition() + (m_sfShape.getSize() / 2.f));

        window.draw(m_sfShape);
        window.draw(*m_text);
    };

    void print() const override {
        std::cout << "Rectangle: ";
        Shape::print();
        std::cout << " width=" << m_width << " height=" << m_height << std::endl;
    };
};

class Config {
    std::string m_configPath;
    int m_windowWidth;
    int m_windowHeight;
    std::string m_fontPath;
    int m_fontSize;
    Color m_fontColor;
    std::vector<std::shared_ptr<Shape>> m_shapes;

public:

    Config(std::string path)
        : m_configPath (path)
    {
        std::ifstream fin(m_configPath);
        std::string token;
        std::string name;
        float x, y, sx, sy;
        Color color;
        int r,g,b;
        float radius, width, height;

        // Window int width, int height
        // Font std::string path, int size, int r, int g, int b
        // Circle std::string name, float x, float y, float speedX, float speedY, int r, int g, int b, float radius
        // Rectangle std::string name, float x, float y, float speedX, float speedY, int r, int g, int b, float width, float height

        while (fin >> token) {
            if (token == "Window") {
                fin >> m_windowWidth >> m_windowHeight;
            } else if (token == "Font") {
                fin >> m_fontPath >> m_fontSize >> r >> g >> b;
                m_fontColor = Color(r, g, b);
            } else if (token == "Circle") {
                fin >> name >> x >> y >> sx >> sy >> r >> g >> b >> radius;
                color = Color(r, g, b);
                m_shapes.push_back(std::make_shared<Circle>(name, x, y, sx, sy, color, radius));
            } else if (token == "Rectangle") {
                fin >> name >> x >> y >> sx >> sy >> r >> g >> b >> width >> height;
                color = Color(r, g, b);
                m_shapes.push_back(std::make_shared<Rectangle>(name, x, y, sx, sy, color, width, height));
            } else {
                std::cout << "Unknown token " << token << std::endl;
                exit(-1);
            }
        }
    }

    virtual ~Config() = default;

    void print() const {
        std::cout << "configPath=" << m_configPath << std::endl;
        std::cout << "window=" << m_windowWidth << "," << m_windowHeight << std::endl;
        std::cout << "font=" << m_fontPath << "," << m_fontSize << ",";
        m_fontColor.print();
        std::cout << std::endl;

        for (auto& shape : m_shapes) {
            shape->print();
        }
    };

    const std::vector<std::shared_ptr<Shape>> getShapes() const {
        return m_shapes;
    }
};

int main()
{
    auto eng = GameEngine();
    eng.getEntityManager()->addEntity("player");

    Config cfg = Config("config.txt");
    cfg.print();

    const int wWidth = 800;
    const int wHeight = 600;

    // Create the main window
    sf::RenderWindow window(sf::VideoMode({wWidth, wHeight}), "SFML works!");
//    window.setFramerateLimit(60);

    sf::Font myFont;

    if (!myFont.openFromFile("../../fonts/arcadeclassic.ttf")) {
        std::cerr << "Could not load font!" << std::endl;
        exit(-1);
    }

    for (const auto shape : cfg.getShapes()) {
        shape->init(std::make_shared<sf::Font>(myFont));
    }

    // Start the game loop
    while (window.isOpen())
    {
        // Process events
        while (const std::optional event = window.pollEvent())
        {
            // Close window: exit
            if (event->is<sf::Event::Closed>() || (event->is<sf::Event::KeyPressed>() &&
                 event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape)) {
                window.close();
            }
        }

        // Clear screen
        window.clear();

        for (const auto shape : cfg.getShapes()) {
            shape->Draw(window);
        }

        // Update the window
        window.display();
    }
}
