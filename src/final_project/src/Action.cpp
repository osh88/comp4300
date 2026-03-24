#include "Action.h"

#include <sstream>

Action::Action() {}

Action::Action(const std::string & name, const std::string & type)
    : m_name(name)
    , m_type(type)
{}

Action::Action(const std::string & name, const std::string & type, const Vec2& pos, const Vec2& screenPos)
    : m_name(name)
    , m_type(type)
    , m_pos(pos)
    , m_screenPos(screenPos)
{}

const std::string & Action::name() const {
    return m_name;
}

const std::string & Action::type() const {
    return m_type;
}

const Vec2 & Action::pos() const {
    return m_pos;
}

const Vec2 & Action::screenPos() const {
    return m_screenPos;
}

std::string Action::toString() const {
    std::stringstream ss;
    ss << name() << " " << type();
    return ss.str();
}