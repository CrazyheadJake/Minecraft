#include "Block.h"

// Define the static member variables for the Block class
std::vector<std::string> Block::s_names;
std::vector<std::vector<Vector3>> Block::s_vertices;
std::vector<std::vector<unsigned short>> Block::s_indices;
std::vector<std::vector<Vector2>> Block::s_texcoords;
std::vector<std::vector<Vector3>> Block::s_normals;
std::vector<bool> Block::s_transparencies;

Block::Block(std::string name, std::vector<Vector3> vertices, std::vector<unsigned short> indices, 
    std::vector<Vector2> texcoords, std::vector<Vector3> normals, bool transparent) : m_id(s_names.size()) {
    s_names.push_back(name);
    s_vertices.push_back(vertices);
    s_indices.push_back(indices);
    s_texcoords.push_back(texcoords);
    s_normals.push_back(normals);
    s_transparencies.push_back(transparent);
    Blocks::s_stringToBlock[name] = *this;
}

const Block &Blocks::getBlock(const std::string &name)
{
    auto it = s_stringToBlock.find(name);
    if (it == s_stringToBlock.end())
        return Blocks::UNKNOWN;
    return it->second;
}
