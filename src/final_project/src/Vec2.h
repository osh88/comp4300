#pragma once

#include <math.h>
#include <iostream>

#include "json.hpp"
using json = nlohmann::json;

template <typename T>
concept SameAsVec2 = requires(T o) {
    { o.x } -> std::convertible_to<float>;
    { o.y } -> std::convertible_to<float>;
};

template <typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

class Vec2 {
public:
    float x = 0;
    float y = 0;

    Vec2() {};

    explicit Vec2(float xy) : x(xy), y(xy) {};

    Vec2(float x, float y) : x(x), y(y) {};

    template <SameAsVec2 T>
    Vec2(const T& v) : x(v.x), y(v.y) {};

    template <SameAsVec2 T>
    T& operator=(const T& v) {
        if (this != &v) { // Self-assignment check
            this->x = v.x;
            this->y = v.y;
        }
        return *this; // Return a reference to the current object
    };

    template <SameAsVec2 T>
    operator T() const {
        return T(x, y);
    };

    template <SameAsVec2 T>
    bool operator == (const T & rhs) const {
        return x == rhs.x && y == rhs.y;
    }

    template <SameAsVec2 T>
    bool operator != (const T & rhs) const {
        return x != rhs.x || y != rhs.y;
    }

    template <SameAsVec2 T>
    Vec2 operator + (const T & rhs) const {
        return Vec2(x+rhs.x, y+rhs.y);
    }

    template <SameAsVec2 T>
    Vec2 operator - (const T & rhs) const {
        return Vec2(x-rhs.x, y-rhs.y);
    }

    template <SameAsVec2 T>
    Vec2 operator % (const T & rhs) const {
        return Vec2((int)x % (int)rhs.x, (int)y % (int)rhs.y);
    }

    template <Numeric T>
    Vec2 operator / (const T val) const {
        return Vec2(x/val, y/val);
    }

    template <SameAsVec2 T>
    Vec2 operator / (const T & rhs) const {
        return Vec2(x/rhs.x, y/rhs.y);
    }

    template <Numeric T>
    Vec2 operator % (const T val) const {
        return Vec2((int)x % (int)val, (int)y % (int)val);
    }

    template <Numeric T>
    Vec2 operator * (const T val) const {
        return Vec2(x*val, y*val);
    }

    template <SameAsVec2 T>
    float operator * (const T & rhs) const {
        return x*rhs.y - y*rhs.x;
    }

    template <Numeric T>
    Vec2 operator + (const T val) const {
        return Vec2(x+val, y+val);
    }

    template <Numeric T>
    Vec2 operator - (const T val) const {
        return Vec2(x-val, y-val);
    }

    Vec2 operator - () const {
        return Vec2(-x, -y);
    }

    template <SameAsVec2 T>
    void operator += (const T & rhs) {
        x += rhs.x;
        y += rhs.y;
    }

    template <SameAsVec2 T>
    void operator -= (const T & rhs) {
        x -= rhs.x;
        y -= rhs.y;
    }

    template <Numeric T>
    void operator *= (const T val) {
        x *= val;
        y *= val;
    }

    template <Numeric T>
    void operator /= (const T val) {
        x /= val;
        y /= val;
    }

    template <Numeric T>
    void operator -= (const T val) {
        x -= val;
        y -= val;
    }

    template <SameAsVec2 T>
    float dist(const T & rhs) const {
        return sqrtf((rhs.x-x)*(rhs.x-x) + (rhs.y-y)*(rhs.y-y));
    }

    float length() const {
        return sqrtf(x*x + y*y);
    }

    Vec2 abs() const {
        return Vec2(std::abs(x), std::abs(y));
    }

    friend std::ostream& operator<<(std::ostream& os, const Vec2& v);

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Vec2, x, y)
};
