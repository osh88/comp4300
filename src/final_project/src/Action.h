#pragma once

#include "Entity.h"

#include <string>

class Action {
    std::string m_name = "NONE";
    std::string m_type = "NONE";
    Vec2        m_pos  = Vec2(0, 0);
    Vec2        m_screenPos = Vec2(0, 0);

public:

    Action();
    Action(const std::string & name, const std::string & type);
    Action(const std::string & name, const std::string & type, const Vec2& pos, const Vec2& screenPos = {0, 0});

    const std::string & name() const;
    const std::string & type() const;
    const Vec2& pos() const;
    const Vec2& screenPos() const;
    std::string toString() const;
};