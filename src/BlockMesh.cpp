#include "BlockMesh.h"
#include <vector>
#include "Blocks.h"
#include <string.h>
#include "raymath.h"
#include <iostream>
#include <climits>
#include <span>
#include <algorithm>

BlockMesh::BlockMesh(Vector3 offset) : m_chunkOffset(offset)
{
    m_image = LoadImage("assets/textures/blocks/dirt.png");
    m_texture = LoadTextureFromImage(m_image);
    for (int i = 0; i < LENGTH*WIDTH*HEIGHT; i++) {
        m_blocks[i] = Block::GRASS;
    }

    generateMeshes();
    generateModel();

}

BlockMesh::~BlockMesh()
{
    UnloadModel(m_model);
    UnloadImage(m_image);
}

void BlockMesh::drawMesh() const
{
    DrawModel(m_model, {0, 0, 0}, 1, WHITE);
}

void BlockMesh::generateMeshes()
{
    std::vector<Vector3> vertices;
    std::vector<unsigned short> indices;
    std::vector<Vector2> texcoords;
    std::vector<Vector3> normals;

    for (int i = 0; i < m_blocks.size(); i++) {
        if (m_blocks[i] == Block::AIR)
            continue;

        const Vector3 offset = getGlobalCoord(i);
        const std::vector<Vector3>& blockVertices = Blocks::getVertices(m_blocks[i]);
        if (vertices.size() + blockVertices.size() > MAX_VERTS) {
            addMesh(vertices, indices, texcoords, normals);
            vertices.clear();
            indices.clear();
            texcoords.clear();
            normals.clear();
            i--;
            continue;
        }
        const std::vector<unsigned short>& blockIndices = Blocks::getIndices(m_blocks[i]);
        const std::vector<Vector2>& blockTexcoords = Blocks::getTexcoords(m_blocks[i]);
        const std::vector<Vector3>& blockNormals = Blocks::getNormals(m_blocks[i]);

        for (unsigned short index : blockIndices) {
            indices.push_back(index + vertices.size());
        }
        for (Vector3 vert : blockVertices) {
            vertices.push_back(Vector3Add(vert, offset));
        }
        texcoords.insert(texcoords.end(), blockTexcoords.begin(), blockTexcoords.end());
        normals.insert(normals.end(), blockNormals.begin(), blockNormals.end());
    }
    if (vertices.size() > 0) {
        addMesh(vertices, indices, texcoords, normals);
    }
}

void BlockMesh::updateMesh()
{
}

Vector3 BlockMesh::getGlobalCoord(int i)
{
    return Vector3Add({(float)(i % LENGTH),
        (float)((i / LENGTH / WIDTH) % HEIGHT), 
        (float)((i / LENGTH) % WIDTH)}, m_chunkOffset);
}

void BlockMesh::addMesh(const std::vector<Vector3>& vertices, const std::vector<unsigned short>& indices, const std::vector<Vector2>& texcoords, const std::vector<Vector3>& normals)
{
    // size_t size = std::min(vertices.size(), (size_t)UINT16_MAX);
    size_t size = vertices.size();
    float* texcoords_ptr = (float*)malloc(size * 2 * sizeof(float));
    memcpy(texcoords_ptr, texcoords.data(), size * sizeof(Vector2));
    
    float* vertices_ptr = (float*)malloc(vertices.size() * 3 * sizeof(float));
    memcpy(vertices_ptr, vertices.data(), size * sizeof(Vector3));

    float* normals_ptr = (float*)malloc(normals.size() * 3 * sizeof(float));
    memcpy(normals_ptr, normals.data(), size * sizeof(Vector3));

    unsigned short* indices_ptr = (unsigned short*)malloc(indices.size() * sizeof(unsigned short));
    memcpy(indices_ptr, indices.data(), indices.size() * sizeof(unsigned short));

    m_meshes.push_back({0});

    Mesh& mesh = m_meshes.back();
    mesh.vertexCount = vertices.size();
    mesh.triangleCount = indices.size() / 3;
    mesh.vertices = vertices_ptr;
    mesh.texcoords = texcoords_ptr;
    mesh.normals = normals_ptr;
    mesh.indices = indices_ptr;

    // Generate tangents (? idk if necessary)
    // GenMeshTangents(&m_mesh);

    UploadMesh(&mesh, true);
}

void BlockMesh::generateModel()
{
    m_model.transform = MatrixIdentity();

    m_model.meshCount = m_meshes.size();
    m_model.meshes = (Mesh*)malloc(m_meshes.size() * sizeof(Mesh));
    memcpy(m_model.meshes, m_meshes.data(), m_meshes.size() * sizeof(Mesh));
    
    m_model.materialCount = 1;
    m_model.materials = (Material*)malloc(sizeof(Material));
    m_model.materials[0] = LoadMaterialDefault();
    m_model.meshMaterial = (int*)malloc(sizeof(int));
    m_model.meshMaterial[0] = 0;

    // I'm afraid of memory leaks here (maybe GPU), but address sanitizer seems to think there are none
    m_model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = m_texture;
}