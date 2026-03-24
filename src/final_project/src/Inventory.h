#pragma once

#include "Artefact.h"
#include "Assets.h"
#include <SFML/Graphics.hpp>
#include <map>
#include <vector>
#include "Entity.h"

class Inventory {
    struct Item {
        Artefact artefact;
        bool has = false;
    };

    bool m_drawInventory = false;

    std::vector<Item> m_artefacts;
    std::vector<int>  m_selected;
    int               m_currentWeapon = -1;

    void drawInventory(sf::RenderWindow & window, const Assets & assets);
    void drawHUD(sf::RenderWindow & window, const Assets & assets);

public:

    Inventory() {};
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
    void selectArtefact(int i);
    void useArtefact(int i, Entity & player);
    void drawInventory(bool value);
    bool drawInventory();
    void clearSelections();
    std::optional<Artefact> getWeapon();

    void draw(sf::RenderWindow & window, const Assets & assets);
};
