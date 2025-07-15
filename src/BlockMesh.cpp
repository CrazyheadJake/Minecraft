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
#include "Player.h"
#include "VectorUtils.h"

BlockMesh::BlockMesh(World &world, const siv::PerlinNoise& perlin, Vector3 offset) : m_chunkOffset(offset), m_world(world)
{   
    generateWorld(perlin);
    m_world.regenerateChunk(getChunkLoc() + Vector2{1, 0});
    m_world.regenerateChunk(getChunkLoc() + Vector2{-1, 0});
    m_world.regenerateChunk(getChunkLoc() + Vector2{0, 1});
    m_world.regenerateChunk(getChunkLoc() + Vector2{0, -1});

    m_boundingBox.min = getGlobalCoord(0);
    m_boundingBox.max = getGlobalCoord(LENGTH * LENGTH * HEIGHT - 1);
}

BlockMesh::~BlockMesh()
{
    clearMeshData();
    clearMeshes();
    clearModel();
}

void BlockMesh::drawMesh() const
{
    if (m_state != State::MESH_UPLOADED)
        return;

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
    DrawBoundingBox(m_boundingBox, RED);
}

void BlockMesh::generateMeshData()
{
    clearMeshData();

    for (int i = 0; i < m_blocks.size(); i++) {
        if (m_blocks[i] == Block::AIR)
            continue;
        genBlockData(getLocalCoord(i));
    }
    m_state = State::MESHDATA_GENERATED;
    m_regenerate = false;
}

void BlockMesh::generateMeshesFromData()
{
    int meshCount = ceilf((float)m_verticesCount / (MAX_VERTS + 1));
    Mesh* meshes = (Mesh*)malloc(meshCount * sizeof(Mesh));

    auto it = m_meshData.begin();
    for (int i = 0; i < meshCount; i++) {
        size_t size = m_verticesCount - i * MAX_VERTS > MAX_VERTS ? MAX_VERTS : m_verticesCount - i * MAX_VERTS;
        size_t indicesSize = size * 1.5f;
        float* texcoords_ptr = (float*)malloc(size * 2 * sizeof(float));
        float* vertices_ptr = (float*)malloc(size * 3 * sizeof(float));
        float* normals_ptr = (float*)malloc(size * 3 * sizeof(float));
        unsigned short* indices_ptr = (unsigned short*)malloc(indicesSize * sizeof(unsigned short));
        meshes[i] = {0};
        meshes[i].vertexCount = size;
        meshes[i].triangleCount = indicesSize / 3;
        meshes[i].vertices = vertices_ptr;
        meshes[i].texcoords = texcoords_ptr;
        meshes[i].normals = normals_ptr;
        meshes[i].indices = indices_ptr;
        
        const BlockData* blockData;
        for (int k = 0; k < size; k += blockData->vertices.size()) {
            blockData = &m_meshData.at(it->first);
            memcpy(meshes[i].vertices + k * 3, blockData->vertices.data(), blockData->vertices.size() * sizeof(Vector3));
            memcpy(meshes[i].texcoords + k * 2, blockData->texcoords.data(), blockData->texcoords.size() * sizeof(Vector2));
            memcpy(meshes[i].normals + k * 3, blockData->normals.data(), blockData->normals.size() * sizeof(Vector3));
            for (int index = 0; index < blockData->indices.size(); index++) {
                meshes[i].indices[index + (int)(k * 1.5f)] = blockData->indices[index] + k;
            }
            
            it++;
        }

    }
    if (m_model.meshCount > 0)
        clearMeshes();
    m_model.meshes = meshes;
    m_model.meshCount = meshCount;

    m_state = State::MESH_GENERATED;
}

bool BlockMesh::shouldRegenerate() const
{
    return m_regenerate;
}

void BlockMesh::clearMeshData() {
    m_meshData.clear();
    m_verticesCount = 0;
}


void BlockMesh::clearMeshes()
{
    for (int mesh = 0; mesh < m_model.meshCount; mesh++) {
        UnloadMesh(m_model.meshes[mesh]);
    }
    free(m_model.meshes);
    m_model.meshCount = 0;
}

void BlockMesh::clearModel()
{
    m_model.materialCount = 0;
    free(m_model.materials);
    free(m_model.meshMaterial);
}

void BlockMesh::updateBlockData(Vector3 localCoord)
{
    int i = (int)(localCoord.x) + (int)(localCoord.y) * LENGTH * LENGTH + (int)(localCoord.z) * LENGTH;
    auto it = m_meshData.find(i);
    if (it != m_meshData.end()) {
        m_verticesCount -= it->second.vertices.size();
        m_meshData.erase(it);
    }
    if (m_blocks[i] != Block::AIR) {
        genBlockData(localCoord);
    }
    m_state = State::MESHDATA_GENERATED;
}

void BlockMesh::genBlockData(Vector3 localCoord)
{
    int i = (int)(localCoord.x) + (int)(localCoord.y) * LENGTH * LENGTH + (int)(localCoord.z) * LENGTH;
    
    auto insertion = m_meshData.insert_or_assign(i, BlockData {});
    BlockData& blockData = insertion.first->second;

    const Vector3 offset = localCoord + m_chunkOffset;
    const std::vector<Vector3>& blockVertices = Blocks::getVertices(m_blocks[i]);
    const std::vector<unsigned short>& blockIndices = Blocks::getIndices(m_blocks[i]);
    const std::vector<Vector2>& blockTexcoords = Blocks::getTexcoords(m_blocks[i]);
    const std::vector<Vector3>& blockNormals = Blocks::getNormals(m_blocks[i]);
    int facesSkipped = 0;
    for (int face = 0; face < 6; face++) {
        // Skip this face if the neighbor block is not air, should really be looking for "transparent" blocks, but for now this is fine
        Vector3 normal = blockNormals[face * 4];
        Block neighborBlock = getBlockLocal(localCoord + normal);
        if (neighborBlock == Block::UNKNOWN) {
            neighborBlock = m_world.getBlockGlobal(localCoord + m_chunkOffset + normal);
        }
        if (neighborBlock != Block::AIR || neighborBlock == Block::UNKNOWN) {
            facesSkipped++;
            continue; 
        }

        for (int index = face * 6; index < face * 6 + 6; index++) {
            blockData.indices.push_back(blockIndices[index] - facesSkipped * 4);
        }
        for (int vert = face * 4; vert < face * 4 + 4; vert++) {
            blockData.vertices.push_back(Vector3Add(blockVertices[vert], offset));
            Direction d = Blocks::getDirection(blockNormals[vert]);
            blockData.texcoords.push_back(TextureLoader::getTexCoord(m_blocks[i], d, blockTexcoords[vert]));
            blockData.normals.push_back(blockNormals[vert]);  
        }
        m_verticesCount += 4;
    }
}

void BlockMesh::requestRegenerate()
{
    m_regenerate = true;
}

void BlockMesh::generateWorld(const siv::PerlinNoise& perlin)
{
    for (int x  = 0; x < LENGTH; x++) {
        for (int z = 0; z < LENGTH; z++) {    
            double noise = perlin.octave2D((double)(x + m_chunkOffset.x) * SCALE, (double)(z + m_chunkOffset.z) * SCALE, 3, 0.5);
            int topHeight = SEALEVEL + (int)(noise * 20);
            for (int y = 0; y < HEIGHT; y++) {
                if (y > topHeight) {
                    setBlock(x, y, z, Block::AIR);
                }
                else if (y == topHeight) {
                    setBlock(x, y, z, Block::GRASS);
                }
                else if (y > topHeight - 4) {
                    setBlock(x, y, z, Block::DIRT);
                }
                else
                    setBlock(x, y, z, Block::STONE);
            }
        }
    }
    m_state = State::WORLD_GENERATED;
}

bool BlockMesh::isValid() const
{
    return m_state == State::MESH_UPLOADED;
}

bool BlockMesh::isVisible(Player &camera) const
{
    bool visible = false;
    Vector3 coord = getCorner(0);
    if (Vector3DotProduct(coord - camera.getLocation(), camera.getDirection()) > 0) {
        visible = true;
    }
    coord = getCorner(1);
    if (Vector3DotProduct(coord - camera.getLocation(), camera.getDirection()) > 0) {
        visible = true;
    }
    coord = getCorner(2);
    if (Vector3DotProduct(coord - camera.getLocation(), camera.getDirection()) > 0) {
        visible = true;
    }
    coord = getCorner(3);
    if (Vector3DotProduct(coord - camera.getLocation(), camera.getDirection()) > 0) {
        visible = true;
    }
    coord = getCorner(4);
    if (Vector3DotProduct(coord - camera.getLocation(), camera.getDirection()) > 0) {
        visible = true;
    }
    coord = getCorner(5);
    if (Vector3DotProduct(coord - camera.getLocation(), camera.getDirection()) > 0) {
        visible = true;
    }
    coord = getCorner(6);
    if (Vector3DotProduct(coord - camera.getLocation(), camera.getDirection()) > 0) {
        visible = true;
    }
    coord = getCorner(7);
    if (Vector3DotProduct(coord - camera.getLocation(), camera.getDirection()) > 0) {
        visible = true;
    }
    
    return visible;
}

void BlockMesh::tryUploadMeshes()
{
    if (m_state == State::MESH_GENERATED) {
        uploadMeshes();
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
    if (m_model.materialCount == 0) {
        generateModel();
    }
    for (int mesh = 0; mesh < m_model.meshCount; mesh++) {
        UploadMesh(&m_model.meshes[mesh], false);
    }
    m_state = State::MESH_UPLOADED;

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

void BlockMesh::setBlock(Vector3 loc, Block block, bool updateMesh)
{
    setBlock((int)loc.x, (int)loc.y, (int)loc.z, block, updateMesh);
}

void BlockMesh::setBlock(int x, int y, int z, Block block, bool updateMesh)
{
    m_blocks[z * LENGTH + y * LENGTH * LENGTH + x] = block;
    if (updateMesh) {
        lock();
        updateBlockData({(float)x, (float)y, (float)z});
        unlock();
    }
}

Block BlockMesh::getBlockLocal(Vector3 localCoord) const
{
    if (localCoord.x < 0 || localCoord.x >= LENGTH ||
        localCoord.y < 0 || localCoord.y >= HEIGHT ||
        localCoord.z < 0 || localCoord.z >= LENGTH) {
        return Block::UNKNOWN;
    }
    return m_blocks[(int)(localCoord.x) + (int)(localCoord.y) * LENGTH * LENGTH + (int)(localCoord.z) * LENGTH];
}

bool BlockMesh::isLocalCoord(Vector3 localCoord) const
{
    return localCoord.x >= 0 && localCoord.x < LENGTH &&
           localCoord.y >= 0 && localCoord.y < HEIGHT &&
           localCoord.z >= 0 && localCoord.z < LENGTH;
}

void BlockMesh::tryGenerateMeshData() {
    if (m_state == State::WORLD_GENERATED || m_regenerate) {
        generateMeshData();
    }
}

void BlockMesh::tryGenerateMeshes()
{
    if (m_state == State::MESHDATA_GENERATED) {
        generateMeshesFromData();
    }
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

// void BlockMesh::addMesh(const std::vector<Vector3>& vertices, const std::vector<unsigned short>& indices, const std::vector<Vector2>& texcoords, const std::vector<Vector3>& normals)
// {
//     // size_t size = std::min(vertices.size(), (size_t)UINT16_MAX);
//     size_t size = vertices.size();
//     float* texcoords_ptr = (float*)malloc(size * 2 * sizeof(float));
//     memcpy(texcoords_ptr, texcoords.data(), size * sizeof(Vector2));
    
//     float* vertices_ptr = (float*)malloc(vertices.size() * 3 * sizeof(float));
//     memcpy(vertices_ptr, vertices.data(), size * sizeof(Vector3));

//     float* normals_ptr = (float*)malloc(normals.size() * 3 * sizeof(float));
//     memcpy(normals_ptr, normals.data(), size * sizeof(Vector3));

//     unsigned short* indices_ptr = (unsigned short*)malloc(indices.size() * sizeof(unsigned short));
//     memcpy(indices_ptr, indices.data(), indices.size() * sizeof(unsigned short));

//     m_meshes.push_back({0});

//     Mesh& mesh = m_meshes.back();
//     mesh.vertexCount = vertices.size();
//     mesh.triangleCount = indices.size() / 3;
//     mesh.vertices = vertices_ptr;
//     mesh.texcoords = texcoords_ptr;
//     mesh.normals = normals_ptr;
//     mesh.indices = indices_ptr;
// }

void BlockMesh::generateModel()
{
    m_model.transform = MatrixTranslate(0.5, 0.5, 0.5); // Block corners are now integers instead of 0.5

    // m_model.meshCount = m_meshes.size();
    if (m_model.meshCount > 1) {
        std::cout << "Warning: More than one mesh in BlockMesh, this is not expected!" << std::endl;
    }
    // m_model.meshes = (Mesh*)malloc(m_meshes.size() * sizeof(Mesh));
    // memcpy(m_model.meshes, m_meshes.data(), m_meshes.size() * sizeof(Mesh));
    
    m_model.materialCount = 1;
    m_model.materials = (Material*)malloc(sizeof(Material));
    m_model.meshMaterial = (int*)malloc(sizeof(int));
    m_model.meshMaterial[0] = 0;

    // I'm afraid of memory leaks here (maybe GPU), but address sanitizer seems to think there are none
    m_model.materials[0] = TextureLoader::s_material;

    m_boundingBox = GetModelBoundingBox(m_model);
}

void BlockMesh::lock()
{
    m_dataLock.lock();
}

void BlockMesh::unlock()
{
    m_dataLock.unlock();
}