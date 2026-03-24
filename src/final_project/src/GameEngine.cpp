#include "GameEngine.h"
#include "Assets.h"
#include "Scene_Menu.h"
#include "Profiler.h"
#include "imgui.h"
#include "imgui-SFML.h"
#include "Helpers.h"

#include <SFML/Graphics.hpp>
#include <iostream>
#include <chrono>
#include <thread>

GameEngine::GameEngine(const std::string & settingsPath) {
    init(settingsPath);
}

void GameEngine::init(const std::string & settingsPath) {
    IMGUI_CHECKVERSION();
    m_settingsPath = settingsPath;
    m_exeDir = Helpers::getExecutableDirectory() + "/";

    loadSettings();

    m_assets.loadFromFile(m_exeDir, getSettings().assetsPath);
    m_assets.buildLargeTexture();
    m_vertexArrays = VertexArrays();

    m_windowPrevSize = Vec2(0, 0);
    setFullScreen(true);
    setSoundVolume();

    if (!ImGui::SFML::Init(m_window, false)) {
        std::cout << "imgui sfml init error" << std::endl;
        std::exit(-1);
    };

    auto& io = ImGui::GetIO();
    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF(assets().getFont("Mario").path.c_str(), 15.f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    if (!ImGui::SFML::UpdateFontTexture()) {
        std::cout << "imgui sfml update font texture error" << std::endl;
        std::exit(-1);
    }

    m_inventory = Inventory(9, 4);

    changeScene("MENU", std::make_shared<Scene_Menu>(this));
}

void GameEngine::setFullScreen(bool init) {
    Vec2 viewCenter = {0, 0};

    if (!init) {
        viewCenter = getViewCenter();
        m_window.close();
    }

    if (getSettings().fullScreen) {
        m_window.create(sf::VideoMode::getDesktopMode(), L"Не Марио", sf::Style::Titlebar|sf::Style::Close|sf::Style::Resize, sf::State::Fullscreen);
    } else {
        m_window.create(sf::VideoMode(sf::Vector2u(1280, 768)), L"Не Марио", sf::Style::Titlebar|sf::Style::Close|sf::Style::Resize, sf::State::Windowed);
    }
    
    auto wh = getWindowSize();

    setViewCenter(viewCenter);

    // Отправляем событие об изменении размера окна только если окно уже было создано
    if (!init) {
        auto delta = wh - m_windowPrevSize;

        for (auto s : m_sceneMap) {
            s.second->setBaseViewSize(wh);
            s.second->doAction(Action("WINDOW_RESIZED", "START", delta));
        }

        setViewCenter(currentScene()->getStoredViewCenter());
        setViewSize(currentScene()->computedViewSize());

        //currentScene()->setViewCenter(currentScene()->computedViewSize());
        // auto view = m_window.getView();
        // view.setCenter(currentScene()->getViewCenter());
        // view.setSize(Vec2(wh));
        // window().setView(view);
    }

    m_windowPrevSize = Vec2(wh.x, wh.y);
}

void GameEngine::setSoundVolume() {
    assets().setVolume(getSettings().mute ? 0 : getSettings().effectsVolume);
    assets().getSound("MusicTitle")->setVolume(getSettings().mute ? 0 : getSettings().musicVolume);
}

std::shared_ptr<Scene> GameEngine::currentScene() {
    return m_sceneMap[m_currentScene];
}

bool GameEngine::isRunning() {
    return m_running && m_window.isOpen();
}

sf::RenderWindow & GameEngine::window() {
    return m_window;
}

void GameEngine::run() {
    int i = 0;
    const float MS_PER_FRAME = 1000 / (m_fps);
    auto s = std::chrono::high_resolution_clock::now();
    std::chrono::nanoseconds d;

    while(isRunning()) {
        PROFILE_SCOPE("GameEngine::onFrame");

        s = std::chrono::high_resolution_clock::now();
        update();
        d = std::chrono::high_resolution_clock::now()-s;
        m_frameTime = (float)std::chrono::duration_cast<std::chrono::microseconds>(d).count() / 1000;
        
        if (i % 200 == 0) {
            std::cout << "frameTime: " << m_frameTime << std::endl;
        }

        if (m_frameTime < MS_PER_FRAME) {
            PROFILE_SCOPE("GameEngine::sleep");
            auto sleep = std::chrono::milliseconds(int(MS_PER_FRAME - m_frameTime));
            
            auto t = std::chrono::high_resolution_clock::now() + sleep;
            while (std::chrono::high_resolution_clock::now() < t) {
                sqrt(131313);
            }
        }

        i++;
    }
}

void GameEngine::setFPS(int v) {
    m_fps = v;
}

const int GameEngine::getFrameTime() const {
    return m_frameTime;
}

void GameEngine::sUserInput() {
    PROFILE_FUNCTION();

    while (const std::optional event = m_window.pollEvent()) {
        ImGui::SFML::ProcessEvent(m_window, *event);

        if (event->is<sf::Event::Closed>())
                m_window.close();

        if (event->is<sf::Event::KeyPressed>()) {
            auto ev = event->getIf<sf::Event::KeyPressed>();

            if (ev->code == sf::Keyboard::Key::X) {
                auto texture = sf::Texture(sf::Vector2u(m_window.getSize().x, m_window.getSize().y), false);
                texture.update(m_window);
                auto path = getExeDir() + "screenshot.png";
                if (texture.copyToImage().saveToFile(path)) {
                    std::cout << "Screenshot saved to " + path << std::endl;
                }
            }

            if (ev->code == sf::Keyboard::Key::F && ev->control) {
                getSettings().fullScreen = !getSettings().fullScreen;
                setFullScreen();
            }

            if (ev->code == sf::Keyboard::Key::M && ev->control) {
                getSettings().mute = !getSettings().mute;
                setSoundVolume();
            }
        }

        if (event->is<sf::Event::KeyPressed>() || event->is<sf::Event::KeyReleased>()) {
            if (ImGui::GetIO().WantCaptureKeyboard) { return; }

            KeyAction key;

            if (event->is<sf::Event::KeyPressed>()) {
                auto o = event->getIf<sf::Event::KeyPressed>();
                key = KeyAction(o->code, o->control, o->shift, o->alt, o->system);
            } else {
                auto o = event->getIf<sf::Event::KeyReleased>();
                key = KeyAction(o->code, o->control, o->shift, o->alt, o->system);
            }

            // if the current scene does not have an action associated with this key, skip the event
            if (auto r = currentScene()->getAction(key); r.has_value()) {
                // determine start or end action by whether it was key press or release
                const std::string actionType = (event->is<sf::Event::KeyPressed>()) ? "START" : "END";

                // look up the action and send the action to the scene
                currentScene()->doAction(Action(r.value(), actionType));   
            }
        }

        if (event->is<sf::Event::MouseButtonPressed>()) {
            if (ImGui::GetIO().WantCaptureMouse) { return; }

            auto mousePos = sf::Mouse::getPosition(m_window);
            Vec2 mpos = p2c(Vec2(mousePos.x, mousePos.y));

            switch (event->getIf<sf::Event::MouseButtonPressed>()->button) {
                case sf::Mouse::Button::Left:   { currentScene()->doAction(Action("LEFT_CLICK",   "START", mpos, mousePos)); break; }
                case sf::Mouse::Button::Middle: { currentScene()->doAction(Action("MIDDLE_CLICK", "START", mpos, mousePos)); break; }
                case sf::Mouse::Button::Right:  { currentScene()->doAction(Action("RIGHT_CLICK",  "START", mpos, mousePos)); break; }
                default: break;
            }
        }

        if (event->is<sf::Event::MouseButtonReleased>()) {
            auto mousePos = sf::Mouse::getPosition(m_window);
            Vec2 mpos = p2c(Vec2(mousePos.x, mousePos.y));

            switch (event->getIf<sf::Event::MouseButtonReleased>()->button) {
                case sf::Mouse::Button::Left:   { currentScene()->doAction(Action("LEFT_CLICK",   "END", mpos, mousePos)); break; }
                case sf::Mouse::Button::Middle: { currentScene()->doAction(Action("MIDDLE_CLICK", "END", mpos, mousePos)); break; }
                case sf::Mouse::Button::Right:  { currentScene()->doAction(Action("RIGHT_CLICK",  "END", mpos, mousePos)); break; }
                default: break;
            }
        }

        if (event->is<sf::Event::MouseWheelScrolled>()) {
            if (ImGui::GetIO().WantCaptureMouse) { return; }
            
            float delta = event->getIf<sf::Event::MouseWheelScrolled>()->delta;
#ifdef _WIN32
            delta *= 5.0; // На маке 5-15, на винде 1-3
#endif
            currentScene()->doAction(Action("MOUSE_SCROLL", "START", Vec2(delta)));
        }

        if (event->is<sf::Event::MouseMoved>()) {
            auto mousePos = sf::Mouse::getPosition(m_window);
            Vec2 mpos = p2c(Vec2(mousePos.x, mousePos.y));

            currentScene()->doAction(Action("MOUSE_MOVE",  "START", mpos, mousePos));
        }

        if (event->is<sf::Event::Resized>()) {
            auto wh = Vec2(event->getIf<sf::Event::Resized>()->size);
            auto delta = wh - m_windowPrevSize;

            for (auto s : m_sceneMap) {
                s.second->setBaseViewSize(wh);
                s.second->doAction(Action("WINDOW_RESIZED", "START", delta));
            }

            setViewCenter(currentScene()->getStoredViewCenter());
            setViewSize(currentScene()->computedViewSize());
            // auto view = m_window.getView();
            // view.setCenter(currentScene()->getViewCenter());
            // view.setSize(sf::Vector2f(wh.x, wh.y));
            // window().setView(view);

            m_windowPrevSize = wh;
        }
    }
}

void GameEngine::changeScene(const std::string & sceneName, std::shared_ptr<Scene> scene, bool endCurrentScene) {
    m_nextScene.sceneName = sceneName;
    m_nextScene.scene = scene;
    m_nextScene.endCurrentScene = endCurrentScene;
}

void GameEngine::checkNextScene() {
    if (m_nextScene.sceneName == "") {
        return;
    }

    if (m_sceneMap.find(m_currentScene) != m_sceneMap.end()) {
        if (m_nextScene.endCurrentScene) {
            currentScene()->onEnd();
            m_sceneMap.erase(m_currentScene);
        } else {
            currentScene()->onPause(m_nextScene.sceneName);
            currentScene()->storeViewCenter(getViewCenter());
        }
    }

    auto oldCurrentScene = m_currentScene;
    m_currentScene = m_nextScene.sceneName;
    if (m_sceneMap.find(m_nextScene.sceneName) == m_sceneMap.end()) {
        m_nextScene.scene->init();
        m_sceneMap[m_nextScene.sceneName] = m_nextScene.scene;
    } else {
        currentScene()->onResume(oldCurrentScene);
    }

    setViewCenter(currentScene()->getStoredViewCenter());
    setViewSize(currentScene()->computedViewSize());

    m_nextScene.sceneName = "";
    m_nextScene.scene = nullptr;
    m_nextScene.endCurrentScene = false;
}

void GameEngine::quit() {
    saveSettings();
    for (auto s : m_sceneMap) {
        s.second->onEnd();
    }

    //ImGui::SFML::Shutdown(m_window);
    m_running = false;
    m_window.close();
}

VertexArrays & GameEngine::getVertexArrays() {
    return m_vertexArrays;
}

void GameEngine::update() {
    PROFILE_FUNCTION();

    {
        PROFILE_SCOPE("ImGui::SFML::Update()");
        ImGui::SFML::Update(m_window, m_deltaClock.restart());
    }

    checkNextScene();
    sUserInput();
    currentScene()->update();

    {
        PROFILE_SCOPE("ImGui::SFML::Render()");
        ImGui::SFML::Render(m_window);
    }

    {
        PROFILE_SCOPE("m_window.display()");
        m_window.display();
    }
}

Assets & GameEngine::assets() {
    return m_assets;
}

const std::string& GameEngine::currentSceneName() const {
    return m_currentScene;
}

std::shared_ptr<Scene> GameEngine::getScene(const std::string& sceneName) {
    if (m_sceneMap.find(sceneName) != m_sceneMap.end()) {
        return m_sceneMap[sceneName];
    }
    
    return nullptr;
}

void GameEngine::deleteScene(const std::string& name) {
    m_sceneMap.erase(name);
}

Settings& GameEngine::getSettings() {
    return m_settings;
}

const std::string & GameEngine::getExeDir() {
    return m_exeDir;
}

void GameEngine::loadSettings() {
    m_settings = Settings::loadFromFile(m_exeDir + m_settingsPath);
}

void GameEngine::saveSettings() {
    getSettings().save(m_exeDir + m_settingsPath);
}

float GameEngine::getPlayerDamageCoeff() {
    static float coeff[3] = {2.0, 1.0, 0.5};
    return  coeff[getSettings().difficulty];
}

float GameEngine::getNpcDamageCoeff() {
    static float coeff[3] = {0.5, 1.0, 2.0};
    return  coeff[getSettings().difficulty];
}

void GameEngine::moveView(const Vec2 & delta) {
    setViewCenter(getViewCenter() + delta);
}

void GameEngine::setViewCenter(const Vec2 & pos) {
    auto v = m_window.getView();
    v.setCenter(pos);
    m_window.setView(v);
}

Vec2 GameEngine::getViewCenter() const {
    return m_window.getView().getCenter();
}

Vec2 GameEngine::getWindowSize() const {
    return m_window.getSize();
}

Vec2 GameEngine::getViewSize() const {
    return m_window.getView().getSize();
}

void GameEngine::setViewSize(const Vec2 & s) {
    auto v = m_window.getView();
    v.setSize(s);
    m_window.setView(v);
}

Inventory & GameEngine::inventory() {
    return m_inventory;
}