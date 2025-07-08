#pragma once
#include "raylib.h"
#include <functional>

namespace Utils {
        // For use in busMap
    struct Vector2Hash {
        size_t operator()(const Vector2& v) const {
            return std::hash<float>()(v.x) ^ (std::hash<float>()(v.y) << 1);
        }
    };

    // For use in busMap
    struct Vector2Equal {
        bool operator()(const Vector2& a, const Vector2& b) const {
            return a.x == b.x && a.y == b.y;
        }
    };

    Vector2 floorVector(const Vector2& v, int precision = 1);
    Vector2 roundVector(const Vector2& v, int precision = 1);
    Vector3 roundVector(const Vector3& v, int precision = 1);

};