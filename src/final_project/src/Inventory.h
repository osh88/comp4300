#pragma once

#include "Artefact.h"
#include "Assets.h"
#include <SFML/Graphics.hpp>
#include <map>
#include <vector>
#include "entities/Entity.h"

class Inventory {
    struct Item {
        Artefact artefact;
        bool has = false;
    };

    bool m_drawInventory = false;

    std::vector<Item>   m_artefacts;
    std::vector<size_t> m_selected;
    size_t              m_currentWeapon = -1;

    void drawInventory(sf::RenderWindow & window, const Assets & assets);
    void drawHUD(sf::RenderWindow & window, const Assets & assets);

public:

    Inventory() = default;
    Inventory(int inventorySize, int maxSelectedArtefacts)
        : m_artefacts(inventorySize)
        , m_selected(maxSelectedArtefacts)
    {
        reset();
    };

    void reset();
    void addArtefact(const std::string & name);
    void delArtefact(const std::string & name, bool all = false);
    void selectArtefact(const std::string & name);
    void selectArtefact(size_t i);
    void useArtefact(size_t i, Entity & player);
    void drawInventory(bool value);
    bool drawInventory();
    void clearSelections();
    std::optional<Artefact> getWeapon();

    void draw(sf::RenderWindow & window, const Assets & assets);
};
