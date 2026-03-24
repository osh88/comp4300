#include "Scene_Editor.h"
#include "Scene_Menu.h"
#include "Scene_Play.h"
#include "Scene_World.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "Level.h"
#include "Physics.h"

#include <iostream>
#include <SFML/Window/Cursor.hpp>

Scene_Editor::Scene_Editor(GameEngine* gameEngine)
    : Scene(gameEngine)
    , m_gridText(m_game->assets().getFont("Tech").font, "0", 10)
{}

void Scene_Editor::init() {
    m_bgColor = sf::Color(0x97, 0xAD, 0xB7, 0x51);

    if (auto cursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Hand); cursor.has_value()) {
        m_game->window().setMouseCursor(cursor.value());
    }

    storeViewCenter(m_game->getWindowSize() / 2.0);
    m_leftBottomOfLevel = Vec2(0, m_game->getWindowSize().y);

    registerAction("QUIT",           sf::Keyboard::Key::Escape);
    registerAction("TOGGLE_GRID",    sf::Keyboard::Key::G, true);
    registerAction("TOGGLE_BB",      sf::Keyboard::Key::B, true);
    registerAction("SELECT_TO_COPY", sf::Keyboard::Key::C, true);
    registerAction("PASTE_ENTITY",   sf::Keyboard::Key::V, true);

    registerAction("MOVE_UP",     sf::Keyboard::Key::Up);
    registerAction("MOVE_DOWN",   sf::Keyboard::Key::Down);
    registerAction("MOVE_LEFT",   sf::Keyboard::Key::Left);
    registerAction("MOVE_RIGHT",  sf::Keyboard::Key::Right);

    registerAction("FAST_MOVE_UP",     sf::Keyboard::Key::Up, false, true);
    registerAction("FAST_MOVE_DOWN",   sf::Keyboard::Key::Down, false, true);
    registerAction("FAST_MOVE_LEFT",   sf::Keyboard::Key::Left, false, true);
    registerAction("FAST_MOVE_RIGHT",  sf::Keyboard::Key::Right, false, true);

    m_entityManager = EntityManager();
    EntityMemoryPool::Instance().clear();


    for (auto& item : m_game->assets().getAnimations()) {
        m_animations.push_back(item.second);
    }

    std::sort(m_animations.begin(), m_animations.end(), [](const Animation& a, const Animation& b) {
        return a.getName() < b.getName();
    });

    // registerAction("PAUSE",          sf::Keyboard::Key::P, true);
    // registerAction("TOGGLE_TEXTURE", sf::Keyboard::Key::T, true);
    // registerAction("TOGGLE_DEBUG",   sf::Keyboard::Key::D, true);
    
    // registerAction("RELOAD",         sf::Keyboard::Key::R, true);
    // registerAction("TOGGLE_SLOW",    sf::Keyboard::Key::S, true);    
}

Vec2 Scene_Editor::newSpritePos() {
    return m_game->p2c(sf::Mouse::getPosition(m_game->window()));
}

void Scene_Editor::afterSpawn(const Entity & e, const Vec2 & pos) {
    m_sDnD.entity = e;
    m_sDnD.mousePos = pos;
    m_sDnD.startMousePos = pos;
    m_sDnD.startEntPos = pos;
    m_selectedEntity = e;
    m_entityManager.sortEntitiesByZ();
}

void Scene_Editor::spawnPlayer() {
    // Player GX GY CW CH SX SY SM GY B H
    //  Grid Pos          GX, GY  float, float (starting position of player)
    //  BoundingBox W/H   CW, CH  float, float
    //  Left/Right Speed  SX      float
    //  Jump Speed        SY      float
    //  Max Speed         SM      float
    //  Gravity           GY      float
    //  Bullet Animation  B       std::string (Animation asset to use for bullets)
    //  Max Health        H       int

    for (auto e : m_entityManager.getEntities("player")) {
        e.destroy();
    }
    m_entityManager.update();

    m_player = m_entityManager.addEntity("player");
    m_player.addComponent<CAnimation>(m_game->assets().getAnimation("Stand"), true);
    auto pos = newSpritePos();
    m_player.addComponent<CTransform>(pos, 1);
    m_player.addComponent<CBoundingBox>(Vec2(44, 48), true, false);
    m_player.addComponent<CGravity>(0.75);
    m_player.addComponent<CDraggable>();
    m_player.addComponent<CDamage>(1);
    m_player.addComponent<CHealth>(10, 10);
    m_player.addComponent<CState>();
    m_player.addComponent<CInput>();

    afterSpawn(m_player, pos);
}

void Scene_Editor::spawnDec() {
    //Dec N GX GY Z R
    //  Animation Name  N   std::string (Animation asset name for this tile)
    //  GX Grid X Pos   GX  float
    //  GY Grid Y Pos   GY  float
    //  Distance Z      Z   float
    //  Repeat X        R   int > 0

    auto e = m_entityManager.addEntity("dec");
    e.addComponent<CAnimation>(m_game->assets().getAnimation("CloudSmall"), true);
    auto pos = newSpritePos();
    e.addComponent<CTransform>(pos, 0);

    afterSpawn(e, pos);
}

void Scene_Editor::spawnTile() {
    //Tile N GX GY BM BV
    //  Animation Name  N   std::string (Animation asset name for this tile)
    //  GX Grid X Pos   GX  float
    //  GY Grid Y Pos   GY  float
    //  Blocks Movement BM  int (1 = true, 0 = false)
    //  Blocks Vision   BV  int (1 = true, 0 = false)

    auto e = m_entityManager.addEntity("tile");
    e.addComponent<CAnimation>(m_game->assets().getAnimation("Ground"), true);
    e.addComponent<CBoundingBox>(m_game->assets().getAnimation("Ground").frameSize(), 1, 1);
    auto pos = newSpritePos();
    e.addComponent<CTransform>(pos, 0);

    afterSpawn(e, pos);
}

void Scene_Editor::spawnMTile() {
    //MTile N GX GY BM BV S N X1 Y1 X2 Y2 ... XN YN
    //  Animation Name    N      std::string (Animation asset name for this tile)
    //  GX Grid X Pos     GX     float
    //  GY Grid Y Pos     GY     float
    //  Blocks Movement   BM     int (1 = true, 0 = false)
    //  Blocks Vision     BV     int (1 = true, 0 = false)
    //  Patrol Speed      S      float
    //  Patrol Positions  N      int (number of patrol positions)
    //  Position 1-N      Xi Yi  int, int (Tile Position of Patrol Position i)

    auto e = m_entityManager.addEntity("tile");
    e.addComponent<CAnimation>(m_game->assets().getAnimation("Block"), true);
    e.addComponent<CBoundingBox>(m_game->assets().getAnimation("Block").frameSize(), 1, 1);
    auto pos = newSpritePos();
    e.addComponent<CTransform>(pos, 0);
    auto v = std::vector<Vec2>{pos, Vec2(pos.x + 3*m_gridSize.x, pos.y)};
    e.addComponent<CPatrol>(v, 2);

    afterSpawn(e, pos);
}

void Scene_Editor::spawnNPC(const std::string & ai) {
    // NPC G F GX GY BM BV H D AI ...
    //   Animation Name    N      string
    //   Gravity           G      int (1 = true, 0 = false)
    //   Tile Position     GX GY  int, int
    //   Blocks Movement   BM     int (1 = true, 0 = false)
    //   Blocks Vision     BV     int (1 = true, 0 = false)
    //   Max Health        H      int > 0
    //   Damage            D      int > 0
    //   AI Behavior Name  AI     string
    //   AI Parameters     ...    (see below)
    //   Bounding Box      BX BY  int, int

    // AI = Follow
    //   ... = S
    //   Follow Speed   S   float (speed to follow player)
    //   Vision Radius  VR  float

    // AI = Patrol
    //   ... = S N X1 Y1 X2 Y2 ... XN YN VR
    //   Patrol Speed      S      float
    //   Patrol Positions  N      int (number of patrol positions)
    //   Position 1-N      Xi Yi  int, int (Tile Position of Patrol Position i)

    int bx = 0, by = 0; //, n = 0;
    float s = 1, vr = 256; //, x = 0, y = 0, g = 0;

    auto e = m_entityManager.addEntity("npc");
    e.addComponent<CAnimation>(m_game->assets().getAnimation("GoombaW"), true);
    auto bb = m_game->assets().getAnimation("GoombaW").frameSize()-4;
    if (bx == 0) bx = bb.x;
    if (by == 0) by = bb.y;
    e.addComponent<CBoundingBox>(Vec2(bx, by), 0, 0); 
    auto pos = newSpritePos();
    e.addComponent<CTransform>(pos, 0);
    e.addComponent<CHealth>(5, 5);
    e.addComponent<CDamage>(1);

    if (ai == "Follow") {
        //fin >> s >> vr;
        if (vr == 0) { // Оставляем значение по умолчанию
            e.addComponent<CFollowPlayer>(pos, s);
        } else {
            e.addComponent<CFollowPlayer>(pos, s, vr);
        }
        e.addComponent<CGravity>(0);
    } else if (ai == "Patrol") {
        std::vector<Vec2> v;
        v.push_back(pos);
        v.push_back(Vec2(pos.x + 3*m_gridSize.x, pos.y));
        e.addComponent<CPatrol>(v, s);
        e.addComponent<CGravity>(0.75);
    }

    afterSpawn(e, pos);
}

void Scene_Editor::sDragAndDrop() {
    if (m_sDnD.entity.has_value()) {
        m_sDnD.entity->getComponent<CTransform>().pos = m_sDnD.mousePos + (m_sDnD.startEntPos - m_sDnD.startMousePos);
    }
}

void Scene_Editor::saveLevel() {
    if (m_levelPath == "") {
        m_levelPath = "config/levels/user" + std::to_string(m_game->getSettings().levels.size()) + ".bin";
        auto& levels = m_game->getSettings().levels;
        levels.push_back(Settings::Level(m_levelPath, true, false, true)); // pass=true чтобы отображалось "You Win!" после прохождения основных уровней
        m_game->saveSettings();

        m_paths.push_back(levels[levels.size()-1].path.c_str());
        m_pathIndex = m_paths.size()-1;
    }

    Level::save(m_game->getExeDir() + m_levelPath, m_entityManager, m_bgColor);
}

void Scene_Editor::loadLevel(const std::string & path) {
    m_levelPath = path;
    Level::loadFromFile(m_game->getExeDir() + path, m_game, m_entityManager, m_bgColor);

    m_leftBottomOfLevel = {100000, -100000};
    for (auto e : m_entityManager.getEntities()) {
        auto pos = e.getComponent<CTransform>().pos;
        auto size = e.getSize() / 2.0;
        pos.x -= size.x;
        pos.y += size.y;

        if (pos.x < m_leftBottomOfLevel.x) m_leftBottomOfLevel.x = pos.x;
        if (pos.y > m_leftBottomOfLevel.y) m_leftBottomOfLevel.y = pos.y;
    }

    if (m_leftBottomOfLevel == Vec2(100000, -100000)) {
        m_leftBottomOfLevel = Vec2(0, m_game->getWindowSize().y);
    }

    auto wh = m_game->getWindowSize() / 2.0;
    auto vc = Vec2(m_leftBottomOfLevel.x + wh.x, m_leftBottomOfLevel.y - wh.y);

    m_game->setViewCenter(vc);

    m_scale = 0;
    m_game->setViewSize(computedViewSize());

    if (auto list = m_entityManager.getEntities("player"); list.size() > 0) {
        m_player = list.at(0);
    }
}

std::optional<std::string> Scene_Editor::selectFile() {
    if (ImGui::Button("OP", ImVec2(50, 50))) {
        m_paths.clear();
        for (auto& lvl : m_game->getSettings().levels) {
            if (lvl.path.ends_with(".bin")) {
                m_paths.push_back(lvl.path.c_str());
            }
        }

        ImGui::OpenPopup("select_file_popup");
    }
    ImGui::SetItemTooltip("Открыть уровень");

    ImGui::SameLine();
    if (m_pathIndex < 0) {
        ImGui::TextUnformatted("<None>");
    } else {
        auto fn = std::filesystem::path(std::string(m_paths[m_pathIndex])).filename().string();
        ImGui::TextUnformatted((const char*)fn.c_str());
    }
    
    bool selected = false;
    if (ImGui::BeginPopup("select_file_popup")) {
        ImGui::SeparatorText("Level");
        for (size_t i = 0; i < m_paths.size(); i++) {            
            if (ImGui::Selectable(m_paths.data()[i])) {
                m_pathIndex = i;
                selected = true;
            }
        }
        ImGui::EndPopup();
    }

    if (selected) {
        return m_paths[m_pathIndex];
    }

    return std::nullopt;
}

std::optional<Animation> Scene_Editor::selectAnimation(const Animation& animation) {
    int index = -1;
    std::string name = "";

    for (size_t i=0; i<m_animations.size(); i++) {
        if (m_animations[i].getName() == animation.getName()) {
            index = i;
            name = animation.getName();
            break;
        }
    }

    int currentIndex = index;

    ImGui::SetNextItemWidth(210.0f);
    if (ImGui::BeginCombo("##selectAnimation", name.c_str(), ImGuiComboFlags_HeightLarge)) {
        for (size_t n = 0; n < m_animations.size(); n++) {
            const bool is_selected = (index == (int)n);
            if (ImGui::Selectable(m_animations[n].getName().c_str(), is_selected)) index = n;
            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();

        if (index >= 0 && index != currentIndex) {
            return m_animations[index];
        }
    }

    return std::nullopt;
}

int Scene_Editor::selectTeleportLevel(int index) {
    auto levels = m_game->getSettings().levels;

    std::string fileName = (index >= 0 && index < (int)levels.size()) ? std::filesystem::path(levels[index].path).filename().string() : "";

    ImGui::SetNextItemWidth(210.0f);
    if (ImGui::BeginCombo("LVL##2", fileName.c_str(), ImGuiComboFlags_HeightLarge)) {
        for (size_t n = 0; n < m_game->getSettings().levels.size(); n++) {
            const bool is_selected = (index == (int)n);
            if (ImGui::Selectable(std::filesystem::path(levels[n].path).filename().string().c_str(), is_selected)) index = n;
            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    return index;
}

std::optional<std::string> Scene_Editor::selectArtefact(std::string & name) {
    bool changed = false;

    ImGui::SetNextItemWidth(210.0f);
    if (ImGui::BeginCombo("ART##ART2", name.c_str(), ImGuiComboFlags_HeightLarge)) {
        for (auto item : ArtefactsList) {
            const bool is_selected = (name == item);
            if (ImGui::Selectable(item.c_str(), is_selected)) {
                name = item;
                changed = true;
            }
            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (changed) {
        return name;
    }

    return std::nullopt;
}

std::vector<std::string> Scene_Editor::selectArtefacts(std::vector<std::string> names) {
    ImGui::SetNextItemWidth(210.0f);
    ImGui::SetNextItemOpen(true);

    if (ImGui::TreeNode("ARTS")) {
        static bool selected[50];
        for (size_t i=0; i < ArtefactsList.size(); i++) {
            selected[i] = std::find(names.begin(), names.end(), ArtefactsList[i]) != names.end();
        }

        for (size_t i=0; i < ArtefactsList.size(); i++) {
            auto item = ArtefactsList[i];

            if (ImGui::Checkbox(item.c_str(), &selected[i])) {
                if (selected[i]) {
                    names.push_back(item);
                } else {
                    names.erase(std::remove(names.begin(), names.end(), item), names.end());
                }
            }
        }

        ImGui::TreePop();
    }

    return names;
}

bool Scene_Editor::CollapsingHeaderWithColor(const std::string & header, ImVec4 color) {
    ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    auto open = ImGui::CollapsingHeader(header.c_str());
    ImGui::PopStyleColor();
    return open;
}

bool Scene_Editor::InputFloat2(const std::string & name, Vec2& p, float step, float fastStep, const std::string & format, ImGuiInputTextFlags flags) {
    float width = (step == 0 && fastStep == 0) ? 100 : 110;
    bool changed = false;

    ImGui::SetNextItemWidth(width);
    if (ImGui::InputFloat(("##"+name+"x").c_str(), &p.x, step, fastStep, format.c_str(), flags)) {
        changed = true;
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(width);
    if (ImGui::InputFloat(name.c_str(), &p.y, step, fastStep, format.c_str(), flags)) {
        changed = true;
    }

    return changed;
}

void Scene_Editor::sGUI() {
    ImGui::Begin("Editor##Editor", nullptr, ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoNav|ImGuiWindowFlags_NoBackground);
    ImGui::SetWindowPos(Vec2(0,0));
    ImGui::SetWindowSize(Vec2(m_game->getWindowSize().x, 64));
    //ImGui::PushItemWidth(m_game->window().getSize().x * 0.5);

    if (ImGui::Button("RC", ImVec2(50, 50))) {
        m_bgColor = sf::Color(0x97, 0xAD, 0xB7, 0x51);
    }
    ImGui::SetItemTooltip("Сбросить цвет фона");

    ImGui::SameLine();
    ImGui::BeginGroup();
    static ImVec4 bgColor;
    bgColor.x = (float)m_bgColor.r/255.0;
    bgColor.y = (float)m_bgColor.g/255.0;
    bgColor.z = (float)m_bgColor.b/255.0;
    bgColor.w = (float)m_bgColor.a/255.0;

    if (ImGui::ColorEdit4("bgColor##3", (float*)&bgColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf)) {
        m_bgColor = sf::Color(bgColor.x * 255, bgColor.y * 255, bgColor.z * 255, bgColor.w * 255);
    };
    ImGui::SameLine();
    ImGui::Text("Фон");
    ImGui::Checkbox("BB", &m_drawBoundingBox);
    ImGui::SetItemTooltip("Рисовать границы объекта");
    ImGui::EndGroup();

    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::Checkbox("Сетка", &m_drawGrid);
    ImGui::Checkbox("Поверх", &m_gridAbove);
    ImGui::SetItemTooltip("Отображать сетку поверх объектов");
    ImGui::EndGroup();

    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::SameLine();
    if (ImGui::Button("NEW", ImVec2(50, 50))) {
        EntityMemoryPool::Instance().clear();
        m_entityManager = EntityManager();
        m_pathIndex = -1;
        m_scale = 0;
        auto wh = m_game->getWindowSize();
        m_leftBottomOfLevel = Vec2(0, wh.y);
        auto vc = m_leftBottomOfLevel;
        vc.x += wh.x / 2.0;
        vc.y -= wh.y / 2.0;
        storeViewCenter(vc);
        m_game->setViewCenter(vc);
        m_levelPath = "";
        m_selectedEntity = std::nullopt;
    }
    ImGui::SetItemTooltip("Новый уровень");

    ImGui::SameLine();
    if (auto path = selectFile(); path.has_value()) {
        m_selectedEntity = std::nullopt;
        loadLevel(path.value());
    }

    ImGui::SameLine();
    if (ImGui::Button("SV", ImVec2(50, 50))) {
        saveLevel();
    }
    ImGui::SetItemTooltip("Сохранить уровень");

    ImGui::SameLine();
    if (ImGui::Button("RUN", ImVec2(50, 50))) {
        saveLevel();
        for (auto& lvl : m_game->getSettings().levels) {
            if (lvl.path == m_levelPath) {
                EntityMemoryPool::Instance().clear();
                m_entityManager = EntityManager();

                if (lvl.world) {
                    m_game->changeScene("PLAY", std::make_shared<Scene_World>(m_game, lvl.path), true);
                } else {
                    m_game->changeScene("PLAY", std::make_shared<Scene_Play>(m_game, lvl), true);
                }
                m_game->assets().getSound("MusicTitle")->pause();
                break;
            }
        }
    }
    ImGui::SetItemTooltip("Запустить уровень");

    ImGui::SameLine();
    if (ImGui::Button("DEL", ImVec2(50, 50))) {
        if (m_levelPath != "") {
            if (auto lvl = m_game->getSettings().getLevelByPath(m_levelPath); lvl.has_value() && lvl->userLevel) {
                ImGui::OpenPopup("Удаление уровня##yes");
            } else {
                ImGui::OpenPopup("Удаление уровня##no");
            }
        }
    }
    ImGui::SetItemTooltip("Удалить уровень");

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    bool deleteLevel = false;
    if (ImGui::BeginPopupModal("Удаление уровня##yes", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Удалить уровень?");
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) { deleteLevel = true; ImGui::CloseCurrentPopup(); }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    if (deleteLevel) {
        std::erase_if(m_game->getSettings().levels, [this](const Settings::Level & lvl){
            return lvl.path == m_levelPath;
        });
        m_game->saveSettings();

        auto path = m_game->getExeDir() + m_levelPath;
        if (std::filesystem::exists(path)) {
            std::filesystem::remove(path);
        }

        EntityMemoryPool::Instance().clear();
        m_entityManager = EntityManager();
        m_levelPath = "";
        m_pathIndex = -1;
        m_selectedEntity = std::nullopt;
        //m_playerConfig
    }

    if (ImGui::BeginPopupModal("Удаление уровня##no", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Нельзя удалить системный уровень");
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::SetItemDefaultFocus();
        ImGui::EndPopup();
    }

    ImGui::EndGroup();

    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::SameLine();
    if (ImGui::Button("HM", ImVec2(50, 50))) {
        auto vc = m_leftBottomOfLevel;
        vc.x += computedViewSize().x / 2.0;
        vc.y -= computedViewSize().y / 2.0;
        storeViewCenter(vc);
        m_game->setViewCenter(vc);
    }
    ImGui::SetItemTooltip("Вернуться домой");

    ImGui::SameLine();
    if (ImGui::Button("RS", ImVec2(50, 50))) {
        m_scale = 0;
        m_game->setViewSize(computedViewSize());
    }
    ImGui::SetItemTooltip("Сбросить масштабирование");
    
    ImGui::SameLine();
    if (ImGui::Button("PLR", ImVec2(50, 50))) {
        spawnPlayer();
    }
    ImGui::SetItemTooltip("Добавить игрока");

    ImGui::SameLine();
    if (ImGui::Button("DEC", ImVec2(50, 50))) {
        spawnDec();
    }
    ImGui::SetItemTooltip("Добавить декорацию");

    ImGui::SameLine();
    if (ImGui::Button("TL", ImVec2(50, 50))) {
        spawnTile();
    }
    ImGui::SetItemTooltip("Добавить тайл");

    ImGui::SameLine();
    if (ImGui::Button("MTL", ImVec2(50, 50))) {
        spawnMTile();
    }
    ImGui::SetItemTooltip("Добавить двигающийся тайл");

    ImGui::SameLine();
    if (ImGui::Button("FNPC", ImVec2(50, 50))) {
        spawnNPC("Follow");
    }
    ImGui::SetItemTooltip("Добавить преследующего NPC");

    ImGui::SameLine();
    if (ImGui::Button("PNPC", ImVec2(50, 50))) {
        spawnNPC("Patrol");
    }
    ImGui::SetItemTooltip("Добавить патрульного NPC");

    ImGui::SameLine();
    ImGui::Button("???", ImVec2(50, 50));
    ImGui::SetItemTooltip(
        "LEFT_CLICK         - переместить с привязкой к сетке\n"
        "SHIFT + LEFT_CLICK - переместить без привязки\n"
        "MIDDLE_CLICK       - удалить элемент\n"
        "RIGHT_CLICK        - переместить мир\n"
        "SCROLL             - изменить масштаб\n"
        "ARROWS             - навигация по миру\n"
        "SHIFT + ARROWS     - быстрая наваигация по миру\n"
        "CTRL+C             - скопировать элемент\n"
        "CTRL+V             - вставить скопированный элемент\n"
        "В окне редактирования свойств нажатие CMD меняет шаг для кнопок +/-"
    );

    ImGui::EndGroup();

    ImGui::End();

    // ---------------------------------------------------------------------------------------------------
    // ---------------------------------------------------------------------------------------------------

    if (m_selectedEntity.has_value()) {
        auto wh = m_game->getWindowSize();

        ImGui::SetNextWindowBgAlpha(0.5f);
        ImGui::SetNextWindowSizeConstraints(Vec2(300, 10), Vec2(300, wh.y-6));
        ImGui::Begin("Editor##EntProps", nullptr, ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoNav|ImGuiWindowFlags_AlwaysAutoResize); // |ImGuiWindowFlags_NoBackground

        ImGui::SetWindowPos(Vec2(wh.x-300-3,3));

        auto e = m_selectedEntity.value();

        auto header = e.tag() + "(" + std::to_string(e.id()) + ")";
        if (e.hasComponent<CPatrol>()) header = "M" + header;
        ImGui::SeparatorText(header.c_str());

        auto collapsingHeaderTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        auto posChanged = false;
        Vec2 oldPos;

        if (e.hasComponent<CTransform>()) {
            if (CollapsingHeaderWithColor("Transform", collapsingHeaderTextColor)) {
                auto& c = e.getComponent<CTransform>();
                oldPos = c.pos;
                posChanged = InputFloat2("Pos", c.pos, 64.0f, 4.0f, "%.0f");
                ImGui::SetNextItemWidth(150.0f);
                if (ImGui::InputFloat("Z", &c.z, 1.0f, 0.1f, "%.1f")) {
                    m_entityManager.sortEntitiesByZ();
                }
                InputFloat2("Scale", c.scale, 1.0f, 0.1f, "%.1f");
            }
        }

        if (e.hasComponent<CAnimation>()) {
            if (CollapsingHeaderWithColor("Animation", collapsingHeaderTextColor)) {
                auto& c = e.getComponent<CAnimation>();
                if (auto a = selectAnimation(c.animation); a.has_value()) {
                    c.animation = a.value();
                    if (e.hasComponent<CBoundingBox>()) {
                        e.getComponent<CBoundingBox>().size = a.value().frameSize();
                        e.getComponent<CBoundingBox>().halfSize = a.value().frameSize() / 2.0;
                    }
                }
                
                ImGui::Checkbox("Repeat", &c.repeat);
            }
        }

        if (e.hasComponent<CBoundingBox>()) {
            if (CollapsingHeaderWithColor("Bounding Box", collapsingHeaderTextColor)) {
                auto& c = e.getComponent<CBoundingBox>();
                InputFloat2("BB", c.size, 1.0, 10.0);
                c.halfSize = c.size / 2.0;
                ImGui::SetNextItemWidth(150.0f);
                ImGui::Checkbox("Block Move", &c.blockMove);
                ImGui::SetNextItemWidth(150.0f);
                ImGui::Checkbox("Block Vision", &c.blockVision);
            }
        }

        if (e.hasComponent<CDamage>()) {
            if (CollapsingHeaderWithColor("Damage", collapsingHeaderTextColor)) {
                auto& c = e.getComponent<CDamage>();
                ImGui::SetNextItemWidth(150.0f);
                ImGui::InputFloat("Damage##2", &c.damage, 1.0f, 0.1f, "%.1f");
            }
        }

        if (e.hasComponent<CHealth>()) {
            if (CollapsingHeaderWithColor("Health", collapsingHeaderTextColor)) {
                auto& c = e.getComponent<CHealth>();
                ImGui::SetNextItemWidth(150.0f);
                ImGui::InputFloat("Max", &c.max, 1.0f, 0.1f, "%.1f");
                c.current = c.max;
            }
        }

        if (e.hasComponent<CGravity>()) {
            if (CollapsingHeaderWithColor("Gravity", collapsingHeaderTextColor)) {
                auto& c = e.getComponent<CGravity>();
                ImGui::SetNextItemWidth(150.0f);
                ImGui::InputFloat("Gravity##2", &c.gravity, 0.1f, 0.01f, "%.2f");
            }
        }

        if (e.hasComponent<CFollowPlayer>()) {
            if (posChanged) {
                auto& c = e.getComponent<CFollowPlayer>();
                c.home = e.getComponent<CTransform>().pos;
            }

            if (CollapsingHeaderWithColor("Follow Player", collapsingHeaderTextColor)) {
                auto& c = e.getComponent<CFollowPlayer>();
                c.home = e.getComponent<CTransform>().pos;
                InputFloat2("Home", c.home, 0, 0, "%.0f", ImGuiInputTextFlags_ReadOnly);
                ImGui::SetNextItemWidth(150.0f);
                ImGui::InputFloat("Speed", &c.speed, 1.0f, 0.1f, "%.1f");
                ImGui::SetNextItemWidth(150.0f);
                ImGui::InputFloat("Vision", &c.visionRadius, 100.0f, 10.0f, "%.0f");
            }
        }

        if (e.hasComponent<CPatrol>()) {
            if (posChanged) {
                auto& c = e.getComponent<CPatrol>();
                auto pos = e.getComponent<CTransform>().pos;

                for(size_t i=0; i<c.positions.size(); i++) {
                    c.positions[i] += pos - oldPos;
                }
            }

            if (CollapsingHeaderWithColor("Patrol", collapsingHeaderTextColor)) {
                auto& c = e.getComponent<CPatrol>();
                ImGui::SetNextItemWidth(150.0f);
                ImGui::InputFloat("Speed##2", &c.speed, 1.0f, 0.1f, "%.1f");
                
                int deletePoint = -1;
                for(size_t i=0; i<c.positions.size(); i++) {
                    InputFloat2("P"+std::to_string(i), c.positions[i], 64.0f, 4.0f);
                    ImGui::SameLine();
                    if (ImGui::Button(("X##"+std::to_string(i)).c_str())) {
                        deletePoint = i;
                    }
                }

                if (deletePoint >= 0) {
                    c.positions.erase(c.positions.begin() + deletePoint);
                }

                if (ImGui::Button("Add")) {
                    Vec2 p = {0, 0};

                    if (c.positions.size() > 0) {
                        p = c.positions[c.positions.size()-1];
                        p.x += m_gridSize.x;
                    } else {
                        p = e.getComponent<CTransform>().pos;
                    }

                    c.positions.push_back(p);
                }
            }
        }

        if (e.hasComponent<CTeleport>()) {
            if (CollapsingHeaderWithColor("Teleport", collapsingHeaderTextColor)) {
                auto& c = e.getComponent<CTeleport>();
                c.level = selectTeleportLevel(c.level);
            }
        }

        if (e.hasComponent<CParallax>()) {
            if (CollapsingHeaderWithColor("Parallax", collapsingHeaderTextColor)) {
                auto& c = e.getComponent<CParallax>();
                InputFloat2("StartPos", c.startPos, 0, 0, "%.0f", ImGuiInputTextFlags_ReadOnly);
                ImGui::SetNextItemWidth(150.0f);
                ImGui::InputInt("Repeat##par", &c.repeat, 1, 10);
            }
        }

        if (e.hasComponent<CArtefact>()) {
            if (CollapsingHeaderWithColor("Artefact", collapsingHeaderTextColor)) {
                auto& c = e.getComponent<CArtefact>();
                if (auto r = selectArtefact(c.artefact); r.has_value()) {
                    c.artefact = r.value();
                }
            }
        }

        if (e.hasComponent<CArtefactSpawner>()) {
            if (CollapsingHeaderWithColor("Artefact spawner", collapsingHeaderTextColor)) {
                auto& c = e.getComponent<CArtefactSpawner>();
                c.artefacts = selectArtefacts(c.artefacts);
            }
        }

        if (auto title = e.hasComponent<CTeleport>() ? "Delete teleport" : "Add teleport"; ImGui::Button(title)) {
            if (e.hasComponent<CTeleport>()) {
                e.removeComponent<CTeleport>();
            } else {
                e.addComponent<CTeleport>(-1);
            }            
        }

        if (auto title = e.hasComponent<CParallax>() ? "Delete parallax" : "Add parallax"; ImGui::Button(title)) {
            if (e.hasComponent<CParallax>()) {
                e.removeComponent<CParallax>();
            } else {
                e.addComponent<CParallax>(e.getComponent<CTransform>().pos, 1);
            }            
        }

        if (auto title = e.hasComponent<CArtefact>() ? "Delete artefact" : "Add artefact"; ImGui::Button(title)) {
            if (e.hasComponent<CArtefact>()) {
                e.removeComponent<CArtefact>();
            } else {
                e.addComponent<CArtefact>();
            }            
        }

        if (auto title = e.hasComponent<CArtefactSpawner>() ? "Delete artefact spawner" : "Add artefact spawner"; ImGui::Button(title)) {
            if (e.hasComponent<CArtefactSpawner>()) {
                e.removeComponent<CArtefactSpawner>();
            } else {
                e.addComponent<CArtefactSpawner>();
            }            
        }

        ImGui::End();
    }
}

void Scene_Editor::update() {
    m_entityManager.update();
    sDragAndDrop();
    sGUI();
    sRender();
    m_currentFrame++;
}

void Scene_Editor::sDoAction(const Action& action) {
    if (action.type() == "START") {
             if (action.name() == "QUIT")           { m_game->changeScene("MENU", nullptr); }
        else if (action.name() == "TOGGLE_GRID")    { m_drawGrid = !m_drawGrid; }
        else if (action.name() == "TOGGLE_BB")      { m_drawBoundingBox = !m_drawBoundingBox; }

        else if (action.name() == "MOVE_UP")        { m_game->moveView(Vec2(0, -m_gridSize.y)); }
        else if (action.name() == "MOVE_DOWN")      { m_game->moveView(Vec2(0, m_gridSize.y)); }
        else if (action.name() == "MOVE_LEFT")      { m_game->moveView(Vec2(-m_gridSize.x, 0)); }
        else if (action.name() == "MOVE_RIGHT")     { m_game->moveView(Vec2(m_gridSize.x, 0)); }

        else if (action.name() == "FAST_MOVE_UP")    { m_game->moveView(Vec2(0, -m_game->getWindowSize().y)); }
        else if (action.name() == "FAST_MOVE_DOWN")  { m_game->moveView(Vec2(0, m_game->getWindowSize().y)); }
        else if (action.name() == "FAST_MOVE_LEFT")  { m_game->moveView(Vec2(-m_game->getWindowSize().x, 0)); }
        else if (action.name() == "FAST_MOVE_RIGHT") { m_game->moveView(Vec2(m_game->getWindowSize().x, 0)); }

        else if (action.name() == "WINDOW_RESIZED") {
            if (m_entityManager.getEntities().size() == 0) {
                m_leftBottomOfLevel = Vec2(0, m_game->getWindowSize().y);
            }
        } else if (action.name() == "LEFT_CLICK") {
            auto mp = action.pos();

            if (m_sDnD.entity.has_value()) {
                return;
            }

            int z = -10000;
            m_selectedEntity = std::nullopt;
            for (auto e : m_entityManager.getEntities()) {
                if (Physics::IsInside(mp, e)) {
                    auto t = e.getComponent<CTransform>();
                    if (t.z > z) {
                        z = t.z;
                        m_sDnD.entity = e;
                        m_sDnD.mousePos = action.pos();
                        m_sDnD.startMousePos = action.pos();
                        m_sDnD.startEntPos = t.pos;
                        m_selectedEntity = e;
                    }
                }
            }
        } else if (action.name() == "MIDDLE_CLICK") {
            auto mp = action.pos();

            int z = -10000;
            m_selectedEntity = std::nullopt;
            for (auto e : m_entityManager.getEntities()) {
                if (Physics::IsInside(mp, e)) {
                    auto t = e.getComponent<CTransform>();
                    if (t.z > z) {
                        z = t.z;
                        m_selectedEntity = e;
                    }
                }
            }
            if (m_selectedEntity.has_value()) {
                m_selectedEntity->destroy();
                m_selectedEntity = std::nullopt;
            }
        } else if (action.name() == "RIGHT_CLICK") {
            m_sDnD.startMousePos = action.screenPos();
            m_sDnD.startViewPos = m_game->getViewCenter();
            m_sDnD.moveCanvas = true;
        } else if (action.name() == "MOUSE_MOVE") {
            if (m_sDnD.moveCanvas) {
                auto mx = computedViewSize().x / m_baseViewSize.x;
                auto my = computedViewSize().y / m_baseViewSize.y;

                auto delta = m_sDnD.startMousePos - action.screenPos();
                delta.x *= mx;
                delta.y *= my;
                
                m_game->setViewCenter(m_sDnD.startViewPos + delta);
            }

            if (m_sDnD.entity.has_value()) {
                m_sDnD.mousePos = action.pos();
            }
        } else if (action.name() == "MOUSE_SCROLL") {
            m_scale -= action.pos().x;
            m_game->setViewSize(computedViewSize());
        } else if (action.name() == "SELECT_TO_COPY") {
            if (m_selectedEntity.has_value() && m_entityForCopy->tag() != "player") {
                m_entityForCopy = m_selectedEntity;
            }
        } else if (action.name() == "PASTE_ENTITY") {
            if (m_entityForCopy.has_value()) {
                copyEntity(m_entityForCopy.value());
            }
        }
    } else if (action.type() == "END") {
             if (action.name() == "MOVE_UP")    { m_player.getComponent<CInput>().up = false; }
        else if (action.name() == "MOVE_DOWN")  { m_player.getComponent<CInput>().down = false; }
        else if (action.name() == "MOVE_LEFT")  { m_player.getComponent<CInput>().left = false; }
        else if (action.name() == "MOVE_RIGHT") { m_player.getComponent<CInput>().right = false; }
        else if (action.name() == "LEFT_CLICK") {
            if (m_sDnD.moveCanvas) {
                m_sDnD.moveCanvas = false;
                return;
            }

            if (m_sDnD.entity.has_value()) {
                // Ставим тайл в ячейку
                auto pos = m_sDnD.mousePos + (m_sDnD.startEntPos - m_sDnD.startMousePos);

                if (!ImGui::GetIO().KeyShift) {
                    auto halfSize = m_sDnD.entity->getSize() / 2.0f;
                    if (halfSize != m_gridSize / 2.0) {
                        pos -= halfSize;
                    }

                    auto shift = pos % m_gridSize;
                    if (shift.x < 0) shift.x += m_gridSize.x;
                    if (shift.y < 0) shift.y += m_gridSize.y;

                    pos -= shift;
                    pos.x = (pos.x < 0) ? ceil(pos.x) : floor(pos.x);
                    pos.y = (pos.y < 0) ? ceil(pos.y) : floor(pos.y);

                    pos += halfSize;
                }

                m_sDnD.entity->getComponent<CTransform>().pos = pos;

                if (m_sDnD.entity->hasComponent<CFollowPlayer>()) {
                    m_sDnD.entity->getComponent<CFollowPlayer>().home = pos;
                }

                if (m_sDnD.entity->hasComponent<CPatrol>()) {
                    auto delta = m_sDnD.entity->getComponent<CTransform>().pos - m_sDnD.startEntPos;
                    auto& c = m_sDnD.entity->getComponent<CPatrol>();

                    for (auto& p : c.positions) {
                        p += delta;
                    }
                }

                if (m_sDnD.entity->hasComponent<CParallax>()) {
                    m_sDnD.entity->getComponent<CParallax>().startPos = pos;
                }

                m_sDnD.entity = std::nullopt;
            }
        } else if (action.name() == "RIGHT_CLICK") {
            if (m_sDnD.moveCanvas) {
                m_sDnD.moveCanvas = false;
            }
        }
    }
}

void Scene_Editor::sRender() {
    m_game->window().clear(m_bgColor);
    m_game->getVertexArrays().clearAll();

    if (m_drawGrid && !m_gridAbove) {
        drawGrid();
    }

    sf::CircleShape dot(4);
    dot.setOrigin(Vec2(4, 4));
    dot.setPosition(Vec2(0, 0));
    dot.setFillColor(sf::Color::Red);
    m_game->window().draw(dot);

    for (auto e : m_entityManager.getEntities()) {
        auto & transform = e.getComponent<CTransform>();

        if (e.hasComponent<CAnimation>()) {
            auto& animation = e.getComponent<CAnimation>().animation;
            animation.getSprite().setRotation(sf::degrees(transform.angle));
            animation.getSprite().setScale({ transform.scale.x, transform.scale.y });

            int repeat = (e.hasComponent<CParallax>()) ? e.getComponent<CParallax>().repeat : 1;
            for (int i=0; i<repeat; i++) {
                animation.getSprite().setPosition({ transform.pos.x + i * e.getSize().x, transform.pos.y });
                m_game->getVertexArrays().draw(animation.getSprite());
            }
        }
    }

    m_game->getVertexArrays().draw(m_game->window(), "", m_game->assets().getLargeTexture());

    // Рисуем точки патрулирования и базовую точку преследующего NPC
    for (auto e : m_entityManager.getEntities()) {
        if (e.hasComponent<CPatrol>()) {
            for (auto p : e.getComponent<CPatrol>().positions) {
                dot.setPosition(sf::Vector2f(p.x, p.y));
                dot.setFillColor(sf::Color::Black);
                m_game->window().draw(dot);
            }
        }

        if (e.hasComponent<CFollowPlayer>()) {
            auto p = e.getComponent<CFollowPlayer>().home;
            dot.setPosition(sf::Vector2f(p.x, p.y));
            dot.setFillColor(sf::Color::Black);
            m_game->window().draw(dot);
        }
    }

    if (m_drawGrid && m_gridAbove) {
        drawGrid();
    }

    if (m_drawBoundingBox) {
        sf::RectangleShape rect;
        rect.setFillColor(sf::Color(0, 0, 0, 0));
        rect.setOutlineThickness(1);

        for (auto e : m_entityManager.getEntities()) {
            if (e.hasComponent<CBoundingBox>()) {
                auto blockMove = e.getComponent<CBoundingBox>().blockMove;
                auto blockVision = e.getComponent<CBoundingBox>().blockVision;
                auto box = e.getBoundingBox();
                auto & transform = e.getComponent<CTransform>();

                rect.setSize(sf::Vector2f(box.x-1, box.y-1));
                rect.setOrigin(sf::Vector2f(box.x / 2.0, box.y / 2.0));
                rect.setPosition({transform.pos.x, transform.pos.y});

                if (blockMove && blockVision) { rect.setOutlineColor(sf::Color(0,255,255)); }
                if (blockMove && !blockVision) { rect.setOutlineColor(sf::Color(0,255,0)); }
                if (!blockMove && blockVision) { rect.setOutlineColor(sf::Color(0,0,255)); }
                if (!blockMove && !blockVision) { rect.setOutlineColor(sf::Color(0,0,0)); }

                m_game->window().draw(rect);
            }
        }
    }
}

void Scene_Editor::drawGrid() {
    auto vs = computedViewSize();
    auto tl = m_game->getViewCenter() - vs / 2.0 + 2.0;
    auto br = tl + vs - 4.0;

    auto nextGrid = tl - tl % m_gridSize;
    for (float x = nextGrid.x; x < br.x; x += m_gridSize.x) {
        auto c = ((int)x % ((int)m_gridSize.x*10) == 0) ? sf::Color::Red : sf::Color::Black;
        drawLine(Vec2(x, tl.y), Vec2(x, br.y), c);
    }

    for (float y = nextGrid.y; y < br.y; y += m_gridSize.y) {
        auto cond = ((int)y % ((int)m_gridSize.y*10) == 0);
        auto c = cond ? sf::Color::Red : sf::Color::Black;
        drawLine(Vec2(tl.x, y), Vec2(br.x, y), c);
        
        std::string yCell = std::to_string((int)y / (int)m_gridSize.y);

        if (!cond) {
            continue;
        }

        for (float x = nextGrid.x; x < br.x; x += m_gridSize.x) {
            if ((int)x % ((int)m_gridSize.x*10) != 0) {
                continue;
            }

            std::string xCell = std::to_string((int)x / (int)m_gridSize.x);
            m_gridText.setString("(" + xCell + "," + yCell + ")");
            m_gridText.setPosition({x + 3.0f, y + 2.0f});
            m_game->window().draw(m_gridText);
        }
    }
}

void Scene_Editor::onResume(const std::string & prevScene) {
    if (auto cursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Hand); cursor.has_value()) {
        m_game->window().setMouseCursor(cursor.value());
    }
}

void Scene_Editor::onPause(const std::string & nextScene) {
    if (auto cursor = sf::Cursor::createFromSystem(sf::Cursor::Type::Arrow); cursor.has_value()) {
        m_game->window().setMouseCursor(cursor.value());
    }
}

Entity Scene_Editor::copyEntity(const Entity & o) {
    auto e = m_entityManager.addEntity(o.tag());

    if (o.hasComponent<CTransform>()) {
        auto src = o.getComponent<CTransform>();
        auto& dst = e.addComponent<CTransform>();
        dst.pos = src.pos + m_gridSize;
        dst.prevPos = src.prevPos + m_gridSize;
        dst.scale = src.scale;
        dst.velocity = src.velocity;
        dst.move = src.move;
        dst.angle = src.angle;
        dst.z = src.z;
    }

    if (o.hasComponent<CLifespan>()) {
        auto src = o.getComponent<CLifespan>();
        auto& dst = e.addComponent<CLifespan>();
        dst.lifespan = src.lifespan;
        dst.frameCreated = src.frameCreated;
    }

    if (o.hasComponent<CDamage>()) {
        auto src = o.getComponent<CDamage>();
        auto& dst = e.addComponent<CDamage>();
        dst.damage = src.damage;
    }

    if (o.hasComponent<CInvincibility>()) {
        auto src = o.getComponent<CInvincibility>();
        auto& dst = e.addComponent<CInvincibility>();
        dst.iframes = src.iframes;
    }

    if (o.hasComponent<CHealth>()) {
        auto src = o.getComponent<CHealth>();
        auto& dst = e.addComponent<CHealth>();
        dst.max = src.max;
        dst.current = src.current;
    }

    if (o.hasComponent<CInput>()) {
        auto src = o.getComponent<CInput>();
        auto& dst = e.addComponent<CInput>();
        dst.up = src.up;
        dst.down = src.down;
        dst.left = src.left;
        dst.right = src.right;
        dst.shoot = src.shoot;
    }

    if (o.hasComponent<CBoundingBox>()) {
        auto src = o.getComponent<CBoundingBox>();
        auto& dst = e.addComponent<CBoundingBox>();
        dst.size = src.size;
        dst.halfSize = src.halfSize;
        dst.blockMove = src.blockMove;
        dst.blockVision = src.blockVision;
    }

    if (o.hasComponent<CAnimation>()) {
        auto src = o.getComponent<CAnimation>();
        auto& dst = e.addComponent<CAnimation>();
        dst.repeat = src.repeat;
        dst.animation = m_game->assets().getAnimation(src.animation.getName());
    }

    if (o.hasComponent<CGravity>()) {
        auto src = o.getComponent<CGravity>();
        auto& dst = e.addComponent<CGravity>();
        dst.gravity = src.gravity;
    }

    if (o.hasComponent<CState>()) {
        auto src = o.getComponent<CState>();
        auto& dst = e.addComponent<CState>();
        dst.direction = src.direction;
        dst.onAir = src.onAir;
        dst.run = src.run;
    }

    if (o.hasComponent<CFollowPlayer>()) {
        auto src = o.getComponent<CFollowPlayer>();
        auto& dst = e.addComponent<CFollowPlayer>();
        dst.home = src.home + m_gridSize;
        dst.speed = src.speed;
        dst.visionRadius = src.visionRadius;
    }

    if (o.hasComponent<CPatrol>()) {
        auto src = o.getComponent<CPatrol>();
        auto& dst = e.addComponent<CPatrol>();
        dst.currentPosition = src.currentPosition;
        dst.speed = src.speed;
        for (auto p : src.positions) {
            dst.positions.push_back(p + m_gridSize);
        }
    }

    if (o.hasComponent<CDraggable>()) {
        auto src = o.getComponent<CDraggable>();
        auto& dst = e.addComponent<CDraggable>();
        dst.dragging = src.dragging;
    }

    if (o.hasComponent<CParallax>()) {
        auto src = o.getComponent<CParallax>();
        auto& dst = e.addComponent<CParallax>();
        dst.startPos = src.startPos;
        dst.repeat = src.repeat;
    }

    if (o.hasComponent<CTeleport>()) {
        auto src = o.getComponent<CTeleport>();
        auto& dst = e.addComponent<CTeleport>();
        dst.level = src.level;
    }

    if (o.hasComponent<CArtefact>()) {
        auto src = o.getComponent<CArtefact>();
        auto& dst = e.addComponent<CArtefact>();
        dst.artefact = src.artefact;
    }

    if (o.hasComponent<CArtefactSpawner>()) {
        auto src = o.getComponent<CArtefactSpawner>();
        auto& dst = e.addComponent<CArtefactSpawner>();
        for (auto item : src.artefacts) {
            dst.artefacts.push_back(item);
        }
    }

    return e;
}
