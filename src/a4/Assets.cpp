#include "Assets.h"
#include "Profiler.h"

#include <fstream>
#include <iostream>


void Assets::addTexture(const std::string& name, const std::string& path) {
    sf::Texture texture;
    if (!texture.loadFromFile(path)) {
        std::cerr << "Could not load image " << path << "!\n";
        exit(-1);
    }
    texture.setSmooth(true);
    m_textures[name] = texture;
}

void Assets::addAnimation(const std::string& name, Animation animation) {
    m_animations[name] = animation;
}

void Assets::addFont(const std::string& name, const std::string& path) {
    sf::Font font;
    if (!font.openFromFile(path)) {
        std::cerr << "Could not load font " << path << "!\n";
        exit(-1);
    }
    m_fonts[name] = font;
}

void Assets::addSound(const std::string& name, const std::string& path) {
    m_soundBuffers[name] = sf::SoundBuffer();
    if (!m_soundBuffers[name].loadFromFile(path)) {
        std::cerr << "Could not load sound " << path << "!\n";
        exit(-1);
    }
    m_sounds[name] = std::shared_ptr<sf::Sound>(new sf::Sound(m_soundBuffers[name]));
}

void Assets::addShader(const std::string& name, const std::string& path) {
    m_shaders[name] = std::shared_ptr<sf::Shader>(new sf::Shader());
    if (!m_shaders[name]->loadFromFile(path, sf::Shader::Type::Fragment)) {
        std::cerr << "Could not load shader " << path << "!\n";
        exit(-1);
    }
}

const sf::Texture& Assets::getTexture(const std::string& name) const {
    try {
        return m_textures.at(name);
    } catch(const std::out_of_range& e) {
        std::cout << "texture '" << name << "' not found" << std::endl;
        throw e;
    }
}

const Animation& Assets::getAnimation(const std::string& name) const {
    try {
        return m_animations.at(name);
    } catch(const std::out_of_range& e) {
        std::cout << "animation '" << name << "' not found" << std::endl;
        throw e;
    }
}

const sf::Font& Assets::getFont(const std::string& name) const {
    try {
        return m_fonts.at(name);
    } catch(const std::out_of_range& e) {
        std::cout << "font '" << name << "' not found" << std::endl;
        throw e;
    }
}

const std::shared_ptr<sf::Sound> Assets::getSound(const std::string& name) const {
    try {
        return m_sounds.at(name);
    } catch(const std::out_of_range& e) {
        std::cout << "sound '" << name << "' not found" << std::endl;
        throw e;
    }
}

const std::shared_ptr<sf::Shader> Assets::getShader(const std::string& name) const {
    try {
        return m_shaders.at(name);
    } catch(const std::out_of_range& e) {
        std::cout << "shader '" << name << "' not found" << std::endl;
        throw e;
    }
}

void Assets::loadFromFile(const std::string& path) {
    PROFILE_FUNCTION();

    std::ifstream file(path);
    if (!file) {
        std::cerr << "Could not load config: " << path << std::endl;
        exit(-1);
    }

    std::string head;
    while (file >> head) {
        if (head == "Font") {
            std::string name;
            std::string path;
            file >> name >> path;
            addFont(name, path);
        } else if (head == "Texture") {
            std::string name;
            std::string path;
            file >> name >> path;
            addTexture(name, path);
        } else if (head == "Animation") {
            std::string aniName;
            std::string texName;
            int frames, speed;
            file >> aniName >> texName >> frames >> speed;
            const sf::Texture &tex = getTexture(texName);
            addAnimation(aniName, Animation(aniName, tex, frames, speed));
        } else if (head == "Sound") {
            std::string name;
            std::string path;
            file >> name >> path;
            addSound(name, path);
        } else if (head == "Shader") {
            std::string name;
            std::string path;
            file >> name >> path;
            addShader(name, path);
        } else {
            std::cerr << "head to " << head << "\n";
            std::cerr << "The config file format is incorrect!\n";
            exit(-1);
        }
    }
}
