// Darwin:
// clang++ -std=c++17 -I ../../sfml-darwin/include -L ../../sfml-darwin/lib -lsfml-window -lsfml-audio -lsfml-system -lsfml-graphics -rpath /Users/osh88/cpp/src/comp4300/sfml-darwin/lib *.cpp && ./a.out
//
// Linux:
// clang++ -std=c++20 -I ../../sfml-linux/include -L ../../sfml-linux/lib -lsfml-window -lsfml-audio -lsfml-system -lsfml-graphics -rpath /home/osh88/cpp/src/comp4300/sfml-linux/lib *.cpp && LD_LIBRARY_PATH=/home/osh88/cpp/src/comp4300/sfml-linux/lib:$LD_LIBRARY_PATH ./a.out

#include <SFML/Graphics/RenderWindow.hpp>
#include <iostream>

#include "ParticleSystem.hpp"
#include "Profiler.h"

int main() {
    PROFILE_FUNCTION();
    sf::RenderWindow window(sf::VideoMode({1024, 768}), "Particle System");
    window.setFramerateLimit(60);

    ParticleSystem particles;
    particles.init(window.getSize());

    while (window.isOpen()) {
        PROFILE_SCOPE("frame");

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::KeyPressed>() && event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape) {
                window.close();
            }

            if (event->is<sf::Event::KeyPressed>()) {
                particles.resetParticles();
            }
        }

        particles.update();

        window.clear(sf::Color::Black);

        particles.draw(window);

        window.display();
    }

    return 0;
}
