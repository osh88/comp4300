#include "Assets.h"
#include "Profiler.h"

#include <fstream>
#include <iostream>
#include <cmath>

void Assets::addTexture(const std::string& name, const std::string& path) {
    sf::Texture texture;
    if (!texture.loadFromFile(path)) {
        std::cerr << "Could not load image " << path << "!\n";
        exit(-1);
    }
    texture.setSmooth(true);
    m_textures[name] = Texture(texture, name, Vec2(0, 0), texture.getSize());
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
    m_fonts[name] = Font(path, font);
}

void Assets::addSound(const std::string& name, const std::string& path) {
    m_sounds[name] = std::make_shared<Sound>(path);
}

void Assets::addShader(const std::string& name, const std::string& path) {
    m_shaders[name] = std::shared_ptr<sf::Shader>(new sf::Shader());
    if (!m_shaders[name]->loadFromFile(path, sf::Shader::Type::Fragment)) {
        std::cerr << "Could not load shader " << path << "!\n";
        exit(-1);
    }
}

const Texture& Assets::getTexture(const std::string& name) const {
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

const Font& Assets::getFont(const std::string& name) const {
    try {
        return m_fonts.at(name);
    } catch(const std::out_of_range& e) {
        std::cout << "font '" << name << "' not found" << std::endl;
        throw e;
    }
}

const std::shared_ptr<Sound> Assets::getSound(const std::string& name) const {
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

void Assets::loadFromFile(const std::string& baseDir, const std::string& path) {
    PROFILE_FUNCTION();

    std::ifstream file(baseDir + path);
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
            addFont(name, baseDir + path);
        } else if (head == "Texture") {
            std::string name;
            std::string path;
            file >> name >> path;
            addTexture(name, baseDir + path);
        } else if (head == "Animation") {
            std::string aniName;
            std::string texName;
            int frames, speed;
            file >> aniName >> texName >> frames >> speed;
            const Texture & tex = getTexture(texName);
            addAnimation(aniName, Animation(aniName, tex, frames, speed));
        } else if (head == "Sound") {
            std::string name;
            std::string path;
            file >> name >> path;
            addSound(name, baseDir + path);
        } else if (head == "Shader") {
            std::string name;
            std::string path;
            file >> name >> path;
            addShader(name, baseDir + path);
        } else {
            std::cerr << "head to " << head << "\n";
            std::cerr << "The config file format is incorrect!\n";
            exit(-1);
        }
    }
}

void Assets::setVolume(int volume) {
    if (volume < 0 || volume > 100) {
        return;
    }

    for (auto& s : m_sounds) {
        s.second->setVolume(volume);
    }
}

const std::map<std::string, Animation> Assets::getAnimations() {
    return m_animations;
}

sf::Texture makeHealthTexture(int health, int max) {
    Vec2 healthSize(64, 4);
    sf::RenderTexture tex(healthSize);

    sf::RectangleShape tick({ 1.0f, 6.0f });
    tick.setFillColor(sf::Color::Black);

    sf::RectangleShape healthRect({ healthSize.x, healthSize.y });
    healthRect.setFillColor(sf::Color(96, 96, 96));
    healthRect.setOutlineColor(sf::Color::Black);
    healthRect.setOutlineThickness(1);

    sf::RectangleShape healthTick({ healthSize.x, healthSize.y });
    healthTick.setFillColor(sf::Color(255, 0, 0));
    healthTick.setOutlineThickness(0);

    tex.draw(healthRect);

    float ratio = (float)(health) / max;
    healthTick.setSize({ healthSize.x*ratio, healthSize.y });
    tex.draw(healthTick);

    return tex.getTexture();
}

void Assets::buildLargeTexture() {
    // Генерируем текстуры полоски здоровья
    for (int i=0; i<=100; i++) {
        auto tex = makeHealthTexture(i, 100);
        auto name = "healthBar" + std::to_string(i);
        auto t = Texture(tex, name, Vec2(0, 0), Vec2(tex.getSize()));
        m_textures[name] = t;

        auto a = Animation(name, t, 1, 0);
        a.getSprite().setOrigin(a.getTexture().size / 2.0);
        addAnimation(name, a);
    }

    // Определяем размер большой текстуры

    auto maxLineWidth = 5000;
    int lineCount = 1;
    int lineWidth;
    Vec2 wh = {0, 0};

    for (auto t : m_textures) {
        auto s = t.second.size;
        wh.x += s.x;
        if (s.y > wh.y) {
            wh.y = s.y;
        }

        lineWidth += s.x;
        if (lineWidth > maxLineWidth) {
            lineCount++;
            lineWidth = 0;
        }
    }

    auto lineHeight = wh.y;
    if (wh.x > maxLineWidth) {
        wh.x = maxLineWidth;
    }
    wh.y *= lineCount;

    // Создаем большую текстуру и копируем в нее все малые текстуры
    
    m_renderTexture = std::shared_ptr<sf::RenderTexture>(new sf::RenderTexture(wh));
    m_renderTexture->clear(sf::Color::Transparent);

    Vec2 pos = {0, 0};
    for (auto& t : m_textures) {
        sf::Sprite sp(t.second.texture);

        if (pos.x + t.second.size.x > wh.x) {
            pos.x = 0;
            pos.y += lineHeight;
        }

        sp.setPosition(pos);
        m_renderTexture->draw(sp);
        t.second.pos = pos;

        pos.x += t.second.size.x;
    }

    // sf::RenderTexture переворачивает изображение, поэтому перед сохранением переворачиваем ее обратно
    m_image = m_renderTexture->getTexture().copyToImage();
    m_image.flipVertically();
    
    m_largeTexture = std::shared_ptr<sf::Texture>(new sf::Texture());
    
    if (!m_largeTexture->loadFromImage(m_image)) {
        std::cerr << "Assets::buildLargeTexture(): can't load texture from image" << std::endl;
    }

    // Сохраняем текстуру на диск для визуального контроля
    if (!m_image.saveToFile("tileset.png")) {
        std::cerr << "Assets::buildLargeTexture(): can't save tileset.png" << std::endl;
    }

    // Затираем малые текстуры, чтобы точно знать, что рендерим с помощью большой текстуры
    for (auto& t : m_textures) {
        t.second.texture = sf::Texture();
    }

    // Задаем текстуру заново, т.к. позиции текстур были скопированы, а не передавались по ссылке.
    for (auto& a : m_animations) {
        auto tex = getTexture(a.second.getTexture().name);
        a.second.setSprite(tex);
    }
}

std::shared_ptr<sf::Texture> Assets::getLargeTexture() const {
    return m_largeTexture;
}

Animation::Animation()
    : Animation("", Texture(), 1, 0)
{}

Animation::Animation(const std::string & name, const Texture & t)
    : Animation(name, t, 1, 0)
{}

Animation::Animation(const std::string & name, const Texture & t, size_t frameCount, size_t speed)
    : m_name(name)
    , m_sprite(t.texture)
    , m_frameCount(frameCount)
    , m_currentFrame(0)
    , m_speed(speed)
    , m_ended(false)
{
    if (m_frameCount <= 0) {
        m_frameCount = 1;
    }

    setSprite(t);
}

void Animation::setSprite(const Texture & tex) {
    m_texture = tex;
    m_frameSize = Vec2(tex.size.x / m_frameCount, tex.size.y);
    
    m_sprite = sf::Sprite(tex.texture);
    m_sprite.setOrigin(m_frameSize / 2.0);

    size_t frame = 0;
    if (m_speed > 0) {
        frame = m_currentFrame / m_speed;
    }

    auto pos = m_texture.pos + Vec2((int)(frame * m_frameSize.x), 0);
    m_sprite.setTextureRect(sf::IntRect(pos, m_frameSize));
}

// updates the animation to show the next frame, depending on its speed
// animation loops when it reaches the end
void Animation::update() {
    if (m_frameCount < 2) {
        m_ended = true;
        return;
    }

    size_t frame = 0;
    if (m_speed > 0) {
        frame = m_currentFrame / m_speed;
    }

    auto pos = m_texture.pos + Vec2((int)(frame * m_frameSize.x), 0);

    m_sprite.setTextureRect(sf::IntRect(pos, m_frameSize));

    m_currentFrame++;

    if (m_currentFrame / m_speed >= m_frameCount) {
        m_currentFrame = 0;
        m_ended = true;
    }

    // TODO: 1) calculate the correct frame of animation to play based on currentFrame and speed
    //       2) set the texture rectangle properly (see constructor for sample)
}

const Vec2 & Animation::frameSize() const {
    return m_frameSize;
}

const std::string & Animation::getName() const {
    return m_name;
}

sf::Sprite & Animation::getSprite() {
    return m_sprite;
}

bool Animation::hasEnded() const {
    // TODO: detect when animation has ended (last frame was played) and return true
    return m_ended;
}

uint8_t Animation::getAlpha() const {
    return m_alpha;
}

void Animation::setAlpha(uint8_t v) {
    m_alpha = v;
}

const sf::Color Animation::getColor() const {
    return m_sprite.getColor();
}

void Animation::setColor(sf::Color v) {
    m_sprite.setColor(v);
}

const std::string & Animation::getShader() {
    return m_shader;
}

void Animation::setShader(const std::string & v) {
    m_shader = v;
}

float Animation::getMix() const {
    return m_mix;
}

void Animation::setMix(float v) {
    m_mix = v;
}

const Texture & Animation::getTexture() const {
    return m_texture;
}
