#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <math.h>

#include "Profiler.h"

class ParticleSystem {
    struct Particle {
        sf::Vector2f velocity;
        int lifetime = 0;
        int delay = 0;
    };

    std::vector<Particle> m_particles;
    sf::VertexArray       m_vertices;
    sf::Vector2u          m_windowSize;
    float                 m_size = 8;

    void resetParticle(size_t index, bool first = false) {
        PROFILE_FUNCTION();

        // give the particle an initial position
        float mx = m_windowSize.x / 2;
        float my = m_windowSize.y / 2;

        float h = (std::sqrt(3)/2) * m_size;
        float h13 = h/3;
        float h23 = (2*h)/3;

        m_vertices[3 * index + 0].position = sf::Vector2f(mx, my-h23);
        m_vertices[3 * index + 1].position = sf::Vector2f(mx+m_size/2, my+h13);
        m_vertices[3 * index + 2].position = sf::Vector2f(mx-m_size/2, my+h13);

        // give the particle a color
        auto color = sf::Color(255, 0, 255, 255);
        color = sf::Color(128 + rand()%128, 55, 55, rand()%255);

        if (first) { color.a = 0; }

        m_vertices[3 * index + 0].color = color;
        m_vertices[3 * index + 1].color = color;
        m_vertices[3 * index + 2].color = color;

        // give the particle a random velocity
        float rx = ((float)rand() / (float)RAND_MAX) * 6 - 3;
        float ry = ((float)rand() / (float)RAND_MAX) * 6 - 3;

        m_particles[index].velocity = sf::Vector2f(rx, ry);

        // give the particle a lifespan
        m_particles[index].lifetime = 30 + rand() % 60;
        //m_particles[index].lifetime = first ? (30 + rand() % 60) : (rand() % 20);
        //m_particles[index].lifetime = first ? (rand() % 20) : (30 + rand() % 60);
    }

public:

    ParticleSystem() {}

    void resetParticles(size_t count = 1024, float size = 8) {
        PROFILE_FUNCTION();

        m_particles = std::vector<Particle>(count);
        m_vertices = sf::VertexArray(sf::PrimitiveType::Triangles, count * 3);
        m_size = size;

        for (size_t p = 0; p < m_particles.size(); p++) {
            resetParticle(p, true);
        }
    }


    void init(sf::Vector2u windowSize) {
        m_windowSize = windowSize;
        resetParticles();
    }

    void update() {
        PROFILE_FUNCTION();
        for (size_t i = 0; i < m_particles.size(); ++i) {
            if (m_particles[i].lifetime == 0) {
                resetParticle(i);
            }

            m_vertices[3 * i + 0].position += m_particles[i].velocity;
            m_vertices[3 * i + 1].position += m_particles[i].velocity;
            m_vertices[3 * i + 2].position += m_particles[i].velocity;

            m_vertices[3 * i + 0].color.a -= (m_vertices[3 * i + 0].color.a > 0) ? 1 : 0;
            m_vertices[3 * i + 1].color.a -= (m_vertices[3 * i + 1].color.a > 0) ? 1 : 0;
            m_vertices[3 * i + 2].color.a -= (m_vertices[3 * i + 2].color.a > 0) ? 1 : 0;

            m_particles[i].lifetime--;
        }
    }

    void draw(sf::RenderWindow & window) {
        PROFILE_FUNCTION();
        window.draw(m_vertices);
    }
};
