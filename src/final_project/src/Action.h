#pragma once

#include "Vec2.h"

#include <string>
#include <string_view>

class Action {
    std::string m_name = "NONE";
    std::string m_type = "NONE";
    Vec2        m_pos  = Vec2(0, 0);
    Vec2        m_screenPos = Vec2(0, 0);

public:

    Action() = default;
    Action(std::string_view name, std::string_view type);
    Action(std::string_view name, std::string_view type, const Vec2& pos, const Vec2& screenPos = {0, 0});

    const std::string & name() const;
    const std::string & type() const;
    const Vec2& pos() const;
    const Vec2& screenPos() const;
    std::string toString() const;
};
