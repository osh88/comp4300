#include "Scene_Settings.h"
#include "Scene_Menu.h"
#include "Scene_Play.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include <iostream>

Scene_Settings::Scene_Settings(GameEngine* gameEngine)
    : Scene(gameEngine)
    , m_help(m_game->assets().getFont("Mario").font, "", 20)
{}

void Scene_Settings::init() {
    storeViewCenter(m_game->getWindowSize() / 2.0);

    registerAction("QUIT", sf::Keyboard::Key::Escape);

    m_help.setString("Escape - quit\nCtrl+P - pause\nCtrl+R - reload\nCtrl+T - toggle texture\nCtrl+D - toggle debug\nCtrl+G - toggle grid\nCtrl+S - toggle slow\nCtrl+F - toggle fullscreen\nCtrl+M - mute\nCtrl+X - screenshot");
    m_help.setPosition(Vec2(20, m_game->getWindowSize().y - m_help.getGlobalBounds().size.y - 20));

    for (size_t i=0; i<keys.size(); i++) {
        m_sGUI.keysToIdx.emplace(keys[i].k, i);
    }

    for (auto& [a, item] : m_game->getSettings().keys) {
        item.index = m_sGUI.keysToIdx[item.key];
        m_sGUI.imguiFilters.emplace(a, ImGuiTextFilter());
    }

    m_oldFullScreenValue = m_game->getSettings().fullScreen;
}

void Scene_Settings::sGUI() {
    ImGui::Begin("Settings##Settings", nullptr, ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoNav|ImGuiWindowFlags_NoBackground);
    ImGui::SetWindowPos(ImVec2(0,0));
    ImGui::SetWindowSize(ImVec2(m_game->window().getSize().x, m_game->window().getSize().y));

    ImGui::PushItemWidth(m_game->window().getSize().x * 0.5);
    
    ImGui::Spacing();
    ImGui::SeparatorText("Основные");
    ImGui::Spacing();

    ImGui::BeginGroup();

    ImGui::InputText("Путь к файлу ресурсов", &m_game->getSettings().assetsPath);

    ImGui::SliderInt("Громкость музыки", &m_game->getSettings().musicVolume, 0, 100, "%d", ImGuiSliderFlags_NoInput);
    ImGui::SliderInt("Громкость эффектов", &m_game->getSettings().effectsVolume, 0, 100, "%d", ImGuiSliderFlags_NoInput);

    const char* elems_names[3] = { "Низкая", "Средняя", "Высокая" };
    const char* elem_name = (m_game->getSettings().difficulty >= 0 && m_game->getSettings().difficulty < 3) ? elems_names[m_game->getSettings().difficulty] : "Unknown";
    ImGui::SliderInt("Сложность", (int *)&m_game->getSettings().difficulty, 0, 2, elem_name);

    ImGui::Checkbox("Полноэкранный режим", &m_game->getSettings().fullScreen);
    ImGui::SameLine();
    ImGui::Checkbox("Без звука", &m_game->getSettings().mute);
    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::SeparatorText("Игрок");
    ImGui::Spacing();

    ImGui::BeginGroup();
    ImGui::InputFloat("Базовая скорость", &m_game->getSettings().playerConfig.speed, 1.0f, 0.1f, "%.1f");
    ImGui::InputFloat("Максимальная скорость", &m_game->getSettings().playerConfig.maxSpeed, 1.0f, 0.1f, "%.1f");
    ImGui::InputFloat("Скорость прыжка", &m_game->getSettings().playerConfig.jumpSpeed, 1.0f, 0.1f, "%.1f");
    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::SeparatorText("Управление");
    ImGui::Spacing();

    ImGui::BeginGroup();

    auto& sKeys = m_game->getSettings().keys;

    for (auto& [a, item] : sKeys) {
        if (ImGui::BeginCombo(item.name.c_str(), keys[item.index].n.c_str(), ImGuiComboFlags_HeightRegular)) {
            auto& filter = m_sGUI.imguiFilters[item.action];

            if (ImGui::IsWindowAppearing()) {
                ImGui::SetKeyboardFocusHere();
                filter.Clear();
            }

            ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F);
            filter.Draw("##Filter", -FLT_MIN);

            for (size_t n = 0; n < keys.size(); n++)
            {
                if (filter.PassFilter(keys[n].n.c_str()) && ImGui::Selectable(keys[n].n.c_str(), item.index == n)) {
                    item.key = keys[n].k;
                    item.index = n;
                }
            }

            ImGui::EndCombo();
        }
    }

    ImGui::EndGroup();
    ImGui::PopItemWidth();

    ImGui::End();
}

void Scene_Settings::update() {
    sGUI();
    sRender();
    m_currentFrame++;
}

void Scene_Settings::onEnd() {
    m_game->saveSettings();

    if (m_game->getSettings().fullScreen != m_oldFullScreenValue) {
        m_game->setFullScreen();
    }

    if (auto p = m_game->getScene("PLAY"); p != nullptr) {
        for (auto [a, item] : m_game->getSettings().keys) {
            p->registerAction(item.action, item.key);
        }
    }
    
    m_game->setSoundVolume();
}

void Scene_Settings::sDoAction(const Action& action) {
    if (action.type() == "START") {
        if (action.name() == "QUIT") {
            m_game->changeScene("MENU", nullptr, true);
        } else if (action.name() == "WINDOW_RESIZED") {
            storeViewCenter(m_game->getWindowSize() / 2.0);
        }
    }
}

void Scene_Settings::sRender() {
    m_game->window().clear(sf::Color(30, 30, 30, 255));
    m_game->window().draw(m_help);
}
