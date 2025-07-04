#pragma once
#include "raylib.h"
#include <memory>
#include <vector>
#include <array>
#include <unordered_map>

enum class Direction {
    NORTH,
    EAST,
    SOUTH,
    WEST,
    UP,
    DOWN
};

enum class Block {
    UNKNOWN,
    AIR,
    GRASS,
    STONE,
    DIRT
};

struct BlockFace {
    Block block;
    Direction face;

    bool operator==(const BlockFace& other) const {
        return other.block == block && other.face == face;
    }
};

class Blocks {
    public:
        static const std::vector<Vector3>& getVertices(Block block);
        static const std::vector<unsigned short>& getIndices(Block block);
        static const std::vector<Vector2>& getTexcoords(Block block);
        static const std::vector<Vector3>& getNormals(Block block);
        static Direction getDirection(Vector3 normal);

        static const Block getBlock(const std::string& name);
    private:
        static const std::vector<Vector3> CUBE_VERTICES;
        static const std::vector<unsigned short> CUBE_INDICES;
        static const std::vector<Vector2> CUBE_TEXCOORDS;
        static const std::vector<Vector3> CUBE_NORMALS;
        static const std::vector<Vector3> EMPTY;
        static const std::unordered_map<std::string, Block> s_stringToBlock;
};
