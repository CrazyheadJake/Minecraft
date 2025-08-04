#include "Block.h"
#include "TextureLoader.h"

// Define the static member variables for the Block class
std::vector<std::string> Block::s_names;
std::vector<std::vector<Vector3>> Block::s_vertices;
std::vector<std::vector<unsigned short>> Block::s_indices;
std::vector<std::vector<Vector2>> Block::s_texcoords;
std::vector<std::vector<Vector3>> Block::s_normals;
std::vector<bool> Block::s_transparencies;
std::vector<bool> Block::s_translucencies;
std::vector<int> Block::s_priorities;

Block::Block(std::string name, std::vector<Vector3> vertices, std::vector<unsigned short> indices, 
    std::vector<Vector2> texcoords, std::vector<Vector3> normals, bool transparent, bool translucent, int priority) : m_id(s_names.size()) {
    s_names.push_back(name);
    s_vertices.push_back(vertices);
    s_indices.push_back(indices);
    s_texcoords.push_back(texcoords);
    s_normals.push_back(normals);
    s_transparencies.push_back(transparent);
    s_translucencies.push_back(translucent);
    s_priorities.push_back(priority);
    Blocks::s_stringToBlock[name] = *this;
}

const Block &Blocks::getBlock(const std::string &name)
{
    auto it = s_stringToBlock.find(name);
    if (it == s_stringToBlock.end())
        return Blocks::UNKNOWN;
    return it->second;
}


Model Block::generateModel()
{
    Model model = {0};
    model.transform = MatrixIdentity();
    model.meshCount = 1;
    model.meshes = (Mesh*)malloc(model.meshCount * sizeof(Mesh));
    model.meshes[0] = generateMesh();
    model.meshMaterial = (int*)malloc(sizeof(int));
    model.meshMaterial[0] = 0;
    model.materialCount = 1;
    model.materials = (Material*)malloc(model.materialCount * sizeof(Material));
    model.materials[0] = TextureLoader::s_material;
    return model;
}

Mesh Block::generateMesh()
{
    Mesh mesh = {0};
    const std::vector<Vector3>& blockVertices = getVertices();
    const std::vector<unsigned short>& blockIndices = getIndices();
    const std::vector<Vector2>& blockTexcoords = getTexcoords();
    const std::vector<Vector3>& blockNormals = getNormals();

    unsigned short* indicesPtr = (unsigned short*)malloc(blockIndices.size() * sizeof(unsigned short));
    float* verticesPtr = (float*)malloc(blockVertices.size() * 3 * sizeof(float));
    float* texcoordsPtr = (float*)malloc(blockTexcoords.size() * 2 * sizeof(float));
    float* normalsPtr = (float*)malloc(blockNormals.size() * 3 * sizeof(float));

    mesh.vertexCount = blockVertices.size();
    mesh.triangleCount = blockIndices.size() / 3;
    mesh.vertices = verticesPtr;
    mesh.texcoords = texcoordsPtr;
    mesh.normals = normalsPtr;
    mesh.indices = indicesPtr;

    memcpy(mesh.vertices, blockVertices.data(), blockVertices.size() * 3 * sizeof(float));
    memcpy(mesh.normals, blockNormals.data(), blockNormals.size() * 3 * sizeof(float));
    memcpy(mesh.indices, blockIndices.data(), blockIndices.size() * sizeof(unsigned short));

    for (int vert = 0; vert < blockVertices.size(); vert++) {
        Direction d = Dir::getDirection(blockNormals[vert]);
        Vector2 texcoord = TextureLoader::getTexCoord(*this, d, blockTexcoords[vert]);
        texcoordsPtr[2*vert] = texcoord.x;
        texcoordsPtr[2*vert+1] = texcoord.y;
    }
    return mesh;
}

