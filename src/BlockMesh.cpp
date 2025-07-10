#include "BlockMesh.h"
#include <vector>
#include "Blocks.h"
#include <string.h>
#include "raymath.h"
#include <iostream>
#include <climits>
#include <span>
#include <algorithm>
#include "TextureLoader.h"
#include "MyCamera.h"

BlockMesh::BlockMesh(Vector3 offset) : m_chunkOffset(offset)
{   
    generateWorld();
    generateMeshes();
    m_state = State::MESH_GENERATED;
}

BlockMesh::~BlockMesh()
{
    for (auto& mesh: m_meshes) {
        UnloadMesh(mesh);
    }
    free(m_model.meshes);
    free(m_model.materials);
    free(m_model.meshMaterial);
}

void BlockMesh::drawMesh() const
{
    // DrawModel is nonfunctional on Windows, so we use DrawMesh instead
    #ifdef __linux__
        DrawModel(m_model, {0, 0, 0}, 1, WHITE);
    #else
        for (int i = 0; i < m_model.meshCount; i++) {
            if (m_model.meshes[i].vertexCount > 0) {
                DrawMesh(m_model.meshes[i], m_model.materials[0], m_model.transform);
            }
        }
    #endif
    // DrawBoundingBox(m_boundingBox, RED);
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
        int vertCount = vertices.size();
        int facesSkipped = 0;
        for (int face = 0; face < 6; face++) {
            // Skip this face if the neighbor block is not air, should really be looking for "transparent" blocks, but for now this is fine
            Vector3 normal = blockNormals[face * 4];
            Vector3 localCoord = getLocalCoord(i);
            Block neighborBlock = getBlockLocal(localCoord + normal);
            if (neighborBlock != Block::AIR && neighborBlock != Block::UNKNOWN) {
                facesSkipped++;
                continue; 
            }
            for (int index = face * 6; index < face * 6 + 6; index++) {
                indices.push_back(blockIndices[index] + vertCount - facesSkipped * 4);
            }
            for (int vert = face * 4; vert < face * 4 + 4; vert++) {
                vertices.push_back(Vector3Add(blockVertices[vert], offset));
                Direction d = Blocks::getDirection(blockNormals[vert]);
                texcoords.push_back(TextureLoader::getTexCoord(m_blocks[i], d, blockTexcoords[vert]));
                normals.push_back(blockNormals[vert]);  
            }
        }
    }
    if (vertices.size() > 0) {
        addMesh(vertices, indices, texcoords, normals);
    }
}

void BlockMesh::generateWorld()
{
    for (int y  = 0; y < HEIGHT; y++) {
        for (int z = 0; z < LENGTH; z++) {
            for (int x = 0; x < LENGTH; x++) {
                if (y > 20) {
                    setBlock(x, y, z, Block::AIR);
                }
                else if (y == 20) {
                    setBlock(x, y, z, Block::GRASS);
                }
                else if (y > 15) {
                    setBlock(x, y, z, Block::DIRT);
                }
                else
                    setBlock(x, y, z, Block::STONE);
            }
        }
    }
}

bool BlockMesh::isValid()
{
    return m_state == State::MESH_UPLOADED;
}

bool BlockMesh::isVisible(MyCamera &camera) const
{
    bool visible = false;
    Vector3 coord = getCorner(0);
    if (Vector3DotProduct(coord - camera.position, camera.fwd) > 0) {
        visible = true;
    }
    coord = getCorner(1);
    if (Vector3DotProduct(coord - camera.position, camera.fwd) > 0) {
        visible = true;
    }
    coord = getCorner(2);
    if (Vector3DotProduct(coord - camera.position, camera.fwd) > 0) {
        visible = true;
    }
    coord = getCorner(3);
    if (Vector3DotProduct(coord - camera.position, camera.fwd) > 0) {
        visible = true;
    }
    coord = getCorner(4);
    if (Vector3DotProduct(coord - camera.position, camera.fwd) > 0) {
        visible = true;
    }
    coord = getCorner(5);
    if (Vector3DotProduct(coord - camera.position, camera.fwd) > 0) {
        visible = true;
    }
    coord = getCorner(6);
    if (Vector3DotProduct(coord - camera.position, camera.fwd) > 0) {
        visible = true;
    }
    coord = getCorner(7);
    if (Vector3DotProduct(coord - camera.position, camera.fwd) > 0) {
        visible = true;
    }
    
    return visible;
}

void BlockMesh::tryUploadMeshes()
{
    if (m_state == State::UNINITIALIZED) {
        return;
    }
    if (m_state == State::MESH_GENERATED) {
        uploadMeshes();
        m_state = State::MESH_UPLOADED;
    }
}

Vector3 BlockMesh::getLocalCoord(int i) const
{
    return {(float)(i % LENGTH),
        (float)((i / LENGTH / LENGTH) % HEIGHT), 
        (float)((i / LENGTH) % LENGTH)};
}

void BlockMesh::uploadMeshes()
{
    for (auto& mesh: m_meshes) {
        UploadMesh(&mesh, true);
    }
    generateModel();
}

Vector3 BlockMesh::getGlobalCoord(int i) const        // Not sure if the math in this works if length != width
{
    return Vector3Add({(float)(i % LENGTH),
        (float)((i / LENGTH / LENGTH) % HEIGHT), 
        (float)((i / LENGTH) % LENGTH)}, m_chunkOffset);
}

Vector2 BlockMesh::getChunkLoc() const
{
    return {floorf(m_chunkOffset.x/LENGTH), floorf(m_chunkOffset.z/LENGTH)};
}

void BlockMesh::setBlock(int x, int y, int z, Block block)
{
    m_blocks[z * LENGTH + y * LENGTH * LENGTH + x] = block;
}

Block BlockMesh::getBlockLocal(Vector3 localCoord) const
{
    if (localCoord.x < 0 || localCoord.x >= LENGTH ||
        localCoord.y < 0 || localCoord.y >= HEIGHT ||
        localCoord.z < 0 || localCoord.z >= LENGTH) {
        return Block::UNKNOWN; // Out of bounds
    }
    return m_blocks[(int)localCoord.x + (int)localCoord.y * LENGTH * LENGTH + (int)localCoord.z * LENGTH];
}

Vector3 BlockMesh::getCorner(int i) const
{
    switch (i) {
        case 0: return m_boundingBox.min;
        case 1: return {m_boundingBox.max.x, m_boundingBox.min.y, m_boundingBox.min.z};
        case 2: return {m_boundingBox.min.x, m_boundingBox.min.y, m_boundingBox.max.z};
        case 3: return {m_boundingBox.max.x, m_boundingBox.min.y, m_boundingBox.max.z};
        case 4: return {m_boundingBox.min.x, m_boundingBox.max.y, m_boundingBox.min.z};
        case 5: return {m_boundingBox.max.x, m_boundingBox.max.y, m_boundingBox.min.z};
        case 6: return {m_boundingBox.min.x, m_boundingBox.max.y, m_boundingBox.max.z};
        case 7: return m_boundingBox.max;
    }
    return {0, 0, 0}; // This line is unreachable, but it prevents a warning about not returning a value
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
}

void BlockMesh::generateModel()
{
    m_model.transform = MatrixIdentity();

    m_model.meshCount = m_meshes.size();
    m_model.meshes = (Mesh*)malloc(m_meshes.size() * sizeof(Mesh));
    memcpy(m_model.meshes, m_meshes.data(), m_meshes.size() * sizeof(Mesh));
    
    m_model.materialCount = 1;
    m_model.materials = (Material*)malloc(sizeof(Material));
    // m_model.materials[0] = LoadMaterialDefault();
    m_model.meshMaterial = (int*)malloc(sizeof(int));
    m_model.meshMaterial[0] = 0;

    // I'm afraid of memory leaks here (maybe GPU), but address sanitizer seems to think there are none
    m_model.materials[0] = TextureLoader::s_material;

    m_boundingBox = {getGlobalCoord(0) - Vector3{0.5, 0.5, 0.5}, getGlobalCoord(LENGTH * LENGTH * HEIGHT - 1) + Vector3{0.5, 0.5, 0.5}};
}