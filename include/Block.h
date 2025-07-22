#pragma once
#include <string>
#include <vector>
#include "raylib.h"
#include <unordered_map>
#include <cstdint>
#include "Direction.h"

class Block {
public:
    typedef uint8_t BlockId;
    
    Block() : m_id(0) { }
    Block(BlockId i) : m_id(i) { m_id = m_id < 0 ? 0 : m_id >= s_names.size() ? s_names.size() - 1 : m_id; }
    Block(std::string name, std::vector<Vector3> vertices, std::vector<unsigned short> indices
        , std::vector<Vector2> texcoords, std::vector<Vector3> normals, bool transparent = false);
    constexpr std::strong_ordering operator<=>(const Block&) const = default;
    const std::string& getName() const { return s_names[m_id]; }
    const std::vector<Vector3>& getVertices() const { return s_vertices[m_id]; }
    const std::vector<unsigned short>& getIndices() const { return s_indices[m_id]; }
    const std::vector<Vector2>& getTexcoords() const { return s_texcoords[m_id]; }
    const std::vector<Vector3>& getNormals() const { return s_normals[m_id]; }
    bool getTransparent() const { return s_transparencies[m_id]; }
    BlockId getId() const { return m_id; }

private:
    BlockId m_id;
    static std::vector<std::string> s_names;
    static std::vector<std::vector<Vector3>> s_vertices;
    static std::vector<std::vector<unsigned short>> s_indices;
    static std::vector<std::vector<Vector2>> s_texcoords;
    static std::vector<std::vector<Vector3>> s_normals;
    static std::vector<bool> s_transparencies;
};

namespace Blocks {
    inline std::unordered_map<std::string, Block> s_stringToBlock;

    std::vector<Vector3> const CUBE_VERTICES = {
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
    std::vector<unsigned short> const CUBE_INDICES = {
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
    std::vector<Vector2> const CUBE_TEXCOORDS = {
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
    std::vector<Vector3> const CUBE_NORMALS = {
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

    inline Block UNKNOWN        = Block("unknown", {}, {}, {}, {});
    inline Block AIR            = Block("air", {}, {}, {}, {});
    inline Block DIRT           = Block("dirt", CUBE_VERTICES, CUBE_INDICES, CUBE_TEXCOORDS, CUBE_NORMALS);
    inline Block GRASS          = Block("grass", CUBE_VERTICES, CUBE_INDICES, CUBE_TEXCOORDS, CUBE_NORMALS);
    inline Block STONE          = Block("stone", CUBE_VERTICES, CUBE_INDICES, CUBE_TEXCOORDS, CUBE_NORMALS);
    inline Block OAK_LOG        = Block("oak_log", CUBE_VERTICES, CUBE_INDICES, CUBE_TEXCOORDS, CUBE_NORMALS, true);
    inline Block OAK_PLANKS     = Block("oak_planks", CUBE_VERTICES, CUBE_INDICES, CUBE_TEXCOORDS, CUBE_NORMALS);
    inline Block OAK_LEAVES     = Block("oak_leaves", CUBE_VERTICES, CUBE_INDICES, CUBE_TEXCOORDS, CUBE_NORMALS, true);

    extern const Block& getBlock(const std::string& name);
};

struct BlockFace {
    Block::BlockId block;
    Direction face;

    constexpr std::strong_ordering operator<=>(const BlockFace&) const = default;
};
