#include "Inventory.h"
#include <SFML/Graphics.hpp>
#include "Artefact.h"
#include <iostream>
#include "Vec2.h"

void Inventory::addArtefact(const std::string & name) {
    auto newA = Artefact::make(name);
    if (!newA.has_value()) { return; }

    auto a = std::find_if(m_artefacts.begin(), m_artefacts.end(), [name](Item& item) -> bool {
        return item.artefact.name == name && item.has;
    });

    if (a != m_artefacts.end()) {
        a->artefact.count += newA->count;
        return;
    }

    for (size_t i=0; i<m_artefacts.size(); i++) {
        if (!m_artefacts[i].has) {
            m_artefacts[i] = Item{newA.value(), true};
            selectArtefact(i);
            break;
        }
    }
}

void Inventory::delArtefact(const std::string & name, bool all) {
    auto a = std::find_if(m_artefacts.begin(), m_artefacts.end(), [name](Item& item) -> bool {
        return item.has && item.artefact.name == name;
    });

    if (a == m_artefacts.end()) {
        return;
    }

    a->artefact.count--;

    if (a->artefact.count <= 0 || all) {
        a->has = false;

        size_t index = std::distance(m_artefacts.begin(), a);
        for (size_t i=0; i<m_selected.size(); i++) {
            if (m_selected[i] == index) {
                m_selected[i] = -1;
            }
        }

        // Если оружие закончилось, выбираем следующее самое сильное (в списке выбранных предметов)
        if (m_currentWeapon == index) {
            int index = -1; 
            int damage = 0;

            for (auto idx : m_selected) {
                if (idx < 0) { continue; }

                if (m_artefacts[idx].has && m_artefacts[idx].artefact.group == AG_WEAPON && m_artefacts[idx].artefact.type == AT_DAMAGE && m_artefacts[idx].artefact.value > damage) {
                    damage = m_artefacts[idx].artefact.value;
                    index = idx;
                }
            }

            if (index >= 0) {
                m_currentWeapon = index;
            }
        }
    }
}

void Inventory::selectArtefact(int index) {
    if (index < 0 || index >= m_artefacts.size()) {
        return;
    }

    if (!m_artefacts[index].has) {
        return;
    }

    for (auto idx : m_selected) {
        if (idx == index) {
            return;
        }
    }

    // Помещаем предмет в свободный слот
    for (auto& idx : m_selected) {
        if (idx < 0) {
            idx = index;

            // Если предмет является оружием, используем его
            if (m_artefacts[index].artefact.group == AG_WEAPON) {
                m_currentWeapon = index;
            }

            break;
        }
    }
}

void Inventory::selectArtefact(const std::string & name) {
    for (int i=0; i<m_artefacts.size(); i++) {
        if (m_artefacts[i].artefact.name == name) {
            selectArtefact(i);
            break;
        }
    }
}

void Inventory::useArtefact(int i, Entity & player) {
    if (i < 0 || i >= m_selected.size()) {
        return;
    }

    i = m_selected[i];
    if (i < 0 || i >= m_artefacts.size() || !m_artefacts[i].has) {
        return;
    }

    auto art = m_artefacts[i].artefact;
    if (art.group == AG_WEAPON) {
        m_currentWeapon = i;
        return;
    }

    switch (art.type) {
        case AT_NONE: {
            break;
        }

        case AT_DAMAGE: {
            break;
        }

        case AT_HEALING: {
            auto& c = player.getComponent<CHealth>();
            c.current += art.value;
            if (c.current > c.max) { c.current = c.max; }
            delArtefact(art.name);
            break;
        }

        case AT_INVINCIBILITY: {
            player.addComponent<CInvincibility>(art.lifetime);
            delArtefact(art.name);
            break;
        }

        case AT_ANTIGRAVITY: {
            player.getComponent<CGravity>().antiGravity = art.value;
            player.getComponent<CGravity>().antiGravLifetime = art.lifetime;
            delArtefact(art.name);
            break;
        }

        case AT_SPEED: {
            // player.getComponent<CTransform>().velocity.x = art.value;
            // player.getComponent<CTransform>().forcedSpeedX = art.value;
            // player.getComponent<CTransform>().forcedSpeedLifetime = art.lifetime;
            delArtefact(art.name);
            break;
        }
    }
}

void Inventory::drawInventory(sf::RenderWindow & window, const Assets & assets) {
    float thickness = 2.0;
    auto size = Vec2(64, 64);
    auto pos = Vec2(window.getView().getCenter());
    pos.x -= size.x * m_artefacts.size() / 2.0;

    sf::RectangleShape sh;

    auto curtainSize = Vec2(window.getView().getSize())-5;
    sh.setSize(curtainSize);
    sh.setOrigin(curtainSize/2.0);
    sh.setFillColor(sf::Color(0, 0, 0, 100));
    sh.setOutlineColor(sf::Color::White);
    sh.setOutlineThickness(thickness);
    sh.setPosition(Vec2(window.getView().getCenter()));
    window.draw(sh);

    sh.setSize(size);
    sh.setOrigin(size/2);
    sh.setFillColor(sf::Color(255, 255, 255, 80));
    sh.setOutlineColor(sf::Color::White);
    sh.setOutlineThickness(thickness);

    for (int i=0; i<m_artefacts.size(); i++) {
        auto art = m_artefacts[i];
        sh.setPosition(pos);
        window.draw(sh);

        if (art.has) {
            auto a = assets.getAnimation(art.artefact.invAnimation);
            a.getSprite().setOrigin(a.frameSize() / 2.0);
            a.getSprite().setPosition(pos);
            a.getSprite().setTexture(*assets.getLargeTexture());
            window.draw(a.getSprite());
        }

        sf::Text t(assets.getFont("Mario").font, std::to_string(i+1), 10);
        t.setOrigin(Vec2(t.getGlobalBounds().size) / 2.0);
        t.setPosition(pos-size/3);
        t.setFillColor(sf::Color::Yellow);
        t.setOutlineColor(sf::Color::Black);
        t.setOutlineThickness(1.0);
        window.draw(t);

        pos.x += size.x+thickness;
    }

    pos = Vec2(window.getView().getCenter());
    pos.x -= size.x * m_artefacts.size() / 2.0;

    for (auto art : m_artefacts) {
        if (art.has && art.artefact.count > 0) {
            sf::Text t(assets.getFont("Mario").font, std::to_string(art.artefact.count), 16);
            t.setOrigin(Vec2(t.getGlobalBounds().size) / 2.0);
            t.setPosition(pos+size/4);
            t.setFillColor(sf::Color::Black);
            t.setOutlineColor(sf::Color::White);
            t.setOutlineThickness(1.0);
            window.draw(t);
        }

        pos.x += size.x+thickness;
    }
}

void Inventory::drawHUD(sf::RenderWindow & window, const Assets & assets) {
    float thickness = 2.0;
    auto size = Vec2(36, 36);
    auto pos = Vec2(window.mapPixelToCoords(Vec2(500, thickness + 5 + size.y/2.0)));

    sf::RectangleShape sh;
    sh.setSize(size);
    sh.setOrigin(size/2);
    sh.setFillColor(sf::Color(255, 255, 255, 80));
    sh.setOutlineColor(sf::Color::White);
    sh.setOutlineThickness(thickness);

    for (int i=0; i<m_selected.size(); i++) {
        auto idx = m_selected[i];
        sh.setPosition(pos);
        window.draw(sh);

        if (idx >= 0 && idx < m_artefacts.size() && m_artefacts[idx].has) {
            auto art = m_artefacts[idx];
            auto a = assets.getAnimation(art.artefact.invAnimation);
            a.getSprite().setOrigin(a.frameSize() / 2.0);
            a.getSprite().setPosition(pos);
            a.getSprite().setTexture(*assets.getLargeTexture());
            a.getSprite().setScale(Vec2(0.55, 0.55));
            window.draw(a.getSprite());
        }

        sf::Text t(assets.getFont("Mario").font, std::to_string(i+1), 7);
        t.setOrigin(Vec2(t.getGlobalBounds().size) / 2.0);
        t.setPosition(pos-size/3);
        t.setFillColor(sf::Color::Yellow);
        t.setOutlineColor(sf::Color::Black);
        t.setOutlineThickness(1.0);
        window.draw(t);

        pos.x += size.x+thickness;
    }

    pos = Vec2(window.mapPixelToCoords(Vec2(500, thickness + 5 + size.y/2.0)));

    for (auto i : m_selected) {
        if (i >= 0 && i < m_artefacts.size() && m_artefacts[i].has) {
            if (auto art = m_artefacts[i]; art.artefact.count > 0) {
                sf::Text t(assets.getFont("Mario").font, std::to_string(art.artefact.count), 9);
                t.setOrigin(Vec2(t.getGlobalBounds().size) / 2.0);
                t.setPosition(pos+size/4);
                t.setFillColor(sf::Color::Black);
                t.setOutlineColor(sf::Color::White);
                t.setOutlineThickness(1.0);
                window.draw(t);
            }
        }

        pos.x += size.x+thickness;
    }
}

void Inventory::draw(sf::RenderWindow & window, const Assets & assets) {
    drawHUD(window, assets);
    if (m_drawInventory) {
        drawInventory(window, assets);
    }
}

bool Inventory::drawInventory() {
    return m_drawInventory;
}

void Inventory::drawInventory(bool value) {
    m_drawInventory = value;
}

void Inventory::reset() {
    for (auto& a : m_artefacts) {
        a.artefact = Artefact();
        a.has = false;
    }

    for (auto& i : m_selected) {
        i = -1;
    }

    addArtefact("sword");
    m_selected[0] = 0;
    m_currentWeapon = 0;
}

void Inventory::clearSelections() {
    for (auto& i : m_selected) {
        i = -1;
    }
    m_currentWeapon = -1;
}

std::optional<Artefact> Inventory::getWeapon() {
    if (m_currentWeapon >= 0 && m_currentWeapon < m_artefacts.size() && m_artefacts[m_currentWeapon].has) {
        return m_artefacts[m_currentWeapon].artefact;
    }

    return std::nullopt;
}