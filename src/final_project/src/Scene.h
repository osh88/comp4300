#pragma once

#include "Action.h"
#include "EntityManager.h"
#include "GameEngine.h"

#include <memory>

struct KeyAction {
    sf::Keyboard::Key key = sf::Keyboard::Key::Unknown;
    bool control = false;
    bool shift   = false;
    bool alt     = false;
    bool system  = false;

    KeyAction() {};

    KeyAction(sf::Keyboard::Key key, bool control, bool shift, bool alt, bool system)
        : key(key), control(control), shift(shift), alt(alt), system(system)
    {};

    bool operator<(const KeyAction& v) const {
        if (key != v.key)         return key < v.key;
        if (control != v.control) return control < v.control;
        if (shift != v.shift)     return shift < v.shift;
        if (alt != v.alt)         return alt < v.alt;
        
        return system < v.system;
    }

    bool operator==(const KeyAction& v) const {
        return key == v.key && control == v.control && shift == v.shift && alt == v.alt && system == v.system;
    }
};

typedef std::map<KeyAction, std::string> ActionMap;

class Scene {
protected:

    GameEngine *  m_game = nullptr;
    EntityManager m_entityManager;
    ActionMap     m_actionMap;
    bool          m_paused = false;
    size_t        m_currentFrame = 0;

    Vec2      m_storedViewCenter = {0, 0};
    Vec2      m_baseViewSize     = {0, 0};
    Vec2      m_scaleStep        = {0, 0};
    float     m_scale            = 0.0;
    sf::Color m_bgColor          = sf::Color(0x61, 0xa5, 0xc3);

public:

    Scene();
    Scene(GameEngine * gameEngine);

    virtual void update() = 0;
    virtual void sDoAction(const Action & action) = 0;
    virtual void sRender() = 0;
    virtual void reload() {};
    virtual void init() {};

    virtual void doAction(const Action & action);
    void simulate(const size_t frames);
    void registerAction(const std::string& actionName, sf::Keyboard::Key key, bool control = false, bool shift = false, bool alt = false, bool system = false);
    void unregisterAction(const std::string& actionName);
    void clearActions();

    size_t currentFrame() const;

    bool hasEnded() const;
    std::optional<std::string> getAction(KeyAction key) const;
    void drawLine(const Vec2 & p1, const Vec2 & p2, sf::Color color = sf::Color::Black);
    
    void storeViewCenter(const Vec2 & pos);
    const Vec2 & getStoredViewCenter() const;
    // void setViewSize(const Vec2 & s);
    // const Vec2 & getViewSize() const;

    Vec2 computedViewSize();
    Vec2 getBaseViewSize();
    void setBaseViewSize(const Vec2 & s);

    virtual void onEnd() {};
    virtual void onResume(const std::string & prevScene) {};
    virtual void onPause(const std::string & nextScene) {};

    virtual void saveState() {};
    virtual bool loadState() { return false; };

    const virtual std::string & levelPath() const { static std::string s = ""; return s; };
};