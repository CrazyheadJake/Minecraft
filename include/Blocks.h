#pragma once
#include "raylib.h"
#include <memory>
#include <vector>
#include <array>

enum class Block {
    AIR,
    GRASS
};

class Blocks {
    public:
        static const std::vector<Vector3>& getVertices(Block block);
        static const std::vector<unsigned short>& getIndices(Block block);
        static const std::vector<Vector2>& getTexcoords(Block block);
        static const std::vector<Vector3>& getNormals(Block block);
    private:
        static const std::vector<Vector3> CUBE_VERTICES;
        static const std::vector<unsigned short> CUBE_INDICES;
        static const std::vector<Vector2> CUBE_TEXCOORDS;
        static const std::vector<Vector3> CUBE_NORMALS;
};
