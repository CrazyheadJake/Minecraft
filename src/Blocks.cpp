#include "Blocks.h"

std::vector<Vector3> const Blocks::CUBE_VERTICES = {
    {-0.500f, -0.500f, 0.500f},
    {0.500f, -0.500f, 0.500f},
    {0.500f, 0.500f, 0.500f},
    {-0.500f, 0.500f, 0.500f},
    {-0.500f, -0.500f, -0.500f}, 
    {-0.500f, 0.500f, -0.500f},
    {0.500f, 0.500f, -0.500f},
    {0.500f, -0.500f, -0.500f},
    {-0.500f, 0.500f, -0.500f},
    {-0.500f, 0.500f, 0.500f},
    {0.500f, 0.500f, 0.500f}, 
    {0.500f, 0.500f, -0.500f},
    {-0.500f, -0.500f, -0.500f},
    {0.500f, -0.500f, -0.500f},
    {0.500f, -0.500f, 0.500f},
    {-0.500f, -0.500f, 0.500f},
    {0.500f, -0.500f, -0.500f},
    {0.500f, 0.500f, -0.500f},
    {0.500f, 0.500f, 0.500f},
    {0.500f, -0.500f, 0.500f},
    {-0.500f, -0.500f, -0.500f},
    {-0.500f, -0.500f, 0.500f},
    {-0.500f, 0.500f, 0.500f},
    {-0.500f, 0.500f, -0.500f}
};

std::vector<unsigned short> const Blocks::CUBE_INDICES = {
    0, 1, 2, 
    0, 2, 3, 
    4, 5, 6, 
    4, 6, 7, 
    8, 9, 10, 
    8, 10, 11, 
    12, 13, 14,
    12, 14, 15, 
    16, 17, 18, 
    16, 18, 19, 
    20, 21, 22, 
    20, 22, 23 
};

std::vector<Vector2> const Blocks::CUBE_TEXCOORDS = {
    {0.000f, 0.000f}, 
    {1.000f, 0.000f}, 
    {1.000f, 1.000f}, 
    {0.000f, 1.000f}, 
    {1.000f, 0.000f}, 
    {1.000f, 1.000f}, 
    {0.000f, 1.000f}, 
    {0.000f, 0.000f}, 
    {0.000f, 1.000f}, 
    {0.000f, 0.000f}, 
    {1.000f, 0.000f}, 
    {1.000f, 1.000f}, 
    {1.000f, 1.000f}, 
    {0.000f, 1.000f}, 
    {0.000f, 0.000f}, 
    {1.000f, 0.000f}, 
    {1.000f, 0.000f}, 
    {1.000f, 1.000f}, 
    {0.000f, 1.000f}, 
    {0.000f, 0.000f}, 
    {0.000f, 0.000f}, 
    {1.000f, 0.000f}, 
    {1.000f, 1.000f}, 
    {0.000f, 1.000f}
};

std::vector<Vector3> const Blocks::CUBE_NORMALS = {
    {0.000f, 0.000f, 1.000f}, 
    {0.000f, 0.000f, 1.000f}, 
    {0.000f, 0.000f, 1.000f}, 
    {0.000f, 0.000f, 1.000f}, 
    {0.000f, 0.000f, -1.000f},
    {0.000f, 0.000f, -1.000f}, 
    {0.000f, 0.000f, -1.000f}, 
    {0.000f, 0.000f, -1.000f}, 
    {0.000f, 1.000f, 0.000f}, 
    {0.000f, 1.000f, 0.000f}, 
    {0.000f, 1.000f, 0.000f}, 
    {0.000f, 1.000f, 0.000f}, 
    {0.000f, -1.000f, 0.000f}, 
    {0.000f, -1.000f, 0.000f}, 
    {0.000f, -1.000f, 0.000f}, 
    {0.000f, -1.000f, 0.000f}, 
    {1.000f, 0.000f, 0.000f}, 
    {1.000f, 0.000f, 0.000f}, 
    {1.000f, 0.000f, 0.000f}, 
    {1.000f, 0.000f, 0.000f}, 
    {-1.000f, 0.000f, 0.000f}, 
    {-1.000f, 0.000f, 0.000f}, 
    {-1.000f, 0.000f, 0.000f}, 
    {-1.000f, 0.000f, 0.000f}
};

const std::vector<Vector3>& Blocks::getVertices(Block block)
{
    switch (block) {
        case Block::AIR:
            return {};
        case Block::GRASS:
            return CUBE_VERTICES;
        default:
            return CUBE_VERTICES;
    }
    return {};
}

const std::vector<unsigned short>& Blocks::getIndices(Block block)
{
    switch (block) {
        case Block::AIR:
            return {};
        case Block::GRASS:
            return CUBE_INDICES;
        default:
            return CUBE_INDICES;
    }
    return {};
}

const std::vector<Vector2>& Blocks::getTexcoords(Block block)
{
    switch (block) {
        case Block::AIR:
            return {};
        case Block::GRASS:
            return CUBE_TEXCOORDS;
        default:
            return CUBE_TEXCOORDS;
    }
    return {};
}

const std::vector<Vector3>& Blocks::getNormals(Block block)
{
    switch (block) {
        case Block::AIR:
            return {};
        case Block::GRASS:
            return CUBE_NORMALS;
        default:
            return CUBE_NORMALS;
    }
    return {};
}
