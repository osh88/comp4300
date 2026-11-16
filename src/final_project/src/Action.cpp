#include "Action.h"

#include <sstream>
#include <string_view>

Action::Action(std::string_view name, std::string_view type)
    : m_name(name)
    , m_type(type)
{}

Action::Action(std::string_view name, std::string_view type, const Vec2& pos, const Vec2& screenPos)
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
