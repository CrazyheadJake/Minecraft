#pragma once
#include "raylib.h"
#include "raymath.h"

enum class Direction {
    NORTH,
    EAST,
    SOUTH,
    WEST,
    UP,
    DOWN
};

namespace Dir
{
    inline Direction getDirection(Vector3 normal)
    {
        if (Vector3Equals(normal, {1, 0, 0}))
            return Direction::NORTH;
        if (Vector3Equals(normal, {-1, 0, 0}))
            return Direction::SOUTH;
        if (Vector3Equals(normal, {0, 1, 0}))
            return Direction::UP;
        if (Vector3Equals(normal, {0, -1, 0}))
            return Direction::DOWN;
        if (Vector3Equals(normal, {0, 0, 1}))
            return Direction::EAST;
        if (Vector3Equals(normal, {0, 0, -1}))
            return Direction::WEST;
        return Direction::NORTH;
    }
}