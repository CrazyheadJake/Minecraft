#include "BlockMesh.h"
#include <vector>
#include "Blocks.h"
#include <string.h>
#include "raymath.h"
#include <iostream>

BlockMesh::BlockMesh(Vector3 offset) : m_chunkOffset(offset)
{
    for (Block& b: m_blocks) {
        b = Block::AIR;
    }
    m_image = GenImageChecked(16, 16, 8, 8, PURPLE, BLACK);
    m_texture = LoadTextureFromImage(m_image);
    generateMesh();
}

BlockMesh::~BlockMesh()
{
    UnloadModel(m_model);
    UnloadImage(m_image);
}

void BlockMesh::drawMesh()
{
    DrawModel(m_model, {0, 0, 0}, 1, WHITE);
}

void BlockMesh::generateMesh()
{
    for (int i = 0; i < LENGTH*WIDTH*HEIGHT; i++) {
        m_blocks[i] = Block::GRASS;
    }

    std::vector<Vector3> normals;

    for (int i = 0; i < m_blocks.size(); i++) {
        if (m_blocks[i] == Block::AIR)
            continue;

        const Vector3 offset = getGlobalCoord(i);
        const std::vector<Vector3>& blockVertices = Blocks::getVertices(m_blocks[i]);
        const std::vector<unsigned short>& blockIndices = Blocks::getIndices(m_blocks[i]);
        const std::vector<Vector2>& blockTexcoords = Blocks::getTexcoords(m_blocks[i]);
        const std::vector<Vector3>& blockNormals = Blocks::getNormals(m_blocks[i]);

        for (unsigned short index : blockIndices) {
            m_indices.push_back(index + m_vertices.size());
        }
        for (Vector3 vert : blockVertices) {
            m_vertices.push_back(Vector3Add(vert, offset));
        }
        m_texcoords.insert(m_texcoords.end(), blockTexcoords.begin(), blockTexcoords.end());
        normals.insert(normals.end(), blockNormals.begin(), blockNormals.end());
    }
    // print out a bunch of debug info
    std::cout << m_vertices.size() << std::endl;

    float* texcoords_ptr = (float*)malloc(m_texcoords.size() * 2 * sizeof(float));
    memcpy(texcoords_ptr, m_texcoords.data(), m_texcoords.size() * sizeof(Vector2));
    
    float* vertices_ptr = (float*)malloc(m_vertices.size() * 3 * sizeof(float));
    memcpy(vertices_ptr, m_vertices.data(), m_vertices.size() * sizeof(Vector3));

    float* normals_ptr = (float*)malloc(normals.size() * 3 * sizeof(float));
    memcpy(normals_ptr, normals.data(), normals.size() * sizeof(Vector3));

    unsigned short* indices_ptr = (unsigned short*)malloc(m_indices.size() * sizeof(unsigned short));
    memcpy(indices_ptr, m_indices.data(), m_indices.size() * sizeof(unsigned short));

    m_mesh = { 0 };
    m_mesh.vertexCount = m_vertices.size();
    m_mesh.triangleCount = m_indices.size() / 3;
    m_mesh.vertices = vertices_ptr;
    m_mesh.texcoords = texcoords_ptr;
    m_mesh.normals = normals_ptr;
    m_mesh.indices = indices_ptr;

    // Generate tangents (? idk if necessary)
    GenMeshTangents(&m_mesh);

    UploadMesh(&m_mesh, true);
    
    m_model = LoadModelFromMesh(m_mesh);
    if (IsTextureValid(m_model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture)) {
        UnloadTexture(m_model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture);
    }
    m_model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = m_texture;

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
