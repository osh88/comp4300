#pragma once

#include "SFML/Graphics.hpp"
#include "SFML/Graphics/Font.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "SFML/Audio/SoundBuffer.hpp"
#include "SFML/Audio/Sound.hpp"
#include <map>
#include <string>
#include <iostream>
#include <vector>

#include "Vec2.h"

#include "json.hpp"
using json = nlohmann::json;

#ifdef _WIN32
    typedef unsigned int uint;
#endif

class Font {
public:
    std::string path;
    sf::Font    font;

    Font() {}
    
    Font(const std::string & p, sf::Font& f)
        : path(p)
        , font(f)
    {}
};

class Sound {
    sf::SoundBuffer            m_buffer;
    std::shared_ptr<sf::Sound> m_sound;

public:

    Sound() {}
    
    Sound(const std::string& path) {
        m_buffer = sf::SoundBuffer();
        if (!m_buffer.loadFromFile(path)) {
            std::cerr << "Could not load sound " << path << "!\n";
            exit(-1);
        }
        m_sound = std::shared_ptr<sf::Sound>(new sf::Sound(m_buffer));
    }

    void setVolume(uint volume) {
        if (volume >= 0 && volume <= 100) {
            m_sound->setVolume(volume);
        }
    }

    void replay() {
        m_sound->setPlayingOffset(sf::Time::Zero);
        play();
    }

    void play() {
        if (m_sound->getStatus() != sf::SoundSource::Status::Playing) {
            m_sound->play();
        }
    }

    void pause() {
        if (m_sound->getStatus() == sf::SoundSource::Status::Playing) {
            m_sound->pause();
        }
    }

    void setLooping(bool v) {
        m_sound->setLooping(v);
    }
};

class Texture {
public:
    sf::Texture texture;
    std::string name = "";
    Vec2        pos  = {0, 0};
    Vec2        size = {0, 0};

    Texture() {}
    Texture(const sf::Texture & tex, const std::string & name, Vec2 pos, Vec2 size) : texture(tex), name(name), pos(pos), size(size) {}

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Texture, name, pos, size)
};

class Animation {
    std::string m_name         = "none";
    Texture     m_texture      = Texture();
    sf::Sprite  m_sprite;
    size_t      m_frameCount   = 1;      // total number of frames of animation
    size_t      m_currentFrame = 0;      // the current frame of animation being played
    Vec2        m_frameSize    = {1, 1}; // size of the animation frame
    size_t      m_speed        = 0;      // the speed to play this animation
    bool        m_ended        = false;
    uint8_t     m_alpha        = 255;
    std::string m_shader       = "";
    float       m_mix          = 0.5; // коэффициент смешивания текстуры и шейдера

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Animation, m_frameCount, m_currentFrame, m_speed, m_texture, m_frameSize, m_name, m_ended, m_alpha, m_shader, m_mix)
public:

    Animation();
    Animation(const std::string & name, const Texture & t);
    Animation(const std::string & name, const Texture & t, size_t frameCount, size_t speed);

    void update();
    bool hasEnded() const;

    const std::string & getName() const;
    const Texture & getTexture() const;
    const Vec2 & frameSize() const;
    
    sf::Sprite & getSprite();
    void setSprite(const Texture & t);
    
    uint8_t getAlpha() const;
    void setAlpha(uint8_t v);
    
    const sf::Color getColor() const;
    void setColor(sf::Color);
    
    const std::string & getShader();
    void setShader(const std::string & c);
    
    float getMix() const;
    void setMix(float v);
};

class Assets
{
    std::map<std::string, Texture> m_textures;
    std::map<std::string, Animation> m_animations;
    std::map<std::string, Font> m_fonts;
    std::map<std::string, std::shared_ptr<Sound>> m_sounds;
    std::map<std::string, std::shared_ptr<sf::Shader>> m_shaders;

    sf::Image                          m_image;
    std::shared_ptr<sf::RenderTexture> m_renderTexture;
    std::shared_ptr<sf::Texture>       m_largeTexture;

public:

    void addTexture(const std::string& name, const std::string& path);
    void addAnimation(const std::string& name, Animation animation);
    void addFont(const std::string& name, const std::string& path);
    void addSound(const std::string& name, const std::string& path);
    void addShader(const std::string& name, const std::string& path);

    const Texture& getTexture(const std::string& name) const;
    const Animation& getAnimation(const std::string& name) const;
    const Font& getFont(const std::string& name) const;
    const std::shared_ptr<Sound> getSound(const std::string& name) const;
    const std::shared_ptr<sf::Shader> getShader(const std::string& name) const;
    const std::map<std::string, Animation> getAnimations();

    void buildLargeTexture();
    std::shared_ptr<sf::Texture> getLargeTexture() const;

    void setVolume(int volume);
    void loadFromFile(const std::string& baseDir, const std::string& path);
};
