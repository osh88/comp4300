#include "GameEngine.h"
#include "Assets.h"
#include "Scene_Menu.h"
#include "Profiler.h"

#include <iostream>
#include <chrono>
#include <thread>

GameEngine::GameEngine(const std::string & path) {
    init(path);
}

void GameEngine::init(const std::string & assetsPath) {
    m_assets.loadFromFile(assetsPath);

    m_window.create(sf::VideoMode(sf::Vector2u(1280, 768)), "Not Zelda");
    //m_window.setFramerateLimit(60);
    m_windowPrevSize = Vec2(window().getSize().x, window().getSize().y);

    changeScene("MENU", std::make_shared<Scene_Menu>(this));
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
    const float MS_PER_FRAME = 1000 / 60;
    int i = 0;
    while(isRunning()) {
        PROFILE_SCOPE("GameEngine::onFrame");
        auto s = std::chrono::high_resolution_clock::now();
        update();
        auto d = std::chrono::high_resolution_clock::now()-s;
        m_frameTime = std::chrono::duration_cast<std::chrono::milliseconds>(d).count();

        if (m_frameTime < MS_PER_FRAME) {
            auto sleep = std::chrono::milliseconds(int(MS_PER_FRAME - m_frameTime));
            std::this_thread::sleep_for(sleep);
        }

        i++;
    }
}

const int GameEngine::getFrameTime() const {
    return m_frameTime;
}

void GameEngine::sUserInput() {
    while (const std::optional event = m_window.pollEvent()) {
        if (event->is<sf::Event::KeyPressed>()) {
            if (event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::X) {
                auto texture = sf::Texture(sf::Vector2u(m_window.getSize().x, m_window.getSize().y), false);
                texture.update(m_window);
                if (texture.copyToImage().saveToFile("test.png")) {
                    std::cout << "screenshot saved to test.png" << std::endl;
                }
            }
        }

        if (event->is<sf::Event::KeyPressed>() || event->is<sf::Event::KeyReleased>()) {
            sf::Keyboard::Key keyCode;

            if (event->is<sf::Event::KeyPressed>()) {
                keyCode = event->getIf<sf::Event::KeyPressed>()->code;
            } else {
                keyCode = event->getIf<sf::Event::KeyReleased>()->code;
            }

            // if the current scene does not have an action associated with this key, skip the event
            if (currentScene()->getActionMap().find(static_cast<int>(keyCode)) == currentScene()->getActionMap().end()) {
                continue;
            }

            // determine start or end action by whether it was key press or release
            const std::string actionType = (event->is<sf::Event::KeyPressed>()) ? "START" : "END";

            // look up the action and send the action to the scene
            currentScene()->doAction(Action(currentScene()->getActionMap().at(static_cast<int>(keyCode)), actionType));
        }

        if (event->is<sf::Event::MouseButtonPressed>()) {
            auto mousePos = sf::Mouse::getPosition(m_window);
            Vec2 mpos = windowToWorld(Vec2(mousePos.x, mousePos.y));

            switch (event->getIf<sf::Event::MouseButtonPressed>()->button) {
                case sf::Mouse::Button::Left:   { currentScene()->doAction(Action("LEFT_CLICK",   "START", mpos)); break; }
                case sf::Mouse::Button::Middle: { currentScene()->doAction(Action("MIDDLE_CLICK", "START", mpos)); break; }
                case sf::Mouse::Button::Right:  { currentScene()->doAction(Action("RIGHT_CLICK",  "START", mpos)); break; }
                default: break;
            }
        }

        if (event->is<sf::Event::MouseButtonReleased>()) {
            auto mousePos = sf::Mouse::getPosition(m_window);
            Vec2 mpos = windowToWorld(Vec2(mousePos.x, mousePos.y));

            switch (event->getIf<sf::Event::MouseButtonReleased>()->button) {
                case sf::Mouse::Button::Left:   { currentScene()->doAction(Action("LEFT_CLICK",   "END", mpos)); break; }
                case sf::Mouse::Button::Middle: { currentScene()->doAction(Action("MIDDLE_CLICK", "END", mpos)); break; }
                case sf::Mouse::Button::Right:  { currentScene()->doAction(Action("RIGHT_CLICK",  "END", mpos)); break; }
                default: break;
            }
        }

        if (event->is<sf::Event::MouseMoved>()) {
            auto mousePos = sf::Mouse::getPosition(m_window);
            Vec2 mpos = windowToWorld(Vec2(mousePos.x, mousePos.y));

            currentScene()->doAction(Action("MOUSE_MOVE",  "START", mpos));
        }

        if (event->is<sf::Event::Resized>()) {
            auto wh = event->getIf<sf::Event::Resized>()->size;
            auto view = window().getView();
            view.setCenter({(float)(wh.x/2.0), (float)(wh.y/2.0)});
            view.setSize({(float)wh.x, (float)wh.y});
            window().setView(view);

            currentScene()->doAction(Action("WINDOW_RESIZED", "START", Vec2(wh.x, wh.y)-m_windowPrevSize));

            m_windowPrevSize = Vec2(wh.x, wh.y);
        }
    }
}

void GameEngine::changeScene(const std::string & sceneName, std::shared_ptr<Scene> scene, bool endCurrentScene) {
    if (endCurrentScene && m_sceneMap.find(m_currentScene) != m_sceneMap.end()) {
        m_sceneMap.erase(m_currentScene);
    }

    m_currentScene = sceneName;
//    if (m_sceneMap.find(sceneName) == m_sceneMap.end()) {
        m_sceneMap[sceneName] = scene;
//    }

    auto wh = window().getSize();
    auto view = window().getView();
    view.setCenter({(float)(wh.x/2.0), (float)(wh.y/2.0)});
    view.setSize({(float)wh.x, (float)wh.y});
    window().setView(view);
}

void GameEngine::quit() {
    m_running = false;
    m_window.close();
}

void GameEngine::update() {
    sUserInput();
    currentScene()->update();
    m_window.display();
}

const Assets & GameEngine::assets() const {
    return m_assets;
}

const std::string& GameEngine::currentSceneName() const {
    return m_currentScene;
}

Vec2 GameEngine::windowToWorld(const Vec2 & windowPos) {
    auto view = window().getView();
    auto tl = view.getCenter()-view.getSize()/2.0f;
    return windowPos + Vec2(tl.x, tl.y);
}
