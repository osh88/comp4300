#pragma once

#include "Scene.h"
#include "imgui.h"

class Scene_Settings : public Scene {
    bool m_oldFullScreenValue;

    struct SGUI {
        std::map<sf::Keyboard::Key, size_t>    keysToIdx;
        std::map<std::string, ImGuiTextFilter> imguiFilters;
    };

protected:
    sf::Text m_help;
    SGUI     m_sGUI;

    void update();
    void onEnd();
    void sDoAction(const Action & action);
    void sGUI();

public:

    Scene_Settings(GameEngine * gameEngine = nullptr);
    void init();
    void sRender();
};

struct key {
    sf::Keyboard::Key k;
    std::string       n;
};

static const std::vector<key> keys {
    {sf::Keyboard::Key::A,          "A"},          //!< The A key
    {sf::Keyboard::Key::B,          "B"},          //!< The B key
    {sf::Keyboard::Key::C,          "C"},          //!< The C key
    {sf::Keyboard::Key::D,          "D"},          //!< The D key
    {sf::Keyboard::Key::E,          "E"},          //!< The E key
    {sf::Keyboard::Key::F,          "F"},          //!< The F key
    {sf::Keyboard::Key::G,          "G"},          //!< The G key
    {sf::Keyboard::Key::H,          "H"},          //!< The H key
    {sf::Keyboard::Key::I,          "I"},          //!< The I key
    {sf::Keyboard::Key::J,          "J"},          //!< The J key
    {sf::Keyboard::Key::K,          "K"},          //!< The K key
    {sf::Keyboard::Key::L,          "L"},          //!< The L key
    {sf::Keyboard::Key::M,          "M"},          //!< The M key
    {sf::Keyboard::Key::N,          "N"},          //!< The N key
    {sf::Keyboard::Key::O,          "O"},          //!< The O key
    {sf::Keyboard::Key::P,          "P"},          //!< The P key
    {sf::Keyboard::Key::Q,          "Q"},          //!< The Q key
    {sf::Keyboard::Key::R,          "R"},          //!< The R key
    {sf::Keyboard::Key::S,          "S"},          //!< The S key
    {sf::Keyboard::Key::T,          "T"},          //!< The T key
    {sf::Keyboard::Key::U,          "U"},          //!< The U key
    {sf::Keyboard::Key::V,          "V"},          //!< The V key
    {sf::Keyboard::Key::W,          "W"},          //!< The W key
    {sf::Keyboard::Key::X,          "X"},          //!< The X key
    {sf::Keyboard::Key::Y,          "Y"},          //!< The Y key
    {sf::Keyboard::Key::Z,          "Z"},          //!< The Z key
    // {sf::Keyboard::Key::Num0,       "Num0"},       //!< The 0 key
    // {sf::Keyboard::Key::Num1,       "Num1"},       //!< The 1 key
    // {sf::Keyboard::Key::Num2,       "Num2"},       //!< The 2 key
    // {sf::Keyboard::Key::Num3,       "Num3"},       //!< The 3 key
    // {sf::Keyboard::Key::Num4,       "Num4"},       //!< The 4 key
    // {sf::Keyboard::Key::Num5,       "Num5"},       //!< The 5 key
    // {sf::Keyboard::Key::Num6,       "Num6"},       //!< The 6 key
    // {sf::Keyboard::Key::Num7,       "Num7"},       //!< The 7 key
    // {sf::Keyboard::Key::Num8,       "Num8"},       //!< The 8 key
    // {sf::Keyboard::Key::Num9,       "Num9"},       //!< The 9 key
    // {sf::Keyboard::Key::Escape,     "Escape"},     //!< The Escape key
    {sf::Keyboard::Key::LControl,   "LControl"},   //!< The left Control key
    {sf::Keyboard::Key::LShift,     "LShift"},     //!< The left Shift key
    {sf::Keyboard::Key::LAlt,       "LAlt"},       //!< The left Alt key
    {sf::Keyboard::Key::LSystem,    "LSystem"},    //!< The left OS specific key: window (Windows and Linux), apple (macOS), ...
    {sf::Keyboard::Key::RControl,   "RControl"},   //!< The right Control key
    {sf::Keyboard::Key::RShift,     "RShift"},     //!< The right Shift key
    {sf::Keyboard::Key::RAlt,       "RAlt"},       //!< The right Alt key
    {sf::Keyboard::Key::RSystem,    "RSystem"},    //!< The right OS specific key: window (Windows and Linux), apple (macOS), ...
    // {sf::Keyboard::Key::Menu,       "Menu"},       //!< The Menu key
    {sf::Keyboard::Key::LBracket,   "["},          //!< The [ key
    {sf::Keyboard::Key::RBracket,   "]"},          //!< The ] key
    {sf::Keyboard::Key::Semicolon,  ";"},          //!< The ; key
    {sf::Keyboard::Key::Comma,      ","},          //!< The , key
    {sf::Keyboard::Key::Period,     "."},          //!< The . key
    {sf::Keyboard::Key::Apostrophe, "'"},          //!< The ' key
    {sf::Keyboard::Key::Slash,      "/"},          //!< The / key
    {sf::Keyboard::Key::Backslash,  "\\"},         //!< The \ key
    {sf::Keyboard::Key::Grave,      "`"},          //!< The ` key
    {sf::Keyboard::Key::Equal,      "="},          //!< The = key
    {sf::Keyboard::Key::Hyphen,     "- (Hyphen)"}, //!< The - key (hyphen)
    {sf::Keyboard::Key::Space,      "Space"},      //!< The Space key
    {sf::Keyboard::Key::Enter,      "Enter"},      //!< The Enter/Return keys
    // {sf::Keyboard::Key::Backspace,  "Backspace"},  //!< The Backspace key
    // {sf::Keyboard::Key::Tab,        "Tab"},        //!< The Tabulation key
    // {sf::Keyboard::Key::PageUp,     "PageUp"},     //!< The Page up key
    // {sf::Keyboard::Key::PageDown,   "PageDown"},   //!< The Page down key
    // {sf::Keyboard::Key::End,        "End"},        //!< The End key
    // {sf::Keyboard::Key::Home,       "Home"},       //!< The Home key
    // {sf::Keyboard::Key::Insert,     "Insert"},     //!< The Insert key
    // {sf::Keyboard::Key::Delete,     "Delete"},     //!< The Delete key
    // {sf::Keyboard::Key::Add,        "+"},          //!< The + key
    // {sf::Keyboard::Key::Subtract,   "- (Minus)"},  //!< The - key (minus, usually from numpad)
    // {sf::Keyboard::Key::Multiply,   "*"},          //!< The * key
    // {sf::Keyboard::Key::Divide,     "/"},          //!< The / key
    {sf::Keyboard::Key::Left,       "Left"},       //!< Left arrow
    {sf::Keyboard::Key::Right,      "Right"},      //!< Right arrow
    {sf::Keyboard::Key::Up,         "Up"},         //!< Up arrow
    {sf::Keyboard::Key::Down,       "Down"},       //!< Down arrow
    {sf::Keyboard::Key::Numpad0,    "Numpad0"},    //!< The numpad 0 key
    {sf::Keyboard::Key::Numpad1,    "Numpad1"},    //!< The numpad 1 key
    {sf::Keyboard::Key::Numpad2,    "Numpad2"},    //!< The numpad 2 key
    {sf::Keyboard::Key::Numpad3,    "Numpad3"},    //!< The numpad 3 key
    {sf::Keyboard::Key::Numpad4,    "Numpad4"},    //!< The numpad 4 key
    {sf::Keyboard::Key::Numpad5,    "Numpad5"},    //!< The numpad 5 key
    {sf::Keyboard::Key::Numpad6,    "Numpad6"},    //!< The numpad 6 key
    {sf::Keyboard::Key::Numpad7,    "Numpad7"},    //!< The numpad 7 key
    {sf::Keyboard::Key::Numpad8,    "Numpad8"},    //!< The numpad 8 key
    {sf::Keyboard::Key::Numpad9,    "Numpad9"},    //!< The numpad 9 key
    // {sf::Keyboard::Key::F1,         "F1"},         //!< The F1 key
    // {sf::Keyboard::Key::F2,         "F2"},         //!< The F2 key
    // {sf::Keyboard::Key::F3,         "F3"},         //!< The F3 key
    // {sf::Keyboard::Key::F4,         "F4"},         //!< The F4 key
    // {sf::Keyboard::Key::F5,         "F5"},         //!< The F5 key
    // {sf::Keyboard::Key::F6,         "F6"},         //!< The F6 key
    // {sf::Keyboard::Key::F7,         "F7"},         //!< The F7 key
    // {sf::Keyboard::Key::F8,         "F8"},         //!< The F8 key
    // {sf::Keyboard::Key::F9,         "F9"},         //!< The F9 key
    // {sf::Keyboard::Key::F10,        "F10"},        //!< The F10 key
    // {sf::Keyboard::Key::F11,        "F11"},        //!< The F11 key
    // {sf::Keyboard::Key::F12,        "F12"},        //!< The F12 key
    // {sf::Keyboard::Key::F13,        "F13"},        //!< The F13 key
    // {sf::Keyboard::Key::F14,        "F14"},        //!< The F14 key
    // {sf::Keyboard::Key::F15,        "F15"},        //!< The F15 key
    // {sf::Keyboard::Key::Pause,      "Pause"}       //!< The Pause key
};