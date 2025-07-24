#include "BlockMesh.h"
#include <vector>
#include "Block.h"
#include <string.h>
#include "raymath.h"
#include <iostream>
#include <climits>
#include <span>
#include <algorithm>
#include "TextureLoader.h"
#include "Player.h"
#include "VectorUtils.h"
#include "Direction.h"

BlockMesh::BlockMesh(World &world, const siv::PerlinNoise& perlin, Vector3 offset) : m_chunkOffset(offset), m_world(world)
{
    generateWorld(perlin);
    m_world.regenerateChunk(getChunkLoc() + Vector2{1, 0});
    m_world.regenerateChunk(getChunkLoc() + Vector2{-1, 0});
    m_world.regenerateChunk(getChunkLoc() + Vector2{0, 1});
    m_world.regenerateChunk(getChunkLoc() + Vector2{0, -1});

    m_boundingBox.min = getGlobalCoord(0);
    m_boundingBox.max = getGlobalCoord(LENGTH * LENGTH * HEIGHT - 1) + Vector3{1.0f, 1.0f, 1.0f};
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

    for (int i = 0; i < m_model.meshCount - m_transparentMeshCount; i++) {
        if (m_model.meshes[i].vertexCount > 0) {
            DrawMesh(m_model.meshes[i], m_model.materials[0], m_model.transform);
        }
    }

    DrawBoundingBox(m_boundingBox, RED);
}

void BlockMesh::drawTransparentMesh() const
{
    if (m_state != State::MESH_UPLOADED)
        return;
    for (int i = m_model.meshCount - m_transparentMeshCount; i < m_model.meshCount; i++) {
        if (m_model.meshes[i].vertexCount == 0)
            return;
        BeginBlendMode(BLEND_ALPHA);
        DrawMesh(m_model.meshes[i], m_model.materials[0], m_model.transform);
        EndBlendMode();
    }

}

void BlockMesh::generateMeshData()
{
    clearMeshData();

    for (int i = 0; i < m_blocks.size(); i++) {
        if (m_blocks[i] == Blocks::AIR)
            continue;
        genBlockData(getLocalCoord(i));
    }
    m_state = State::MESHDATA_GENERATED;
    m_regenerate = false;
}

void BlockMesh::generateMeshesFromData()
{
    int transparentMeshCount = ceilf((float)m_transparentVerticesCount / (MAX_VERTS + 1));
    int meshCount = ceilf((float)m_verticesCount / (MAX_VERTS + 1)) + transparentMeshCount;
    
    Mesh* meshes = (Mesh*)malloc(meshCount * sizeof(Mesh));
    m_transparentBlocks.clear();
    
    size_t verticesRemaining = m_verticesCount;
    auto it = m_meshData.begin();
    for (int i = 0; i < meshCount - transparentMeshCount; i++) {
        size_t size = verticesRemaining > MAX_VERTS ? MAX_VERTS : verticesRemaining;
        size_t indicesSize = size * 1.5f;
        float* verticesPtr = (float*)malloc(size * 3 * sizeof(float));
        float* texcoordsPtr = (float*)malloc(size * 2 * sizeof(float));
        float* normalsPtr = (float*)malloc(size * 3 * sizeof(float));
        unsigned short* indicesPtr = (unsigned short*)malloc(indicesSize * sizeof(unsigned short));
        meshes[i] = {0};
        meshes[i].triangleCount = indicesSize / 3;
        meshes[i].vertices = verticesPtr;
        meshes[i].texcoords = texcoordsPtr;
        meshes[i].normals = normalsPtr;
        meshes[i].indices = indicesPtr;
        
        const BlockData* blockData;
        int k = 0;
        for (; it != m_meshData.end(); it++) {
            blockData = &it->second;
            if (blockData->transparent){
                m_transparentBlocks.push_back(blockData);
                continue;
            }
            if (k + blockData->vertices.size() > size)
                break;
            memcpy(meshes[i].vertices + k * 3, blockData->vertices.data(), blockData->vertices.size() * sizeof(Vector3));
            memcpy(meshes[i].texcoords + k * 2, blockData->texcoords.data(), blockData->texcoords.size() * sizeof(Vector2));
            memcpy(meshes[i].normals + k * 3, blockData->normals.data(), blockData->normals.size() * sizeof(Vector3));
            for (int index = 0; index < blockData->indices.size(); index++) {
                meshes[i].indices[index + (int)(k * 1.5f)] = blockData->indices[index] + k;
            }
            
            k += blockData->vertices.size();
        }
        meshes[i].vertexCount = k;
        indicesSize = k * 1.5f;
        meshes[i].triangleCount = indicesSize / 3;
        verticesRemaining -= k;

    }
    
    generateTransparentMeshes(Vector3{0, 0, 0}, &meshes[meshCount - transparentMeshCount], meshCount, transparentMeshCount);
    
    if (m_model.meshCount > 0)
        clearMeshes();
    m_model.meshes = meshes;
    m_model.meshCount = meshCount;
    m_transparentMeshCount = transparentMeshCount;

    m_state = State::MESH_GENERATED;
}

void BlockMesh::updateTransparentMeshes(Vector3 location)
{
    std::vector<Mesh> meshes(m_transparentMeshCount);
    generateTransparentMeshes(location, meshes.data(), m_model.meshCount, m_transparentMeshCount);
    for (int i = 0; i < m_transparentMeshCount; i++) {
        Mesh& mesh = m_model.meshes[i + m_model.meshCount - m_transparentMeshCount];
        UpdateMeshBuffer(mesh, TextureLoader::MESH_BUFFER_VERTEX, meshes[i].vertices, meshes[i].vertexCount * sizeof(Vector3), 0);
        UpdateMeshBuffer(mesh, TextureLoader::MESH_BUFFER_TEXCOORD, meshes[i].texcoords, meshes[i].vertexCount * sizeof(Vector2), 0);
        UpdateMeshBuffer(mesh, TextureLoader::MESH_BUFFER_NORMAL, meshes[i].normals, meshes[i].vertexCount * sizeof(Vector3), 0);
        memcpy(mesh.vertices, meshes[i].vertices, meshes[i].vertexCount * sizeof(Vector3));
        memcpy(mesh.texcoords, meshes[i].texcoords, meshes[i].vertexCount * sizeof(Vector2));
        memcpy(mesh.normals, meshes[i].normals, meshes[i].vertexCount * sizeof(Vector3));
        free(meshes[i].vertices);
        free(meshes[i].texcoords);
        free(meshes[i].normals);
        free(meshes[i].indices);
    } 
}

void BlockMesh::generateTransparentMeshes(Vector3 location, Mesh* meshes, int meshCount, int transparentMeshCount)
{
    std::sort(m_transparentBlocks.begin(), m_transparentBlocks.end(), [location](const BlockData* a, const BlockData* b) {
        return Vector3LengthSqr(a->vertices[0] - location) > Vector3LengthSqr(b->vertices[0] - location);
    });
    
    auto transparent_it = m_transparentBlocks.begin();
    size_t transparentVertsRemaining = m_transparentVerticesCount;
    for (int i = 0; i < transparentMeshCount; i++) {
        size_t size = transparentVertsRemaining > MAX_VERTS ? MAX_VERTS : transparentVertsRemaining;
        size_t indicesSize = size * 1.5f;
        float* texcoordsPtr = (float*)malloc(size * 2 * sizeof(float));
        float* verticesPtr = (float*)malloc(size * 3 * sizeof(float));
        float* normalsPtr = (float*)malloc(size * 3 * sizeof(float));
        unsigned short* indicesPtr = (unsigned short*)malloc(indicesSize * sizeof(unsigned short));
        meshes[i] = {0};
        meshes[i].vertices = verticesPtr;
        meshes[i].texcoords = texcoordsPtr;
        meshes[i].normals = normalsPtr;
        meshes[i].indices = indicesPtr;
        
        const BlockData* blockData;
        int k = 0;
        for (; transparent_it != m_transparentBlocks.end(); transparent_it++) {
            blockData = *transparent_it;
            if (k + blockData->vertices.size() > size)
                break;
            memcpy(meshes[i].vertices + k * 3, blockData->vertices.data(), blockData->vertices.size() * sizeof(Vector3));
            memcpy(meshes[i].texcoords + k * 2, blockData->texcoords.data(), blockData->texcoords.size() * sizeof(Vector2));
            memcpy(meshes[i].normals + k * 3, blockData->normals.data(), blockData->normals.size() * sizeof(Vector3));
            for (int index = 0; index < blockData->indices.size(); index++) {
                meshes[i].indices[index + (int)(k * 1.5f)] = blockData->indices[index] + k;
            }
            
            k += blockData->vertices.size();
        }
        meshes[i].vertexCount = k;
        indicesSize = k * 1.5f;
        meshes[i].triangleCount = indicesSize / 3;
        transparentVertsRemaining -= k;
    }
}

bool BlockMesh::shouldRegenerate() const
{
    return m_regenerate;
}

void BlockMesh::clearMeshData() {
    m_meshData.clear();
    m_transparentBlocks.clear();
    m_verticesCount = 0;
    m_transparentVerticesCount = 0;
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
        if (it->second.transparent)
            m_transparentVerticesCount -= it->second.vertices.size();
        else
            m_verticesCount -= it->second.vertices.size();
        m_meshData.erase(it);
    }
    if (m_blocks[i] != Blocks::AIR) {
        genBlockData(localCoord);
    }
    m_state = State::MESHDATA_GENERATED;
}

void BlockMesh::genBlockData(Vector3 localCoord)
{
    int i = (int)(localCoord.x) + (int)(localCoord.y) * LENGTH * LENGTH + (int)(localCoord.z) * LENGTH;

    const Vector3 offset = localCoord + m_chunkOffset;
    const std::vector<Vector3>& blockVertices = m_blocks[i].getVertices();
    const std::vector<unsigned short>& blockIndices = m_blocks[i].getIndices();
    const std::vector<Vector2>& blockTexcoords = m_blocks[i].getTexcoords();
    const std::vector<Vector3>& blockNormals = m_blocks[i].getNormals();

    std::pair<std::unordered_map<int, BlockMesh::BlockData>::iterator, bool> insertion;
    if (m_blocks[i].getTransparent()) {
        insertion = m_meshData.insert_or_assign(i, BlockData {});
        BlockData& blockData = insertion.first->second;
        blockData.transparent = true;
        blockData.block = m_blocks[i];
        for (int index = 0; index < blockVertices.size() * 1.5f; index++) {
            blockData.indices.push_back(blockIndices[index]);
        }
        for (int vert = 0; vert < blockVertices.size(); vert++) {
            blockData.vertices.push_back(blockVertices[vert] + offset);
            Direction d = Dir::getDirection(blockNormals[vert]);
            blockData.texcoords.push_back(TextureLoader::getTexCoord(m_blocks[i], d, blockTexcoords[vert]));
            blockData.normals.push_back(blockNormals[vert]);
        }
        m_transparentVerticesCount += blockVertices.size();
        return;
    }
    int facesSkipped = 0;
    BlockData* blockData;
    for (int face = 0; face < 6; face++) {
        // Skip this face if the neighbor block is not air, should really be looking for "transparent" blocks, but for now this is fine
        Vector3 normal = blockNormals[face * 4];
        Block neighborBlock = getBlockLocal(localCoord + normal);
        if (neighborBlock == Blocks::UNKNOWN) {
            neighborBlock = m_world.getBlockGlobal(localCoord + m_chunkOffset + normal);
        }
        if (neighborBlock.getTransparent())
            neighborBlock = Blocks::AIR;
        if (neighborBlock != Blocks::AIR || neighborBlock == Blocks::UNKNOWN) {
            facesSkipped++;
            continue; 
        }
        if (m_meshData.find(i) == m_meshData.end()) {
            insertion = m_meshData.insert_or_assign(i, BlockData {});
            blockData = &insertion.first->second;
            blockData->transparent = false;
            blockData->block = m_blocks[i];
        }
        for (int index = face * 6; index < face * 6 + 6; index++) {
            blockData->indices.push_back(blockIndices[index] - facesSkipped * 4);
        }
        for (int vert = face * 4; vert < face * 4 + 4; vert++) {
            blockData->vertices.push_back(blockVertices[vert] + offset);
            Direction d = Dir::getDirection(blockNormals[vert]);
            blockData->texcoords.push_back(TextureLoader::getTexCoord(m_blocks[i], d, blockTexcoords[vert]));
            blockData->normals.push_back(blockNormals[vert]);
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
                    setBlock(x, y, z, Blocks::AIR);
                }
                else if (y == topHeight) {
                    setBlock(x, y, z, Blocks::GRASS);
                }
                else if (y > topHeight - 4) {
                    setBlock(x, y, z, Blocks::DIRT);
                }
                else
                    setBlock(x, y, z, Blocks::STONE);
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
    // float maxAngle = Utils::toRadians(camera.getFov()) / sqrt(2);
    for (int i = 0; i < 8; i++) {
        Vector3 corner = getCorner(i);
        // if (Vector3Angle(corner - camera.getLocation(), camera.getDirection()) < maxAngle)
        //     return true;
        if (Vector3DotProduct(corner - camera.getLocation(), camera.getDirection()) > 0)
            return true;
    }
    return false;
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
    for (int mesh = 0; mesh < m_model.meshCount - m_transparentMeshCount; mesh++) {
        UploadMesh(&m_model.meshes[mesh], false);
    }
    for (int mesh = m_model.meshCount - m_transparentMeshCount; mesh < m_model.meshCount; mesh++) {
        UploadMesh(&m_model.meshes[mesh], true);
    }
    m_state = State::MESH_UPLOADED;
    m_boundingBox = GetModelBoundingBox(m_model);
}

Vector3 BlockMesh::getGlobalCoord(int i) const
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
        return Blocks::UNKNOWN;
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

void BlockMesh::generateModel()
{
    m_model.transform = MatrixTranslate(0.5, 0.5, 0.5); // Block corners are now integers instead of 0.5
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